#ifndef PKT_QUEUE_PRIVATE_H_INCLUDED
#define PKT_QUEUE_PRIVATE_H_INCLUDED

#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>

#define MAX_PKT_SIZE 256
#define STATE_FIND_START  1
#define STATE_FIND_END    2
#define STATE_ESCAPE_NEXT 3
#define STATE_FINALIZE    4

struct pkt;

// This implementation uses a singly-linked list with tail tracking to implement the queue

struct pkt_queue
{
  size_t rawinputsize;
  int readstate;
  int closed;

  struct pkt* head;
  struct pkt* tail;

  pthread_mutex_t mutex;

  uint8_t rawinput[];
};

#endif
