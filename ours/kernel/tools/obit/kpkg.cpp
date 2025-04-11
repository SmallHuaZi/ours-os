/// The final form of Kernel package would be an simple RAM file system.
///
/// Assume the initial kernel package layout:
///  ours/
///  |-- logs
///  |   |-- phys-boot.log
///  |-- config
///  |   |-- phys-config.txt
///  |   |-- main-config.txt
///
/// After the following command run.
///     obit kpkg --add kernel_main.elf --path mods/main.elf --type elf
///
/// The Kernel package has been changed as:
///  ours/
///  |-- logs
///  |   |-- phys-boot.log
///  |-- config
///  |   |-- phys-config.txt
///  |   |-- main-config.txt
///  |-- mods
///      |-- main.elf
///
/// After the following command run.
///     obit kpkg --add kernel_main.elf --path mods/main.elf --type obi
///

#include "subcmd.hpp"

#include <map>
#include <span>
#include <limits>
#include <iostream>
#include <fstream>
#include <format>

#include <omitl/obi-header.hpp>
#include <spdlog/spdlog.h>

#undef PACKED
#include <LIEF/LIEF.hpp>

enum class OpCode {
    Add,
    Remove,
    Unknown,
};

struct KpkgContext: SubCmd::Context {
    auto write_back(std::span<char> payload) -> bool {
        std::fstream fs(package_, std::ios::in | std::ios::out | std::ios::binary);
        if (!fs) {
            SPDLOG_ERROR("Failed to open file: {}", package_);
            return false;
        }
        omitl::ObiHeader header;
        fs.read(reinterpret_cast<char *>(&header), sizeof(header));
        fs.write(payload.data(), payload.size());

        header.length += payload.size();
        fs.seekp(0).write(reinterpret_cast<char *>(&header), sizeof(header));

        return true;
    }

    OpCode opcode_ = OpCode::Unknown;
    std::string out_;
    std::string file_;
    std::string package_;
    bool executable_;
    omitl::ObiExeSign exe_type_;
};

static auto extract_obi_exe_raw_from_elf(KpkgContext &context, std::unique_ptr<LIEF::ELF::Binary> elf) -> bool {
    auto constexpr kMetaSize = sizeof(omitl::ObiExeHeader);

    std::uintptr_t minimal_va = UINTPTR_MAX, maximal_va = 0;
    for (auto &segment: elf->segments()) {
        if (segment.is_load()) {
            minimal_va = std::min(minimal_va, segment.virtual_address());
            maximal_va = std::max(maximal_va, segment.virtual_address() + segment.virtual_size());
        }
    }
    assert(minimal_va < maximal_va);

    auto const payload_size = kMetaSize + ustl::mem::align_up(maximal_va - minimal_va, OMI_ALIGNMENT);
    std::vector<char> buffer(payload_size);

    std::size_t reserved_size = 0;
    for (auto const &segment : elf->segments()) {
        if (!segment.is_load()) {
            continue;
        }

        auto const va = segment.virtual_address();
        auto const vs = segment.virtual_size();
        if (segment.physical_size() == 0) {
            if (minimal_va < va && va < maximal_va) {
                // Usually we do not expect the case happen, but it not an error. Giveing out an warning is enough.
                SPDLOG_INFO("Inside BSS-like segment (v:{}, s:{})", va, vs);
            } else if (va >= maximal_va) {
                // Being alignment requires possibly, so plus (va - maximal_va)
                reserved_size += vs + va - maximal_va;
            }
        }

        auto const virtual_end = std::min(maximal_va, va + vs);
        auto const virtual_size = virtual_end - va;
        auto const ofs = va - minimal_va + kMetaSize;
        std::copy_n(segment.content().begin(), virtual_size, buffer.begin() + ofs);
    }

    omitl::ObiExeHeader exe_header;
    exe_header.signature = omitl::ObiExeSign::Raw;
    exe_header.entry_point = elf->entrypoint();
    exe_header.reserved_size = reserved_size;
    std::copy_n(reinterpret_cast<std::uint8_t *>(&exe_header), sizeof(exe_header), buffer.begin());

    return context.write_back(buffer);
}

static auto create_obi_exe_raw(KpkgContext &context) -> bool {
    if (auto elf = LIEF::ELF::Parser::parse(context.file_)) {
        return extract_obi_exe_raw_from_elf(context, std::move(elf));
    }

    return false;
}

static auto create_obi_exe_elf(KpkgContext &context) -> bool {
    if (!std::filesystem::exists(context.file_)) {
        SPDLOG_ERROR("No such file or directory: ", context.file_);
        return false;
    }

    auto constexpr kMetaSize = sizeof(omitl::ObiExeSign);
    auto const file_size = std::filesystem::file_size(context.file_);
    std::vector<char> buffer(file_size + kMetaSize);

    auto signature = reinterpret_cast<omitl::ObiExeSign *>(buffer.data());
    *signature = omitl::ObiExeSign::Elf;

    std::ifstream fs(context.file_, std::ios::binary | std::ios::in);
    if (!fs) {
        SPDLOG_ERROR("Failed to open file: ", context.file_);
        return false;
    }

    fs.read(buffer.data() + kMetaSize, file_size);
    if (fs.gcount() != file_size) {
        SPDLOG_ERROR("Failed to read file: ", context.file_);
        return false;
    }

    return context.write_back(buffer);
}

static auto dispatch_creation_action(KpkgContext &context) -> bool {
    switch (context.exe_type_) {
        case omitl::ObiExeSign::Raw: return create_obi_exe_raw(context);
        case omitl::ObiExeSign::Elf: return create_obi_exe_elf(context);
    };

    return false;
}

static auto do_kpkg(std::unique_ptr<SubCmd::Context> &context_base) -> bool {
    auto context = dynamic_cast<KpkgContext *>(context_base.get());
    if (!context) {
        SPDLOG_ERROR("Invalid KPKG context");
        return false;
    }

    // Do diapatch
    switch (context->opcode_) {
        case OpCode::Add: {
            return dispatch_creation_action(*context);
        }
        case OpCode::Remove: 
            break;
    }

    return false;
}

static auto prepare_kpkg_context(cxxopts::ParseResult &parser) -> std::unique_ptr<SubCmd::Context> {
    auto context = std::make_unique<KpkgContext>();
    if (!context) {
        SPDLOG_ERROR("Failed to create kpkg context");
        return {};
    }

    if (parser.contains("package")) {
        context->package_ = parser["package"].as<std::string>();
    }

    if (parser.contains("add")) {
        context->opcode_ = OpCode::Add;
        context->file_ = parser["add"].as<std::string>();
    }

    if (parser.contains("remove")) {
        context->opcode_ = OpCode::Remove;
        context->file_ = parser["remove"].as<std::string>();
    }

    if (context->opcode_ == OpCode::Unknown) {
        SPDLOG_ERROR("Unknown command: {}", parser.begin()->as<std::string>());
        return {};
    }

    if (parser.contains("out")) {
        context->out_ = parser["out"].as<std::string>();
    } else {
        context->out_ = std::filesystem::path(context->file_).filename();
    }

    context->executable_ = parser["executable"].as<bool>();
    if (context->executable_) {
        context->exe_type_ = omitl::from_string<omitl::ObiExeSign>(parser["type"].as<std::string>().data());
    }

    return context;
}

static SubCmd s_kpkg_subcmd {
    "kpkg",
    prepare_kpkg_context,
    do_kpkg,
    SubCmd::ParamsDemand::Required,
    {
        cxxopts::Option{"p,package", "The target kernel package", cxxopts::value<std::string>()},
        cxxopts::Option{"add", "Add a file to kernel package", cxxopts::value<std::string>()},
        cxxopts::Option{"remove", "Remove a file from kernel package", cxxopts::value<std::string>()},
        cxxopts::Option{"o,out", "Explicitly give out the target name in kernel package", cxxopts::value<std::string>()},

        // The following two options is only valid to add/remove a runnable target
        cxxopts::Option{"e,executable", "Explicitly give out the target name in kernel package", cxxopts::value<bool>()->default_value("false")},
        cxxopts::Option{"t,type", "raw, obi, elf, e.g.", cxxopts::value<std::string>()->default_value("raw")},
    }
};