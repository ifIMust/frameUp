#include "pkt_queue.h"

#define MAX_DECODED_PKT_LENGTH 256

int testMultiplePacketStart();
int testGivenExample();
int testIncompleteFrameMidEscapeEnd();
int testIncompleteFrameMidEscapeStart();
int testTwoPacketsInOneWrite();

int main()
{
  const int numTests = 5;
  int result[numTests];
  result[0] = testMultiplePacketStart();
  result[1] = testGivenExample();
  result[2] = testIncompleteFrameMidEscapeEnd();
  result[3] = testIncompleteFrameMidEscapeStart();
  result[4] = testTwoPacketsInOneWrite();
  for (int i = 0; i < numTests; i = i + 1)
  {
    if (result[i] != 0)
    {
      return result[i];
    }
  }
}

int testMultiplePacketStart()
{
  uint8_t datums[MAX_DECODED_PKT_LENGTH];
  ssize_t bytes_read = 0;
  uint8_t data[] = { 0x02, 0x04, 0x02, 0x56, 0x03 };
  pkt_queue_t* q = pkt_queue_create();
  pkt_queue_write_bytes(q, sizeof(data), data);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 1)
  {
    return 1;
  }
  if (datums[0] != 0x56)
  {
    return 2;
  }
  pkt_queue_close(q);
  pkt_queue_destroy(q);
  return 0;
}

int testGivenExample()
{
  const uint8_t bytestream[] = {0x02, 0x10, 0x30, 0xFF, 0x03};
  struct pkt_queue *q = NULL;
  ssize_t pkt_len = 0;
  uint8_t pkt_buffer[MAX_DECODED_PKT_LENGTH];

  q = pkt_queue_create();
  if (q == 0)
  {
    return 3;
  }
  
  pkt_queue_write_bytes(q, sizeof(bytestream), bytestream);
  pkt_queue_close(q);

  pkt_queue_read_pkt(q, &pkt_len, pkt_buffer);
  if (pkt_len != 2)
  {
    return 4;
  }

  if (pkt_buffer[0] != 0x10 || pkt_buffer[1] != 0xFF)
  {
    return 5;
  }

  pkt_queue_read_pkt(q, &pkt_len, pkt_buffer);
  if (pkt_len != -1)
  {
    return 6;
  }
  pkt_queue_destroy(q);
  return 0;
}

/* Test that a packet-end that follows an escape causes the partial packet to be dropped. */
int testIncompleteFrameMidEscapeEnd()
{
  uint8_t datums[MAX_DECODED_PKT_LENGTH];
  ssize_t bytes_read = 0;
  uint8_t data[] = { 0x02, 0x04, 0x05, 0x10, 0x03, 0x03, 0x02, 0x56, 0x03 };
  pkt_queue_t* q = pkt_queue_create();
  pkt_queue_write_bytes(q, sizeof(data), data);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 1)
  {
    return 7;
  }
  if (datums[0] != 0x56)
  {
    return 8;
  }
  pkt_queue_close(q);
  pkt_queue_destroy(q);
  return 0;
};

/* Test that a packet-start that follows an escape causes the partial packet to be dropped. */
/* Question: How can I tell whether a packet is improperly encoded versus incomplete? */
/* Incomplete: finding a 0x02 after an escape means begin a new packet immediately. */
/* Improperly encoded: finidng a 0x02 after an escape means drop everything and look for the next 0x02. */
// uint8_t data[] = { 0x02, 0x10, 0x02, 0x42, 0x03, 0x02, 0x56, 0x03 };
int testIncompleteFrameMidEscapeStart()
{
  uint8_t datums[MAX_DECODED_PKT_LENGTH];
  ssize_t bytes_read = 0;
  uint8_t data[] = { 0x02, 0x10, 0x02, 0x42, 0x03, 0x02, 0x56, 0x03 };
  pkt_queue_t* q = pkt_queue_create();
  pkt_queue_write_bytes(q, sizeof(data), data);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 1)
  {
    return 17;
  }
  if (datums[0] != 0x42)
  {
    return 18;
  }
  pkt_queue_close(q);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 1)
  {
    return 19;
  }
  if (datums[0] != 0x56)
  {
    return 20;
  }
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != -1)
  {
    return 21;
  }
  pkt_queue_destroy(q);
  return 0;
};

/* Test that packets are queued properly */
int testTwoPacketsInOneWrite()
{
  uint8_t datums[MAX_DECODED_PKT_LENGTH];
  ssize_t bytes_read = 0;
  uint8_t data[] = { 0x02, 0x04, 0x05, 0x03, 0x02, 0x56, 0x03 };
  pkt_queue_t* q = pkt_queue_create();
  pkt_queue_write_bytes(q, sizeof(data), data);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 2)
  {
    return 9;
  }
  if (datums[0] != 0x04 || datums[1] != 0x05 )
  {
    return 10;
  }
  pkt_queue_close(q);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 1)
  {
    return 11;
  }
  if (datums[0] != 0x56)
  {
    return 12;
  }
  pkt_queue_destroy(q);
  return 0;
};
