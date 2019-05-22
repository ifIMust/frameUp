#include "pkt_queue.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define MAX_DECODED_PKT_LENGTH 256

int testMultiplePacketStart();
int testGivenExample();
int testIncompleteFrameMidEscapeEnd();
int testIncompleteFrameMidEscapeStart();
int testTwoPacketsInOneWrite();
int testNoPacketsAvailable();
int testCloseWhileReadBlocked();
int testWriteAfterClose();

int main()
{
  const int numTests = 8;
  int result[numTests];
  result[0] = testMultiplePacketStart();
  result[1] = testGivenExample();
  result[2] = testIncompleteFrameMidEscapeEnd();
  result[3] = testIncompleteFrameMidEscapeStart();
  result[4] = testTwoPacketsInOneWrite();
  result[5] = testNoPacketsAvailable();
  result[6] = testCloseWhileReadBlocked();
  result[7] = testWriteAfterClose();
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
    pkt_queue_destroy(q);
    return 1;
  }
  if (datums[0] != 0x56)
  {
    pkt_queue_destroy(q);
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
    pkt_queue_destroy(q);
    return 3;
  }
  
  pkt_queue_write_bytes(q, sizeof(bytestream), bytestream);
  pkt_queue_close(q);

  pkt_queue_read_pkt(q, &pkt_len, pkt_buffer);
  if (pkt_len != 2)
  {
    pkt_queue_destroy(q);
    return 4;
  }

  if (pkt_buffer[0] != 0x10 || pkt_buffer[1] != 0xFF)
  {
    pkt_queue_destroy(q);
    return 5;
  }

  pkt_queue_read_pkt(q, &pkt_len, pkt_buffer);
  if (pkt_len != -1)
  {
    pkt_queue_destroy(q);
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
    pkt_queue_destroy(q);
    return 7;
  }
  if (datums[0] != 0x56)
  {
    pkt_queue_destroy(q);
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
    pkt_queue_destroy(q);
    return 17;
  }
  if (datums[0] != 0x42)
  {
    pkt_queue_destroy(q);
    return 18;
  }
  pkt_queue_close(q);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 1)
  {
    pkt_queue_destroy(q);
    return 19;
  }
  if (datums[0] != 0x56)
  {
    pkt_queue_destroy(q);
    return 20;
  }
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != -1)
  {
    pkt_queue_destroy(q);
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
    pkt_queue_destroy(q);
    return 9;
  }
  if (datums[0] != 0x04 || datums[1] != 0x05 )
  {
    pkt_queue_destroy(q); 
    return 10;
  }
  pkt_queue_close(q);
  pkt_queue_read_pkt(q, &bytes_read, datums);
  if (bytes_read != 1)
  {
    pkt_queue_destroy(q);
    return 11;
  }
  if (datums[0] != 0x56)
  {
    pkt_queue_destroy(q);
    return 12;
  }
  pkt_queue_destroy(q);
  return 0;
};

struct threadDataForWrite
{
  pkt_queue_t *q;
  uint8_t* data;
};

void writeBytes(void* thrData)
{
  struct threadDataForWrite* t = (struct threadDataForWrite*)thrData;
  pkt_queue_write_bytes(t->q, sizeof(t->data), t->data);
}

/* Test that a read will block until a packet becomes available */
int testNoPacketsAvailable()
{
  struct threadDataForWrite thrData;
  const uint8_t data[] = { 0x02, 0x04, 0x56, 0x03 };
  uint8_t datums[MAX_DECODED_PKT_LENGTH];
  ssize_t bytes_read = 0;
  pkt_queue_t* q = pkt_queue_create();
  pthread_t thread;
  int error = 0;
  thrData.q = q;
  thrData.data = malloc(sizeof(data));
  memcpy(thrData.data, data, sizeof(data));
  error = pthread_create(&thread, 0, (void*)(&writeBytes), &thrData);
  if (error != 0)
  {
    pkt_queue_destroy(q);
    return error;
  }
  pkt_queue_read_pkt(q, &bytes_read, datums);
  error = pthread_join(thread, 0);
  if (error != 0)
  {
    pkt_queue_destroy(q);
    return error;
  }

  if (bytes_read != 2)
  {
    pkt_queue_destroy(q);
    return 13;
  }
  pkt_queue_destroy(q);
  return 0;
}

struct threadDataForRead
{
  pkt_queue_t *q;
  ssize_t bytes_read;
  uint8_t data[32];
};

void readBytes(void* thrData)
{
  struct threadDataForRead* t = (struct threadDataForRead*)thrData;
  pkt_queue_read_pkt(t->q, &(t->bytes_read), t->data);
}

/* Test correct behavior when the queue is closed when another thread is blocked on a read */
int testCloseWhileReadBlocked()
{
  struct threadDataForRead thrData;
  pkt_queue_t* q = pkt_queue_create();
  pthread_t thread;
  int error = 0;
  thrData.q = q;
  error = pthread_create(&thread, 0, (void*)(&readBytes), &thrData);
  if (error != 0)
  {
    pkt_queue_destroy(q);
    return error;
  }
  sleep(1);
  pkt_queue_close(q);
  error = pthread_join(thread, 0);
  if (error != 0)
  {
    pkt_queue_destroy(q);
    return error;
  }

  if (thrData.bytes_read != -1)
  {
    pkt_queue_destroy(q);
    return 23;
  }
  pkt_queue_destroy(q);
  return 0;
}

/* Test that writing to a queue after it has been closed will not produce further packets */
int testWriteAfterClose()
{
  const uint8_t bytestream[] = {0x02, 0x10, 0x30, 0xFF, 0x03};
  uint8_t pkt_buffer[MAX_DECODED_PKT_LENGTH];
  ssize_t bytes_read = 0;
  pkt_queue_t* q = pkt_queue_create();
  pkt_queue_close(q);
  pkt_queue_write_bytes(q, sizeof(bytestream), bytestream);
  pkt_queue_read_pkt(q, &bytes_read, pkt_buffer);
  if (bytes_read != -1)
  {
    pkt_queue_destroy(q);
    return 22;
  }
  pkt_queue_destroy(q);
  return 0;
}

/* Test that a huge packet will be dropped, also test many small writes. */
int testWriteExcessivelyLongPacket()
{
  pkt_queue_t* q = pkt_queue_create();
  const uint8_t properPacket[] = { 0x02, 0x12, 0x23, 0x03 };
  uint8_t datum = 0x02;
  uint8_t pkt_buffer[MAX_DECODED_PKT_LENGTH];
  ssize_t bytes_read = 0;
  pkt_queue_write_bytes(q, 1, &datum);
  datum = 0xEE;
  for (int i = 0; i < MAX_DECODED_PKT_LENGTH + 3; i = i + 1)
  {
    pkt_queue_write_bytes(q, 1, &datum);
  }
  datum = 0x03;
  pkt_queue_write_bytes(q, 1, &datum);
  pkt_queue_write_bytes(q, sizeof(properPacket), properPacket);
  pkt_queue_read_pkt(q, &bytes_read, pkt_buffer);
  if (bytes_read != 2)
  {
    pkt_queue_destroy(q);
    return 25;
  }
  pkt_queue_destroy(q);
  return 0;
}
