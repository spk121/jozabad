Abbreviations
=============

tput    throughput (bits/sec)
iodir   input/output direction
dx      diagnostic
actn    action
enq     request aka enquiry
ack     positive request acknowledgement or confirmation
nak     negative request acknowledgement

Visual Studio C++ 2012 vs GCC / GNU libc
========================================
Supporting these two cases together is rather difficult.

Herb at the MS team are working hard at implementing the C++1x
standards and standard lib, but, have given up on C11.  Herb says that
if you want C11 on Visual Studio, you should just use the C++ compiler
and then skip the features of C11 that aren't in C++11.

Message formats
===============


X.121 Addresses
---------------

The original X.25 spec used X.121 addresses, which is as good a scheme as any, for now.

According to http://support.microsoft.com/kb/123203

AAA-B-CCCCCCCC-DD

AAA is a 3 digit "country code"
- Digit 1 is 2..7
- Digit 2 to 4 is 0..9

B is a "network id"

CCCCCCCC is the "national number" identifying a worker

DD is a subaddress identifying an instance of a worker.

In practice, this means that the broker will have a code AAA-B and will reject any worker connections with a different AAA-B.

Also, if a worker calls CCCCCCCC and it is busy, there might someday be functionality to forward the call to a specific CCCCCCCC-DD.

For now, the last two DD should never be used.

Stringprep / SASLPrep
--------------------
X.121 is probably too restrictve, even for this kind of nonsense.

Probably should just have a fixed-length UTF-8 string that passes rules like SASLPrep.

GNU libidn could be used for that.

There's also GNU SASL if I want simple authentication / OpenID.