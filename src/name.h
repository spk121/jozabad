#ifndef JOZA_NAME_H
#define JOZA_NAME_H

/*
#ifndef WIN32
# include <stdbool.h>
#else
// Use MSVC's native bool type
#endif

#include <stdint.h>

#define NAME_LEN (16)

typedef union {
    char str[NAME_LEN];
    uint64_t u64[2];
} name_t;

/* Returns TRUE if the worker name N is valid. */
//bool val_name(name_t N);

#endif
