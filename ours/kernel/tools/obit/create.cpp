#include "subcmd.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include <ustl/mem/align.hpp>
#include <omitl/obi-header.hpp>
#include <omitl/obi-type.hpp>

#include <cxxopts.hpp>  // Third-party library for command-line options parsing
#include <spdlog/spdlog.h>

#undef PACKED
#include <LIEF/LIEF.hpp>

namespace fs = ::std::filesystem;

struct CreatationContext: SubCmd::Context {
    std::vector<std::string> inputs;
    std::string output;
    omitl::ObiType type;
    std::size_t payload_size;
    bool zipped;
};

static auto do_create_inner(CreatationContext &context) -> bool {
    auto const is_container = context.type == omitl::ObiType::Container;

    // Should check the payload if it is a valid group of OBI.
    omitl::ObiHeader obi_header{};

    // Do not move it to the top, because we must check if the input items are existent at first.
    std::ofstream out(context.output, std::ios::binary | std::ios::out);
    if (!out) {
        SPDLOG_ERROR("Failed to open output file: {}", context.output);
        return false;
    }

    std::size_t total_size = 0;
    std::vector<char> buffer(4096);
    // Preserve a size bytes of OBI header. 
    out.seekp(sizeof(obi_header));
    for (auto iter = context.inputs.begin(), end = context.inputs.end(); iter != end; ++iter) {
        if (!fs::exists(*iter)) {
            SPDLOG_ERROR("No such file or directory: {}", *iter);
            return false;
        }

        std::ifstream in(*iter, std::ios::binary | std::ios::in);
        if (!in) {
            SPDLOG_ERROR("Failed to open input file: {}", *iter);
            return false;
        }

        // If the target is a container OBI, we should check if the all payload files are a valid OBI object.
        if (is_container) {
            std::error_code error;
            auto size = fs::file_size(*iter, error);
            if (error.value()) {
                SPDLOG_ERROR("{}", error.message());
                // Should we skip it?
            }

            if (size % OMI_ALIGNMENT != 0) {
                SPDLOG_ERROR("The file is misaligned: {}", *iter);
                std::terminate();
            }
            
            if (size < sizeof(omitl::ObiHeader)) {
                SPDLOG_ERROR("The size of file lesser than size of ObiHeader, it may not be a valid OBI file: {}", *iter);
                return false;
            } else if (size == sizeof(omitl::ObiHeader)) {
                SPDLOG_WARN("The size of file equal to size of ObiHeader, it may not be a valid OBI file."
                             "Please consider to remove it: {}", *iter);
            }

            in.read(reinterpret_cast<char *>(&obi_header), sizeof(obi_header));
            if (!obi_header.validate()) {
                SPDLOG_ERROR("The file is not a valid OBI object: {}", *iter);
                return false;
            }

            in.seekg(-sizeof(obi_header), std::ios::cur);
        }

        auto file_size = 0;
        while (1) {
            in.read(buffer.data(), buffer.size());
            if (!in.gcount()){
                break;
            }
            file_size += in.gcount();
            out.write(buffer.data(), in.gcount());
        };

        // Because the code above to read file size through the interface from std::filesystem
        // may be failed, we must at here do the second check.
        if (is_container) {
            if (file_size % OMI_ALIGNMENT != 0) {
                SPDLOG_ERROR("The file is misaligned to OMI_ALIGNMENT: {}", *iter);
                std::terminate();
            }
        }

        if (!in.eof()) {
            SPDLOG_ERROR("Failed to read input file: {}", *iter);
            return false;
        }

        total_size += file_size;
    }

    obi_header.type = context.type;
    obi_header.magic = OMI_HEADER_MAGIC;
    obi_header.length = total_size;
    obi_header.check_code = 0;
    obi_header.flags = 0;
    obi_header.extra = 0;

    out.seekp(0);
    out.write(reinterpret_cast<char *>(&obi_header), sizeof(obi_header));

    return true;
}

static auto do_create(std::unique_ptr<SubCmd::Context> &context_base) -> bool {
    auto context = dynamic_cast<CreatationContext *>(context_base.get());
    if (!context) {
        SPDLOG_ERROR("Invalid OBI creation context.");
        return false;
    }

    return do_create_inner(*context);
}

static auto prepare_create_context(cxxopts::ParseResult &parser) -> std::unique_ptr<SubCmd::Context> {
    auto context = std::make_unique<CreatationContext>();
    if (!context) {
        SPDLOG_ERROR("Failed to create the creation context of OBI.");
        return {};
    }

    // Parse the list of input files
    if (parser.count("files")) {
        context->inputs = parser["files"].as<std::vector<std::string>>();
    } else {
        SPDLOG_WARN("No payload file given.");
    }

    if (!parser.count("type")) {
        SPDLOG_ERROR("No OBI type given, please use -t to pass the type of target OBI");
        return {};
    }
    auto type = parser["type"].as<std::string>();

    auto obitype = omitl::from_string<omitl::ObiType>(type.data());
    if (obitype == omitl::ObiType::Unknown) {
        SPDLOG_ERROR("Invalid OBI type {}", type);
        return {};
    }
    context->type = obitype;
    context->output = parser["output"].as<std::string>();
    context->zipped = parser["zip"].as<bool>();

    return context;
}

static SubCmd s_create_subcmd {
    "create",
    prepare_create_context,
    do_create,
    SubCmd::ParamsDemand::Required,
    {
        cxxopts::Option{"f,files", "File to fill as the payload of target OBI", cxxopts::value<std::vector<std::string>>()},
        cxxopts::Option{"t,type", "Type for OBI to generate", cxxopts::value<std::string>()},
        cxxopts::Option{"o,output", "Name of OBI ouput file", cxxopts::value<std::string>()->default_value("a.obi")},
        cxxopts::Option{"z,zip", "Compress the payload", cxxopts::value<bool>()->default_value("false")},
        cxxopts::Option{"h,help", "Print help"}
    }
};