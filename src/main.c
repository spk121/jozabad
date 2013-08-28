

// Features
//  - FLOW_CONTROL -- disables reset messages, RR and RNR
//  - PACKET_SIZE -- negotiation
//  - THROUGHPUT -- negotiation
//  - DIRECTIONALITY -- disable call barring
//  - TIMEOUTS -- disable timeouts

/*
w/o all that
connect - register worker
cal connect - make logical channel



connect, add to worker directory
call connect, find peer, creqate logical channel, add hashes to channel, state


message comes in.
hash address,
check logical channel to see if address is x or y
perform actions according to state

--
connect request
  hash address
  check logical channel
  if found
     do state machine action
  else
     check directory
     if found
         do error
     else
         do registration


call request
    hash address
    check logical channel
    if found
        do state machine action
    else
        check directory
        if found
            do call
        else
        reject */

#include "../include/svc.h"

uint8_t broker_packet_size_index_limit = PACKET_INDEX_MAX;
uint8_t broker_throughput_index_limit = THROUGHPUT_INDEX_MAX;
uint16_t broker_window_limit = WINDOW_MAX;

int loglevel = 5;

int main (int argc, char *argv[]) {
    NOTE("entering main()");
    NOTE("argc = %d", argc);
    for (int i = 0; i < argc; i ++) {
        NOTE("argv[%d] = '%s'", i, argv[i]);
    }
    
    channel_store_init(&channel_store);
    connection_store_init(&connection_store);
    poll_init(true, "tcp://*:5555");
    poll_start();
    // finalize_poll();
    return 0;
}
