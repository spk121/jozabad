#ifndef JOZA_CHANNEL_H
#define JOZA_CHANNEL_H

#ifndef SEQ_WIDTH
#define SEQ_WIDTH 2
#endif

#if SEQ_WIDTH == 1
typedef uint8_t seq_t;
#define SEQ_MIN UINT8_C(0)
#define SEQ_MAX UINT8_C(SCHAR_MAX)
#define SEQ_C(x) UINT8_C(x)

#elif SEQ_WIDTH == 2
typedef uint16_t seq_t;
#define SEQ_MIN UINT16_C(0)
#define SEQ_MAX UINT16_C(INT16_MAX)
#define SEQ_C(x) UINT16_C(x)

#elif SEQ_WIDTH == 4
typedef uint32_t seq_t;
#define SEQ_MIN UINT32_C(0)
#define SEQ_MAX UINT32_C(INT32_MAX)
#define SEQ_C(x) UINT32_C(x)

#else
#error "Bad SEQ_WIDTH"
#endif

#ifndef WINDOW_DEFAULT
#define WINDOW_DEFAULT SEQ_C(2)
#endif

#endif
