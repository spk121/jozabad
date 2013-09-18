#ifndef JOZA_IODIR_H
#define JOZA_IODIR_H

typedef enum {
    io_bidirectional,
    io_incoming_calls_barred,
    io_outgoing_calls_barred,
    io_calls_barred
} iodir_t;

int iodir_validate(iodir_t x);
const char *iodir_name(iodir_t x);
#endif
