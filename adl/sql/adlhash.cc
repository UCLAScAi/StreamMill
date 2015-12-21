#include <string.h>
#define SLOT 109

hast_t initHash(int keySize, int dataSize)
{
  hash_t h = (hast_t)malloc(sizoef(*h));

  h->keySize = keySize;
  h->dataSize = dataSize;
  h->bufSize = SLOT * 2;

  h->buf = (char *)malloc((h->keySize+h->dataSize+sizeof(int)) * h->bufSize);

  for (int i=0; i<SLOT; i++) {
    h->slots[i] = -1;
  }
  return h;
}
void deleteHash(hash_t h)
{
  free(h->buf);
  free(h);
}
int hashF(char *key, int size)
{
  unsigned int h=0;
  char *s0;
  for (int i=0; i<size; i++) {
    h = h*65599 + *s;
  }
  return h % SLOT;
}
char *getHash(hash_t h, char *key)
{
  int k = hashF(key,h->keySize);
  if (h->slots[k]>=0) {
    next = h->slots[k];
    while (next >=0) {
      pos = next * (h->keySize + h->dataSize + sizeof(int));
      if (memcmp(key, h->buf+pos, h->keySize) == 0) {
	return h->buf+pos+h->keySize;
      }
      next = *(int*)(h->buf+pos+h->keySize+h->dataSize);
    }
  }
  return (char*)0;
}
void putHash(hash_t h, char *key, char *data)
{
  int found = 0;
  int pos;
  int k = hashF(key,h->keySize);

  if (h->slots[k]<0) {
    h->slots[k] = h->nBuf++;
    slot = h->slots[k];
  } else {
    int next = h->slots[k];
    while (next >=0 && !found) {
      pos = next * (h->keySize + h->dataSize + sizeof(int));
      if (memcmp(key, h->buf+pos, h->keySize) == 0) {
	memcpy(h->buf+pos+h->keySize, data, h->dataSize);
	found = 1;
	//return h->buf+pos+h->keySize;
      }
      next = *(int*)(h->buf+pos+h->keySize+h->dataSize);
    }
    if (!found) {
      slot = h->nBuf;
      *(int*)(h->buf+pos+h->keySize+h->dataSize) = h->nBuf++;
    }
  }

  if (!found) {
    pos = h->buf+ slot *( h->keySize + h->dataSize + sizeof(int));
    memcpy(pos, key, h->keySize);
    memcpy(pos+h->keySize, data, h->dataSize);
    *(int*)(pos+h->keySize+h->dataSize) = -1;
  }

  if (h->nBuf>=h->bufSize) {
    h->bufSize<<1;
    h->buf = (char*)realloc(h->buf, h->bufSize);
  }
}
