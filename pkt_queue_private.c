#include "pkt_queue_private.h"
#include "pkt.h"
#include "pkt_private.h"
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

// Protocol markers
#define PKT_START 0x02
#define PKT_END 0x03
#define PKT_ESC 0x10
#define PKT_ESC_MASK 0x20

struct pkt_queue* pkt_queue_create(void)
{
  int error = 0;
  pthread_mutexattr_t mutexattr;
  struct pkt_queue* q = malloc(sizeof(struct pkt_queue) + MAX_PKT_SIZE);
  if (q == 0)
  {
    return q;
  }
  q->rawinputsize = 0;
  q->readstate = STATE_FIND_START;
  q->closed = 0;
  q->head = 0;
  q->tail = 0;
  error = pthread_mutexattr_init(&mutexattr);
  if (error != 0)
  {
    return 0;
  }
  error = pthread_mutex_init(&(q->mutex), &mutexattr);
  if (error != 0)
  {
    return 0;
  }
  pthread_mutexattr_destroy(&mutexattr);
  return q;
}

void pkt_queue_destroy(struct pkt_queue *queue)
{
  pthread_mutex_lock(&(queue->mutex));
  while (queue->head != 0)
  {
    struct pkt* nextPkt = queue->head->next;
    pkt_destroy(queue->head);
    queue->head = nextPkt;
  }
  pthread_mutex_unlock(&(queue->mutex));
  pthread_mutex_destroy(&(queue->mutex));

  // TODO Destroy the condvar
  free(queue);
}

void add_raw_byte(struct pkt_queue *queue, uint8_t datum)
{
  queue->rawinput[queue->rawinputsize] = datum;
  queue->rawinputsize = queue->rawinputsize + 1;
}

void add_pkt(struct pkt_queue *queue, struct pkt* packet)
{
  if (queue->tail == 0)
  {
    queue->head = packet;
    queue->tail = packet;
  }
  else
  {
    queue->tail->next = packet;
    queue->tail = packet;
  }
}

void pkt_queue_write_bytes(struct pkt_queue *queue, size_t len, const uint8_t *data)
{
  int pos = 0;
  uint8_t datum = 0;
  struct pkt *completed_pkt = 0;
  pthread_mutex_lock(&(queue->mutex));
  while (pos < len)
  {
    datum = data[pos];
    if (queue->readstate == STATE_FIND_END ||
        queue->readstate == STATE_ESCAPE_NEXT)
    {
      // TODO check for rawinputsize >= MAX_PKT_SIZE and rename it to max input size perhaps
    }
    switch (queue->readstate)
    {
    case STATE_FIND_START:
      if (datum == PKT_START)
      {
        queue->readstate = STATE_FIND_END;
      }
      pos = pos + 1;
      break;
      
    case STATE_FIND_END:
      if (datum == PKT_END)
      {
        queue->readstate = STATE_FINALIZE;
      }
      else
      {
        if (datum == PKT_START)
        {
          queue->rawinputsize = 0;
          queue->readstate = STATE_FIND_START;
        }
        else
        {
          if (datum == PKT_ESC)
          {
            queue->readstate = STATE_ESCAPE_NEXT;
          }
          else
          {
            add_raw_byte(queue, datum);
          }
          pos = pos + 1;
        }
      }
      break;

    case STATE_ESCAPE_NEXT:
      if (datum == PKT_START)
      {
        queue->readstate = STATE_FIND_START;
        queue->rawinputsize = 0;
      }
      else
      {
        if ((datum & PKT_ESC_MASK) == PKT_ESC_MASK)
        {
          // The byte is escaped properly and not a frame start, so use it
          // once the mask has been stripped.
          datum = datum & ~PKT_ESC_MASK;
          add_raw_byte(queue, datum);
          queue->readstate = STATE_FIND_END;
        }
        else
        {
          // Invalid sequence, discard the partial packet
          queue->readstate = STATE_FIND_START;
          queue->rawinputsize = 0;
        }
        pos = pos + 1;
      }
      break;
      
    case STATE_FINALIZE:
      completed_pkt = pkt_create(queue->rawinputsize, queue->rawinput);
      if (completed_pkt != 0)
      {
        add_pkt(queue, completed_pkt);
      }
      queue->rawinputsize = 0;
      queue->readstate = STATE_FIND_START;
      pos = pos + 1;
      break;
    };
  }
  pthread_mutex_unlock(&(queue->mutex));
}

void pkt_queue_close(struct pkt_queue *queue)
{
  pthread_mutex_lock(&(queue->mutex));
  queue->closed = 1;
  pthread_mutex_unlock(&(queue->mutex));
}

void pkt_queue_read_pkt(struct pkt_queue *queue, ssize_t *len, uint8_t *data)
{
  // TODO condvar wait for pkt available
  if (queue->head == 0)
  {
    *len = -1;
    return;
  }
  struct pkt* packet = queue->head;
  *len = packet->size;
  memcpy(data, packet->data, packet->size);
  queue->head = packet->next;
  if (queue->tail == packet)
  {
    queue->tail = 0;
  }
  pkt_destroy(packet);
}
