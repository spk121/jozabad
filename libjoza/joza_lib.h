#ifndef LIBJOZA_H_INCLUDE
#define LIBJOZA_H_INCLUDE

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define WINDOW_MIN 1U
#define WINDOW_MAX 32767U
#define WINDOW_DEFAULT 128U
    
typedef enum _cause_t {
    c_unspecified,
    c_worker_originated,
    c_number_busy,
    c_out_of_order,
    c_remote_procedure_error,
    c_reverse_charging_acceptance_not_subscribed,
    c_incompatible_destination,
    c_fast_select_acceptance_not_subscribed,
    c_ship_absent,
    c_invalid_facility_request,
    c_access_barred,
    c_local_procedure_error,
    c_network_congestion,
    c_not_obtainable,
    c_ROA_out_of_order,
    
    c_last = c_ROA_out_of_order
} cause_t;
    
typedef enum _diagnostic_t {
    d_unspecified,
    d_invalid_ps,
    d_invalid_pr,
        
    d_packet_type_invalid,
    d_packet_type_invalid_for_s1,
    d_packet_type_invalid_for_s2,
    d_packet_type_invalid_for_s3,
    d_packet_type_invalid_for_s4,
    d_packet_type_invalid_for_s5,
    d_packet_type_invalid_for_s6,
    d_packet_type_invalid_for_s7,
    d_packet_type_invalid_for_s8,
    d_packet_type_invalid_for_s9,

    d_packet_not_allowed,
    d_unidentifiable_packet,
    d_call_on_one_way_logical_channel,
    d_invalid_packet_type_on_permanent_virtual_circuit,
    d_packet_on_unassigned_logical_channel,
    d_reject_not_subscribed_to,
    d_packet_too_short,
    d_packet_too_long,
    d_invalid_general_format_specifier,
    d_restart_packet_with_non_zero_logical_channel,
    d_packet_type_not_compatible_with_facility,
    d_unauthorized_interrupt_confirmation,
    d_unauthorized_interrupt,
    d_unaurhorized_reject,
    d_TOA_NPI_address_subscription_facility_not_subscribed_to,

    d_time_expired,
    d_time_expired_for_incoming_call,
    d_time_expired_for_clear_indication,
    d_time_expired_for_reset_indication,
    d_time_expired_for_restart_indication,
    d_time_expired_for_call_deflection,
        
    d_call_set_up_or_clearing_problem,
    d_facility_code_not_allowed,
    d_facility_parameter_not_allowed,
    d_invalid_called_address,
    d_invalid_calling_address,
    d_invalid_facility_length,
    d_incoming_call_barred,
    d_no_logical_channel_available,
    d_call_collision,
    d_duplicate_facility_requested,
    d_non_zero_address_length,
    d_non_zero_facility_length,
   d_facility_not_provided_when_expected,
    d_maximum_number_of_call_redirections_exceeded,

    d_miscellaneous,
    d_improper_cause_code_from_worker,
    d_not_aligned_octet,
    d_inconsistent_q_bit_setting,
    d_NUI_problem,
    d_ICRD_problem,

    d_not_assigned,

    d_international_problem,
    d_remote_network_problem,
    d_international_protocol_problem,
    d_international_link_out_of_order,
    d_international_link_busy,
    d_transit_network_facility_problem,
    d_remote_network_facility_problem,
    d_international_routing_problem,
    d_temporary_routing_problem,
    d_unknown_called_DNIC,
    d_maintenance_action,

    d_last = d_maintenance_action
} diagnostic_t;
    
// Note that bit 0 is input barred and bit 1 is output barred
typedef enum _direction_t {
    direction_bidirectional = 0,
    direction_input_barred = 1,
    direction_output_barred = 2,
    direction_io_barred = 3,

    direction_default = direction_bidirectional,
    direction_last = direction_io_barred
} direction_t;
    
typedef enum _packet_t {
    p_unspecified = 0,
    p_reserved = 1,
    p_reserved2 = 2,
    p_reserved3 = 3,
    p_16_bytes = 4,
    p_32_bytes = 5,
    p_64_bytes = 6,
    p_128_bytes = 7,
    p_256_bytes = 8,
    p_512_bytes = 9,
    p_1_Kbyte = 10,
    p_2_Kbytes = 11,
    p_4_Kbytes = 12,
        
    p_first = p_16_bytes,
    p_default = p_128_bytes,
    p_last = p_4_Kbytes
} packet_t;

typedef enum _throughput_t {
    t_unspecified,
    t_reserved_1,
    t_reserved_2,
    t_75bps,
    t_150bps,
    t_300bps,
    t_600bps,
    t_1200bps,
    t_2400bps,
    t_4800bps,
    t_9600bps,
    t_19200bps,
    t_48kpbs,
    t_64kbps,
    t_128kbps,
    t_192kbps,
    t_256kbps,
    t_320kbps,
    t_384kbps,
    t_448kbps,
    t_512kbps,
    t_578kbps,
    t_640kbps,
    t_704kbps,
    t_768kbps,
    t_832kbps,
    t_896kbps,
    t_960kbps,
    t_1024kbps,
    t_1088kbps,
    t_1152kbps,
    t_1216kbps,
    t_1280kbps,
    t_1344kbps,
    t_1408kbps,
    t_1472kbps,
    t_1536kbps,
    t_1600kbps,
    t_1664kbps,
    t_1728kbps,
    t_1792kbps,
    t_1856kbps,
    t_1920kbps,
    t_1984kbps,
    t_2048kbps,
        
    t_first = t_75bps,
    t_default = t_64kbps,
    t_last = t_2048kbps
} throughput_t;
    
const char *cause_name(cause_t c);
const char *diagnostic_name(diagnostic_t c);
const char *direction_name(direction_t d);
bool direction_validate(direction_t d);
direction_t direction_negotiate(direction_t r, direction_t c, bool *valid);
const char *packet_name(packet_t c);
bool packet_validate(packet_t p);
packet_t packet_throttle(packet_t request, packet_t limit);
packet_t packet_negotiate(packet_t request, packet_t c, bool *valid);
uint32_t packet_bytes(packet_t p);
const char *throughput_name(throughput_t c);
bool throughput_validate(throughput_t p);
throughput_t throughput_throttle(throughput_t request, throughput_t limit);
throughput_t throughput_negotiate(throughput_t r, throughput_t c, bool *valid);
uint32_t throughput_bps(throughput_t p);
bool window_validate(uint16_t i);
uint16_t window_throttle(uint16_t request, uint16_t limit);
uint16_t window_negotiate(uint16_t request, uint16_t current, bool *valid);

#ifdef	__cplusplus
}
#endif

#endif

