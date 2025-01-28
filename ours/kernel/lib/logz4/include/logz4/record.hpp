#ifndef LOGZ4_RECORD_H_
#define LOGZ4_RECORD_H_ 1

#include <logz4/level.hpp>

namespace logz4 {
    class Record
    {
    private:
        Level level_;
        char const *target_;
    };
}

#endif // #ifndef LOGZ4_RECORD_H_
