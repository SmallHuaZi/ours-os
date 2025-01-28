#ifndef OURS_ASSERT_H
#define OURS_ASSERT_H 1

#define DEBUG_ASSERT(condition, ...) 

#ifndef STOP_AFTER_N_TIMES
#define STOP_AFTER_N_TIMES(n) {   \
    static int j = 0;       \
    j++;                    \
    while (j > n);          \
}
#endif // #ifndef STOP_AFTER_N_TIMES 

#endif // #ifndef OURS_ASSERT_H