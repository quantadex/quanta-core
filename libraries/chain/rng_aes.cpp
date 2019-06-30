#include "include/graphene/rng/rng_aes.hpp"
#include <string.h>

/*
 * This is a wrapping of the AES algorithm as a generator
 */

unsigned long int aes_get (void *vstate);
double aes_get_double (void *vstate);
void aes_set (void *vstate, unsigned long int s);

unsigned long int aes_get (void *vstate) {
	AES_state_t *state = (AES_state_t*)vstate;
	unsigned int ret;

	if (state->pos + sizeof(ret) > BLOCKS_SIZE) {
		rijndaelEncrypt(state->rk, NR, state->block, state->block);
		state->pos = 0;
	}

	ret = *((unsigned int *) (state->block + state->pos));
	state->pos += sizeof(ret);

//	ret &= 0x7fffffff;
	return(ret);
}


double aes_get_double (void *vstate) {
    //	return aes_get_long(vstate) / (double) ULONG_MAX;
	return (double) aes_get(vstate) / (double) (UINT_MAX >> 0);
}

void aes_set (void *vstate, unsigned long int s) {
    AES_state_t *state = (AES_state_t*)vstate;
	int i;
	u8 key[16];

	//printf("set seed to %ld\n",s);
	memset(state, 0, sizeof(*state));	// Zero pos and block

	/* Make sure to use all bits of s in the key:
	 * (5 * i) % 26 => {0,5,10,15,20,25,4,9,14,19,24,3,8,13,18,23}
	 * */
	for (i = 0; i < 16; i++) {
		key[i] = (u8) (112 + i + (s >> ((5 * i) % 26)));
	}
	rijndaelKeySetupEnc(state->rk, key, 128);
	rijndaelEncrypt(state->rk, NR, state->block, state->block);

	return;
}