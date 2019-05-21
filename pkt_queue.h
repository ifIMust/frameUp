#ifndef PKT_QUEUE_H_INCLUDED
#define PKT_QUEUE_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DECODED_PACKET_LEN (512)

typedef struct pkt_queue pkt_queue_t;


///////////////////////////////////////////////////////////////////////////////
// Construction/destruction

// Constructor for a pkt_queue
pkt_queue_t* pkt_queue_create(void);

// Destructor for a pkt_queue
void pkt_queue_destroy(pkt_queue_t *queue);


///////////////////////////////////////////////////////////////////////////////
// Write side

// Called on incoming, undecoded bytes to be translated into packets
void pkt_queue_write_bytes(pkt_queue_t *queue, size_t len, const uint8_t *data);

// Called when the connection is closed and there are no more bytes to write
void pkt_queue_close(pkt_queue_t *queue);


///////////////////////////////////////////////////////////////////////////////
// Read side

// This is a blocking call to read a packet up to MAX_DECODED_PACKET_LEN
// This blocks until a packet is available, and fills out the value pointed to
// len with the decoded packet length, and the buffer pointed to by data with
// the decoded packet contents.
// After a call to pkt_queue_close(), and all previous packets read out,
// this will set the value pointed to len to -1, and the 
void pkt_queue_read_pkt(pkt_queue_t *queue, ssize_t *len, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif //PKT_QUEUE_H_INCLUDED

