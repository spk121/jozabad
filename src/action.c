#include "../include/svc.h"

const action_t x_action_table[(size_t) state_max + 1][PARCH_MSG_DISCONNECT_INDICATION + 1] = {
    [state_ready] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_x_call_request,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_x_call_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_y_call_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_x_call_collision,
        [PARCH_MSG_CALL_ACCEPTED] = a_x_call_accepted,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_data_transfer] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_x_data,
        [PARCH_MSG_RR] = a_x_rr,
        [PARCH_MSG_RNR] = a_x_rnr,
        [PARCH_MSG_RESET_REQUEST] = a_x_reset,
        [PARCH_MSG_RESET_CONFIRMATION] = a_reset,
        [PARCH_MSG_CONNECT] = a_reset,
        [PARCH_MSG_CONNECT_INDICATION] = a_reset,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_reset
    },
    [state_call_collision] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_x_clear_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_discard,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_y_clear_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_discard,
        [PARCH_MSG_CALL_ACCEPTED] = a_discard,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_confirmation,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_x_clear_confirmation,
        [PARCH_MSG_DATA] = a_discard,
        [PARCH_MSG_RR] = a_discard,
        [PARCH_MSG_RNR] = a_discard,
        [PARCH_MSG_RESET_REQUEST] = a_discard,
        [PARCH_MSG_RESET_CONFIRMATION] = a_discard,
        [PARCH_MSG_CONNECT] = a_discard,
        [PARCH_MSG_CONNECT_INDICATION] = a_discard,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_discard
    },
    [state_x_reset_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_x_call_request,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_reset,
        [PARCH_MSG_RR] = a_reset,
        [PARCH_MSG_RNR] = a_reset,
        [PARCH_MSG_RESET_REQUEST] = a_discard,
        [PARCH_MSG_RESET_CONFIRMATION] = a_reset,
        [PARCH_MSG_CONNECT] = a_reset,
        [PARCH_MSG_CONNECT_INDICATION] = a_reset,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_reset
    },
    [state_y_reset_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_x_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_discard,
        [PARCH_MSG_RR] = a_discard,
        [PARCH_MSG_RNR] = a_discard,
        [PARCH_MSG_RESET_REQUEST] = a_x_reset_confirmation,
        [PARCH_MSG_RESET_CONFIRMATION] = a_x_reset_confirmation,
        [PARCH_MSG_CONNECT] = a_discard,
        [PARCH_MSG_CONNECT_INDICATION] = a_discard,
        [PARCH_MSG_DISCONNECT] = a_x_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_discard
    }
};

const action_t y_action_table[(size_t) state_max + 1][PARCH_MSG_DISCONNECT_INDICATION + 1] = {
    [state_ready] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_y_call_request,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_x_call_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_y_call_collision,
        [PARCH_MSG_CALL_ACCEPTED] = a_y_call_accepted,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_y_call_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_data_transfer] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_y_data,
        [PARCH_MSG_RR] = a_y_rr,
        [PARCH_MSG_RNR] = a_y_rnr,
        [PARCH_MSG_RESET_REQUEST] = a_y_reset,
        [PARCH_MSG_RESET_CONFIRMATION] = a_reset,
        [PARCH_MSG_CONNECT] = a_reset,
        [PARCH_MSG_CONNECT_INDICATION] = a_reset,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_reset
    },
    [state_call_collision] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_y_call_accepted,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_x_clear_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_discard,
        [PARCH_MSG_CALL_ACCEPTED] = a_discard,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_confirmation,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_y_clear_confirmation,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_y_clear_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_y_call_request,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_discard,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_clear,
        [PARCH_MSG_RR] = a_clear,
        [PARCH_MSG_RNR] = a_clear,
        [PARCH_MSG_RESET_REQUEST] = a_clear,
        [PARCH_MSG_RESET_CONFIRMATION] = a_clear,
        [PARCH_MSG_CONNECT] = a_clear,
        [PARCH_MSG_CONNECT_INDICATION] = a_clear,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_clear
    },
    [state_x_reset_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_clear,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_discard,
        [PARCH_MSG_RR] = a_discard,
        [PARCH_MSG_RNR] = a_discard,
        [PARCH_MSG_RESET_REQUEST] = a_y_reset_confirmation,
        [PARCH_MSG_RESET_CONFIRMATION] = a_y_reset_confirmation,
        [PARCH_MSG_CONNECT] = a_discard,
        [PARCH_MSG_CONNECT_INDICATION] = a_discard,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_discard
    },
    [state_y_reset_request] =
    {
        [PARCH_MSG_CALL_REQUEST] = a_y_call_request,
        [PARCH_MSG_CALL_ACCEPTED] = a_clear,
        [PARCH_MSG_CLEAR_REQUEST] = a_y_clear_request,
        [PARCH_MSG_CLEAR_CONFIRMATION] = a_clear,
        [PARCH_MSG_DATA] = a_reset,
        [PARCH_MSG_RR] = a_reset,
        [PARCH_MSG_RNR] = a_reset,
        [PARCH_MSG_RESET_REQUEST] = a_discard,
        [PARCH_MSG_RESET_CONFIRMATION] = a_reset,
        [PARCH_MSG_CONNECT] = a_reset,
        [PARCH_MSG_CONNECT_INDICATION] = a_reset,
        [PARCH_MSG_DISCONNECT] = a_y_disconnect,
        [PARCH_MSG_DISCONNECT_INDICATION] = a_reset
    }
};
