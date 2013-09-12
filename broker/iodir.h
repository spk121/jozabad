#ifndef JOZA_IODIR_H
#define JOZA_IODIR_H

typedef enum { FREE, INPUT, OUTPUT, BLOCK } iodir_t;

#define VAL_IODIR(x) ((x)==FREE || (x)==INPUT || (x)==OUTPUT)

#endif
