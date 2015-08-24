#ifndef AES_H
#define AES_H

#include <openssl/aes.h>
#include <openssl/rand.h> 
#include <openssl/hmac.h>
#include <openssl/buffer.h>
#include <string.h>
#include <math.h>

struct ctr_state 
{ 
	unsigned char ivec[AES_BLOCK_SIZE];	 
	unsigned int num; 
	unsigned char ecount[AES_BLOCK_SIZE]; 
}; 

int init_ctr(struct ctr_state *state, const unsigned char iv[16]);
void aesEncrypt(char* read, char* write, const unsigned char* enc_key);
void aesDecrypt(char* read, char* write, const unsigned char* enc_key);

#endif
