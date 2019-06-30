#include <limits.h>
#include "rijndael-alg-fst.hpp"

// State blocks = 1 for weakest form
#define STATE_BLOCKS (1)	/* MUST be 1 for AES */
#define BLOCKS_SIZE (16 * STATE_BLOCKS)
#define NR 10

typedef struct {
    u32 rk[4*(NR + 1)];
    unsigned char block[BLOCKS_SIZE];
    short int pos;
} AES_state_t;

unsigned long int aes_get (void *vstate);
double aes_get_double (void *vstate);
void aes_set (void *vstate, unsigned long int s);