/*
    cause.h - general categories of errors

    Copyright 2013, 2014 Michael L. Gran <spk121@yahoo.com>

    This file is part of Jozabad.

    Jozabad is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Jozabad is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Jozabad.  If not, see <http://www.gnu.org/licenses/>.

*/

/**
 * @file cause.h
 * @brief List of error categories for diagnostic messages
 *
 * Diagnostic messages have a general 'cause' and a more specific
 * 'diagnostic'.
 */

#ifndef JOZA_CAUSE_H
#define JOZA_CAUSE_H

// These are general error categories for DIAGNOSTIC, RESET, and CLEAR
// messages.
/**
 * @brief The list of error categories.
 *
 * In DIAGNOSTIC, RESET, and CLEAR messages, an error category is supplied
 * to indicate the reason for the message.
 */

// In a client-originated CLEAR REQUEST, the client may set the cause to
// either 0 == c_client_originated, or 128 to 255, where only the top bit
// is significant from the point of view of the server.

// In a server-originated CLEAR REQUEST, the clearing causes are as
// follows,

// See Table 5-7/X.25

typedef enum {
  c_clear__client_originated = 0,

  c_clear__number_busy = 1,
  c_clear__out_of_order = 9,
  c_clear__remote_procedure_error = 17,
  c_clear__reverse_charging_not_accepted = 25,
  c_clear__incompatible_destination = 33,
  c_clear__fast_select_not_accepted = 41,
  c_clear__ship_absent = 57,

  c_clear__invalid_facility_request = 3,
  c_clear__access_barred = 11,
  c_clear__local_procedure_error = 19,

  c_clear__network_congestion = 5,
  c_clear__not_obtainable = 13,
  c_clear__roa_out_of_order = 21,
} clear_cause_t;

// In a client-originated RESET REQUEST, the clearing cause may be 0
// or may be 128 to 255, of which only the top bit is checked by the
// server.  The other 7 bits may be used by the client.

// In a server-originated RESET REQUEST, the clearing cause is one
// of the following

// Ref Table 5-8/X.25
typedef enum {
  c_reset__client_originated = 0,
  
  c_reset__out_of_order = 1,
  c_reset__remote_procedure_error = 3,
  c_reset__local_procedure_error = 5,
  c_reset__network_congestion = 7,
  c_reset__remote_client_operational = 9,
  c_reset__network_operational = 15,
  c_reset__incompatible_destination = 17,
  c_reset__network_out_of_order = 29,
} reset_cause_t;

// In a client-originated RESTART REQUEST, the clearing cause may be 0
// or may be 128 to 255, of which only the top bit is checked by the
// server.  The other 7 bits may be used by the client.

// In a server-originated RESTART REQUEST, the clearing cause is one
// of the following.

// Set Table 5-9/X.25
typedef enum {
  // These are the cause codes for RESTART packets
  c_restart__client_originated = 0,
  c_restart__local_procedure_error = 1,
  
  c_restart__network_congestion = 3,
  c_restart__network_operational = 7,
} restart_cause_t;

#define CAUSE_IS_CLIENT_ORIGINATED(x) (((x) == 0) || (((x) >= 128) && ((x) <= 255)))

const char *clear_cause_name(clear_cause_t c);
gboolean clear_cause_validate(clear_cause_t c);
const char *reset_cause_name(reset_cause_t c);
gboolean reset_cause_validate(reset_cause_t c);
const char *restart_cause_name(restart_cause_t c);
gboolean restart_cause_validate(restart_cause_t c);

#endif
