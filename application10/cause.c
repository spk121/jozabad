/*
    cause.c - causes for diagnostic messages

    Copyright 2013--2015 Michael L. Gran <spk121@yahoo.com>

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

#include <glib.h>
#include "cause.h"

const char *clear_cause_name(clear_cause_t c)
{
    switch (c) {
    case c_clear__client_originated:
      return "CLIENT ORIGINATED";
      break;
    case c_clear__number_busy:
      return "NUMBER BUSY";
      break;
    case c_clear__out_of_order:
      return "OUT OF ORDER";
      break;
    case c_clear__remote_procedure_error:
      return "REMOTE PROCEDURE ERROR";
      break;
    case c_clear__reverse_charging_not_accepted:
      return "REVERSE CHARING NOT ACCEPTED";
      break;
    case c_clear__incompatible_destination:
      return "INCOMPATIBLE DESTINATION";
      break;
    case c_clear__fast_select_not_accepted:
      return "FAST SELECT NOT ACCEPTED";
      break;
    case c_clear__ship_absent:
      return "SHIP ABSENT";
      break;
    case c_clear__invalid_facility_request:
      return "INVALID FACILITY REQUEST";
      break;
    case c_clear__access_barred:
      return "ACCESS BARRED";
      break;
    case c_clear__local_procedure_error:
      return "LOCAL PROCEDURE ERROR";
      break;
    case c_clear__network_congestion:
      return "NETWORK CONGESTION";
      break;
    case c_clear__not_obtainable:
      return "NOT OBTAINABLE";
      break;
    case c_clear__roa_out_of_order:
      return "ROA OUT OF ORDER";
      break;
    }
    if ((unsigned)c & 0b10000000)
      return "CLIENT ORIGINATED";

    return "UNKNOWN CAUSE";
}

gboolean clear_cause_validate(clear_cause_t c)
{
    switch (c) {
    case c_clear__client_originated:
    case c_clear__number_busy:
    case c_clear__out_of_order:
    case c_clear__remote_procedure_error:
    case c_clear__reverse_charging_not_accepted:
    case c_clear__incompatible_destination:
    case c_clear__fast_select_not_accepted:
    case c_clear__ship_absent:
    case c_clear__invalid_facility_request:
    case c_clear__access_barred:
    case c_clear__local_procedure_error:
    case c_clear__network_congestion:
    case c_clear__not_obtainable:
    case c_clear__roa_out_of_order:
      return TRUE;
    }
    if ((unsigned)c & 0b10000000)
      return TRUE;

    return FALSE;
}

const char *reset_cause_name(reset_cause_t c)
{
    switch (c) {
    case c_reset__client_originated:
      return "CLIENT ORIGINATED";
      break;
    case c_reset__out_of_order:
      return "OUT OF ORDER";
      break;
    case c_reset__remote_procedure_error:
      return "REMOTE PROCEDURE ERROR";
      break;
    case c_reset__local_procedure_error:
      return "LOCAL PROCEDURE ERROR";
      break;
    case c_reset__network_congestion:
      return "NETWORK CONGESTION";
      break;
    case c_reset__remote_client_operational:
      return "REMOTE CLIENT OPERATIONAL";
      break;
    case c_reset__network_operational:
      return "NETWORK OPERATIONAL";
      break;
    case c_reset__incompatible_destination:
      return "INCOMPATIBLE DESTINATION";
      break;
    case c_reset__network_out_of_order:
      return "NETWORK OUT OF ORDER";
      break;
    }
    if ((unsigned)c & 0b10000000)
      return "CLIENT ORIGINATED";

    return "UNKNOWN CAUSE";
}

gboolean reset_cause_validate(reset_cause_t c)
{
    switch (c) {
    case c_reset__client_originated:
    case c_reset__out_of_order:
    case c_reset__remote_procedure_error:
    case c_reset__local_procedure_error:
    case c_reset__network_congestion:
    case c_reset__remote_client_operational:
    case c_reset__network_operational:
    case c_reset__incompatible_destination:
    case c_reset__network_out_of_order:
      return TRUE;
    }
    if ((unsigned)c & 0b10000000)
      return TRUE;

    return FALSE;
}

const char *restart_cause_name(restart_cause_t c)
{
    switch (c) {
    case c_restart__client_originated:
      return "CLIENT ORIGINATED";
      break;
    case c_restart__local_procedure_error:
      return "LOCAL PROCEDURE ERROR";
      break;
    case c_restart__network_congestion:
      return "NETWORK CONGESTION";
      break;
    case c_restart__network_operational:
      return "NETWORK OPERATIONAL";
      break;
    }
    if ((unsigned)c & 0b10000000)
      return "CLIENT ORIGINATED";

    return "UNKNOWN CAUSE";
}

gboolean restart_cause_validate(restart_cause_t c)
{
    switch (c) {
    case c_restart__client_originated:
    case c_restart__local_procedure_error:
    case c_restart__network_congestion:
    case c_restart__network_operational:
      return TRUE;
    }
    if ((unsigned)c & 0b10000000)
      return TRUE;

    return FALSE;
}
