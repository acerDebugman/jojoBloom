#include "../jojo/jojo.h"
#include "bloomfilter.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

#define WORD_BUF_SIZE 128
#define HASH_FUNCS 8
#define FILTER_SIZE 20

#define FILTER_BYTE_SIZE (2 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((2 << FILTER_SIZE) - 1)

int hashId = -1;

int main(int argc, char **argv){
	unsigned char filter[FILTER_BYTE_SIZE];
	int conflict;
	int i;
	char *pkey = (char *)"joe zhang";

	if(argc < 4)
		JojoUtil::err_sys("usage: %s dictionary words file, test words file, hashId\n", argv[0]);

	//initialization
	for(int i = 0; i < FILTER_BYTE_SIZE; i++){
		filter[i] = 0;
	}

	hashId = atoi(argv[3]);
	if(hashId > 7)
		JojoUtil::err_sys("hashId must less than 8");

	fprintf(stdout, "input file %s\n", argv[1]);
	fprintf(stdout, "test words %s\n", argv[2]);

	conflict = 0;
	//load words from file first
	loadwords(argv[1], filter, &conflict);

	//test key if in dict
	testwords(argv[2], filter);

//	insertwords(pkey, filter);
//	if(bloom_check(pkey, filter))
//		conflict++;

	fprintf(stdout, "the input conflict result is : %d\n", conflict);
}

void loadwords(char *file, unsigned char filter[], int *pConflict){
	int fd, n;
	char buf[BUFSIZ];
	char word[WORD_BUF_SIZE + 1];
	char * pos;
	int idx;

	if((fd = open(file, O_RDONLY)) < 0)
		JojoUtil::err_sys("open file error!\n");

	while((n = read(fd, buf, BUFSIZ)) > 0){
		for(pos = buf; pos < buf + n; pos++, idx++){
			if(idx > WORD_BUF_SIZE + 1)
				JojoUtil::err_sys("loadwords func: words too long!\n");

			if(*pos == '\n'){
				//end of the words
				word[idx] = 0;

				if(1 == bloom_check(word, filter))
					(*pConflict)++;

				insertwords(word, filter);

				idx = -1;
			}
			else
				word[idx] = *pos;
		}
	}

	if(n < 0)
		JojoUtil::err_sys("read file error!\n");

	close(fd);
}

void insertwords(char *key, unsigned char filter[]){
	unsigned int hash[HASH_FUNCS];
	for(int i = 0; i < HASH_FUNCS; i++){
		hash[i] = 0;
	}

	hashkey(key, hash);
	if(-1 == hashId){
		for(int i = 0; i < HASH_FUNCS; i++){
			//hash functions, ^ is xor,
			hash[i] = (hash[i] >> FILTER_SIZE) ^ (hash[i] & FILTER_BITMASK); //random number less than 2^20
			//set the bit
			filter[hash[i] >> 3] |= (1 << (hash[i] & 7));
		}
	}
	else{
		//hash functions, ^ is xor,
		hash[hashId] = (hash[hashId] >> FILTER_SIZE) ^ (hash[hashId] & FILTER_BITMASK); //random number less than 2^20
		//set the bit
		filter[hash[hashId] >> 3] |= (1 << (hash[hashId] & 7));
	}
}

void hashkey(char * key, unsigned int hash[]){
	unsigned char * str = (unsigned char*)key;
	int len = strlen(key);
	hash[0] = BKDRHash(str, len);
	hash[1] = RSHash(str, len);
	hash[2] = DJBHash(str, len);
	hash[3] = FNVHash(str, len);
	hash[4] = JSHash(str, len);
	hash[5] = PJWHash(str, len);
	hash[6] = SDBMHash(str, len);
	hash[7] = DEKHash(str, len);
}

int bloom_check(char *key, unsigned char filter[]){
	unsigned int hash[HASH_FUNCS];
	for(int i = 0; i < HASH_FUNCS; i++){
		hash[i] = 0;
	}

	hashkey(key, hash);
	if(-1 == hashId){
		for(int i = 0; i < HASH_FUNCS; i++){
			hash[i] = (hash[i] >> FILTER_SIZE) ^ (hash[i] & FILTER_BITMASK);
			if(0 == (filter[hash[i] >> 3] & (1 << (hash[i] & 7)))) //in a revert way
				return 0;
		}
	}
	else{
		hash[hashId] = (hash[hashId] >> FILTER_SIZE) ^ (hash[hashId] & FILTER_BITMASK);
		if(0 == (filter[hash[hashId] >> 3] & (1 << (hash[hashId] & 7)))) //in a revert way
			return 0;
	}

	return 1;
}

void testwords(char *file, unsigned char filter[]){
	int fd;
	int n;
	char buf[BUFSIZ];
	int hits = 0;
	char word[WORD_BUF_SIZE + 1];
	int idx;

	if((fd = open(file, O_RDONLY)) < 0)
		JojoUtil::err_sys("read file error!");

	idx = 0;
	while((n = read(fd, buf, BUFSIZ)) > 0){
		for(int i = 0; i < n; idx++, i++){
			if(idx > WORD_BUF_SIZE + 1)
				JojoUtil::err_sys("testwords func: word too long!");

			if(buf[i] == '\n'){
				word[idx] = 0;
				if(bloom_check(word, filter))
					hits++;

				idx = -1;
			}
			else
				word[idx] = buf[i];
		}
	}

	printf("test data hits %d\n", hits);

	close(fd);
}


//hash funcitons! this one is best, simple and effecient!
unsigned int BKDRHash(unsigned char *key, unsigned int len){
	unsigned int seed = 131; //31 131 1313 13131 131313 etc.  //java String use 31 as seed
	unsigned int hash = 0;

	for(int i = 0; i < len; key++, i++){
		hash = hash * seed + (*key);
	}

	return (hash & 0x7fffffff);
}

unsigned int RSHash(unsigned char *str, unsigned int len){
	unsigned int hash = 1315423911;
	unsigned int i = 0;

	for(int i = 0; i < len; str++, i++){
		hash ^= ((hash << 5) + (*str) + (hash >> 2));
	}

	return hash;
}
unsigned int DJBHash(unsigned char *str, unsigned int len){
	unsigned int hash = 5381;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
	  hash = ((hash << 5) + hash) + (*str);
	}

	return hash;
}
unsigned int FNVHash(unsigned char *str, unsigned int len){
	const unsigned int fnv_prime = 0x811C9DC5;
	unsigned int hash      = 0;
	unsigned int i         = 0;

	for(i = 0; i < len; str++, i++)
	{
	  hash *= fnv_prime;
	  hash ^= (*str);
	}

	return hash;
}
unsigned int JSHash(unsigned char * str, unsigned int len){
	unsigned int hash = 1315423911;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
	  hash ^= ((hash << 5) + (*str) + (hash >> 2));
	}

	return hash;
}
unsigned int PJWHash(unsigned char * str, unsigned int len){
	const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
	const unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedInt  * 3) / 4);
	const unsigned int OneEighth         = (unsigned int)(BitsInUnsignedInt / 8);
	const unsigned int HighBits          = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
	unsigned int hash              = 0;
	unsigned int test              = 0;
	unsigned int i                 = 0;

	for(i = 0; i < len; str++, i++)
	{
		hash = (hash << OneEighth) + (*str);

		if((test = hash & HighBits)  != 0)
		{
			hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}

	return hash;
}
unsigned int SDBMHash(unsigned char * str, unsigned int len){
	unsigned int hash = 0;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++)
	{
	  hash = (*str) + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}
unsigned int DEKHash(unsigned char * str, unsigned int len){
	unsigned int hash = len;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++){
	  hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
	}
	return hash;
}

