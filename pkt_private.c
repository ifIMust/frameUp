#include "pkt_private.h"
#include <stdlib.h>
#include <string.h>

struct pkt* pkt_create(size_t n, uint8_t *data)
{
  struct pkt* packet = malloc(sizeof(struct pkt));
  if (packet == 0)
  {
    return packet;
  }
  packet->next = 0;
  packet->data = malloc(n);
  memcpy(packet->data, data, n);
  packet->size = n;
  return packet;
}

void pkt_destroy(struct pkt* p)
{
  free(p);
}
