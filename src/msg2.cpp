// CALL_REQUEST packet

// Calling address - 1 to 15 digit decimal number
// Called address - 1 to 15 digit decimal number
// Max Desired Packet Size - 1 byte enum
// Max Desired Window Size - 2 byte unsigned integer (1 to 32767)
// Throughput - 1 byte enum
// User data - 16 bytes free form

// CALL_ACCEPTED packet

// Max Desired Packet Size - 1 byte enum
// Max Desired Window Size - 2 byte unsigned integer (1 to 32767)
// Throughput - 1 byte enum
// User data - 16 bytes free form

// CLEAR_REQUEST packet
// Clearing Cause - 1 byte enum
// Diagnostic - 1 byte enum

// CLEAR_CONFIRMATION packet
// (no payload)

// Clearing Cause
// - Worker Originated (default) - if clear request came from worker
// - Number Busy - returned by Broker if peer is on a call
// - Not Obtainable - returned by Broker if peer is unknown
// - Out Of Order - returned by Broker for all ZeroMQ failure conditions
// - Remote Procedure Error - returned by Broker because of mistakes from peer worker
// - Local Procedure Error - returned by Broker because of mistakes by this worker
// - Invalid Facility Request - decided by Broker when facilities requests are out of range
// - Access Barred - returned by Broker if peer won't take incoming calls
// - Network Congestion - returned by Broker if there are too many open calls

// In a CLEAR_REQUEST from a worker, the Diagnostic code may be d_unspecified.
// In a CLEAR_REQUEST that descends from a DISCONNECT request, the diagnostic code from the DISCONNECT is used

// Diagnostic
// - Unspecified (default) - only clearing requests from workers can be unspecified
// - (Any format error) - local procdure error
// - Facility Parameter Not Allowed - from Broker with Invalid Facility Request cause
// - Invalid Called DTE Address - from Broker with Local Procedure Error cause
// - Invalid Calling DTE Address - from Broker with Local Procedure Error cause
// - Unknown Number - from Broker with Not Obtainable cause
// - Called DTE out of order - from Broker with Out Of Order cause (this will come from HeartBeating!!!)
// - Call Collision - from Broker with Number Busy cause
