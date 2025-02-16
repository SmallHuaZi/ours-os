#ifndef OURS_MARCO_ABI_H
#define OURS_MARCO_ABI_H 1

#define __OURS_MPL_CONTACT(X, Y) X ## Y
#define OURS_MPL_CONTACT(X, Y) __OURS_MPL_CONTACT(X, Y)

#define BIT(POS)    (1ULL << (POS))

#define BIT_WIDTH(N)

/// [START_BIT, END_BIT)
#define BIT_RANGE(START_BIT, END_BIT)   (((1ULL << ((END_BIT) - (START_BIT) + 1)) - 1) << (START_BIT))

#endif // #ifndef OURS_MARCO_ABI_H