#ifndef LOGZ4_LOGGER_H_
#define LOGZ4_LOGGER_H_ 1

#include <logz4/record.hpp>

namespace logz4 {
    class Logger
    {
    public:
        Logger();

        virtual ~Logger();

        virtual auto is_enabled() -> bool = 0;

        virtual auto log(Record &record) -> void = 0;

        virtual auto flush() -> void = 0;

        virtual auto kill() -> void = 0;
    };

} // namespace logz4

#endif // #ifndef LOGZ4_LOGGER_H_