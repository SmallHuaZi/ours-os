#include <logz4/log.hpp>

namespace logz4 {
    static Logger *GLOBAL_LOGGER;

    auto init(Logger *logger) -> void
    {}

    auto set_global_logger(Logger &logger) -> void
    {
        GLOBAL_LOGGER = &logger;
    }

    auto get_global_logger() -> Logger &
    {
        return *GLOBAL_LOGGER;
    }
} // namespace logz4