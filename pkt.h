#ifndef PKT_DEFINED
#define PKT_DEFINED

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pkt pkt_t;

pkt_t* pkt_create(size_t n, uint8_t *data);
void pkt_destroy(pkt_t*);

#ifdef __cplusplus
}
#endif

  
#endif
