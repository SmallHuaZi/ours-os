#ifndef TOOLS_MKOBI_SUBCMD_HPP
#define TOOLS_MKOBI_SUBCMD_HPP

#include <map>
#include <string>
#include <iostream>
#include <functional>
#include <expected>

#include <cxxopts.hpp>  // Third-party library for command-line options parsing

struct SubCmd {
    struct Context {
        Context() = default;
        virtual ~Context() = default;
    };

    using Parser = std::function<auto (cxxopts::ParseResult &) -> std::unique_ptr<Context>>;
    using Handler = std::function<auto (std::unique_ptr<Context> &) -> bool>;

    enum class ParamsDemand {
        Required,
        Optional,
        NoParams,
    };

    SubCmd(std::string name, 
           Parser parser, 
           Handler handler, 
           ParamsDemand params_demand, 
           std::initializer_list<cxxopts::Option> option)
        : name_(name), 
          parser_(parser), 
          handler_(handler), 
          params_demand_(params_demand), 
          options_("obit" + name)
    {
        if (!handler) {
            std::cerr << "Invalid handler for subcommand '" << name << "'\n";
        }
        options_.add_options(name, option);

        install();
    }

    enum class GetSubCmdError {
        NoFound,
    };
    static auto get(std::string name) -> std::expected<SubCmd *, GetSubCmdError> {
        if (!g_subcommands.contains(name)) {
            return std::unexpected(GetSubCmdError::NoFound);
        }

        return std::addressof(g_subcommands.at(name));
    }

    auto options() -> cxxopts::Options & {
        return options_;
    }

    auto prepare(cxxopts::ParseResult &args) -> std::unique_ptr<Context> {
        if (args.begin() == args.end()) {
            if (params_demand_ == ParamsDemand::Required) {
                std::cerr << options_.program() << " requires arguments but no provide.\n";
                std::terminate();
            }
        } else if (params_demand_ == ParamsDemand::NoParams) {
            std::cerr << options_.program() << " do not need any parameters.\n";
        }

        return std::invoke(parser_, args);
    }

    auto execute(std::unique_ptr<Context> context) -> bool {
        if (!context) {
            return false;
        }

        return std::invoke(handler_, context);
    }

    auto install() -> bool {
        if (g_subcommands.contains(name_)) {
            std::cerr << "SubCmd `" << name_ << "` has been installed.\n";
            return false;
        }

        g_subcommands.insert({ name_, *this });
        return true;
    }

  private:
    std::string name_;
    Parser  parser_;
    Handler handler_;
    cxxopts::Options options_;
    ParamsDemand  params_demand_;

    static inline std::map<std::string, SubCmd> g_subcommands;
};

inline auto to_string(SubCmd::GetSubCmdError error) -> char const * {
    switch (error) {
        case SubCmd::GetSubCmdError::NoFound: return "No such sub-command found.";
    }
    return "";
}

#endif // TOOLS_MKOBI_SUBCMD_HPP