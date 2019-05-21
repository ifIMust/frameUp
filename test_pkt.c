#include "pkt.h"
#include "pkt_private.h" //TODO fixify

int main()
{
  uint8_t data[] = { 0x99, 0x09, 0x69, 0x33 };
  pkt_t* packet = pkt_create(sizeof(data), data);
  if (packet == 0)
  {
    return 1;
  }

  if ((*packet).size != 4)
  {
    return 2;
  }

  for (int i = 0; i < 4; i = i + 1)
  {
    if ((*packet).data[i] != data[i])
    {
      return 3;
    }
  }
  pkt_destroy(packet);
  return 0;
}
