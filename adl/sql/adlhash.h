#ifndef ADL_HASH_H
#define ADL_HASH_H

#define SLOT	10131

typedef struct hash_s *hash_t;
struct hash_s
{
  char *buf;

  int bufSize;
  int nBuf;

  int keySize;
  int dataSize;
  int slots[SLOT];
};


hash_t initHash(int keySize, int dataSize);
void deleteHash(hash_t h);
char *getHash(hash_t h, char *key);
void putHash(hash_t h, char *key, char *data);

#endif
