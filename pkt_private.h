#ifndef PKT_PRIVATE_DEFINED
#define PKT_PRIVATE_DEFINED

#include <stdint.h>
#include <sys/types.h>

struct pkt
{
  struct pkt* next;
  uint8_t* data;
  size_t size;
};

#endif
