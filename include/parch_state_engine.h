#ifndef __STATE_ENGINE_H_INCLUDED__
#define __STATE_ENGINE_H_INCLUDED__

#include "parch_broker.h"

#ifdef __cplusplus
extern "C" {
#endif

// data structures, macros, typedefs, functions

//  ---------------------------------------------------------------------------
//  MACROS

#define MAX_SERVICE_NAME_LEN 128
#define WINDOW_MIN 1
#define WINDOW_MAX 32768
#define WINDOW_DEFAULT 128
#define PACKET_CLASS_MIN 3            // 16 bytes
#define PACKET_CLASS_MAX 12           // 4  kbytes
#define PACKET_CLASS_DEFAULT 7        // 128 bytes
#define BUCKET_DRAIN_TIME 5.0         // seconds

//  ---------------------------------------------------------------------------
//  TYPEDEFS

// These mostly come from TABLE 5-7/X.25

enum _clearing_cause_t {
    number_busy = 1,
    out_of_order = 9,
    remote_procedure_error = 17,
    // reverse_charging_acceptance_not_subscribed = 25,
    incompatible_destination = 33,
    // fast_select_acceptance_not_subscribed = 41,
    // ship_absent = 57,                  // LOL. This is in X.25, I swear.
    invalid_facility_request = 3,         // bad CONNECT or CALL_REQUEST parameter
    access_barred = 11,
    local_procedure_error = 19,
    network_congestion = 5,
    not_obtainable = 13,
    // roa_out_of_order = 21
};

typedef enum _clearing_cause_t clearing_cause_t;



//  Create a new state engine instance
CZMQ_EXPORT parch_state_engine_t *
    parch_state_engine_new(broker_t *broker, node_t * node, byte incoming_barred, byte outgoing_barred, byte throughput);


//  Destroy a state engine instance
CZMQ_EXPORT void
    parch_state_engine_destroy (parch_state_engine_t **self_p);

int
parch_state_engine_x_message (parch_state_engine_t *self, parch_msg_t *msg);


//  Self test of this class
void
    parch_state_engine_test (bool verbose);

#ifdef __cplusplus
}
#endif

#endif
