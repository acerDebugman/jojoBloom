#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER

unsigned int BKDRHash(unsigned char *, unsigned int);
unsigned int RSHash(unsigned char *, unsigned int);
unsigned int DJBHash(unsigned char *, unsigned int);
unsigned int FNVHash(unsigned char *, unsigned int);
unsigned int JSHash(unsigned char *, unsigned int);
unsigned int PJWHash(unsigned char *, unsigned int);
unsigned int SDBMHash(unsigned char *, unsigned int);
unsigned int DEKHash(unsigned char *, unsigned int);

void loadwords(char * file, unsigned char filter[], int *confict);
void insertwords(char *key, unsigned char filter[]);
void hashkey(char * key, unsigned int hash[]);

int bloom_check(char *key, unsigned char filter[]);

void testwords(char *file, unsigned char filter[]);

#endif
