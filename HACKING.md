# HACKING NOTES #

Lessons learned and info to remember.

## Comparison to X.25 ##

### Address Assignment ###

One thing I could never figure out from reading these X.25 docs is how
callers get their X.25 addresses in the first place, and what
protection exists agains spoofing.  The calling and caller addresses
are in every packet, but, it looks like anyone can just create a
packet with a given calling address.

Anyway, I found a file here: http://www.textfiles.com/phreak/x25.txt.
"This report specifies the attachment of an X.25 host to the Defense
Data Network (DDN)."

It says

> DDN addresses are assigned to subscriber DTEs by the
> Administration.     Two  basic  forms  of  address  are  provided:
> physical addresses, which correspond to the node number and DCE
> port number of the node to which the DTE is connected, and
> logical addresses, which are mapped transparently by DCE software
> into a corresponding physical network address.  Each DTE is
> assigned one physical address, and may be assigned one or more
> logical addresses.  All DDN addresses are either twelve or
> fourteen BCD (binary-coded decimal) digits in length.  A calling
> DTE need not determine whether a given address is a physical or
> logical address, in order to establish a call to that address.

So it looks like getting a name was an off-line pre-assigned thing.
And I guess there was no protection against spoofing.

The DDN variant of X.25 address were of the form

ZZZZ F DDDDDDD (SS)

- ZZZZ: always 0000

- F: 0 -> 'physical address', 1 -> 'logical address'

- DDDDDDD: address assigned by the administration

- (SS): optional sub-address used by the DTE for any purpose, carried
  across the network without modification

A DTE could be assigned a single physical address and multiple logical
addresses.

Oooh.  Check this out.  It says

> If a DTE sends a CALL REQUEST packet with no calling address
> field, the local DCE will insert the physical calling DDN Host
> Identifier with no subaddress field.  If a DTE sends a CALL
> REQUEST or CALL ACCEPTED packet with either or both calling or
> called addresses that contain F = zero and DDDDDDD = zero, the
> local DCE will replace the DDN Host Identifier field (DDDDDDD)
> with the physical address of the DTE.

So a client would just send a CALL REQUEST and when it receives its
CALL ACCEPTED, its address would be filled in.

### Call Initialization ###

I found a note here that explains how a call is initialized.
http://textfiles.com/hacking/datapac.inf

> THE CUSTOMER'S TERMINAL (HOST) IS CONNECTED TO A LOCAL MODEM WHICH
> IN TURN, IS CONNECTED TO A SECOND MODEM (REMOTE) IN THE CENTRAL
> OFFICE VIA BY 4 WIRES WHICH IN TURN, IS CONNECTED TO A LINE
> PROCESSING MODULE IN THE DATAPAC NETWORK. THIS CONFIGURATION IS
> CALLED THE DTE/DCE LINK AND CAN BE ASSIGNED SPEEDS OF 1200 BPS
> THROUGH 19200 BPS.

> THIS DTE/DCE LINK IS ASSIGNED A UNIQUE DATAPAC NETWORK ADDRESS (DNA)
> AND OTHER LINK PARA METERS SUCH AS LINE SPEED, MODEM TYPE, FLOW
> CONTROL AND SECURITY BY TELECOM CANADA.

> WHEN THE ELECTRICAL SIGNALS ARE IN THE CORRECT STATE AS SPECIFIED IN
> LEVEL 1, THE DATAPAC LINE PROCESSING MODULE CONTINUOUSLY TRANSMITS A
> CCITT COMMAND CALLED SABM (SET ASYNCHRONOUS BALANCED MODE) TO THE
> CUSTOMERS'S TERMINAL (HOST) EVERY THREE SECONDS.  IF THE HOST IS
> READY, IT RESPONDS TO THE SABM WITH A CCITT RESPONSE UA (UNNUMBERED
> ACKNOWLEDGEMENT).  WHEN THIS OCCURS, THE LINK IS INITIALIZED (LEVEL
> 2 READY), THE HOST AND DATAPAC MODULE EXCHANGE RESTARTS OR
> RESTART/RESTART CONFIRMATION COMMANDS.  WHEN THIS OCCURS, THE
> DTE/DCE LINK GENERATES A TRANSITION TO THE NEXT X.25 LEVEL. LEVEL 3.

> THE DTE THEN SIGNALS THE ADDRESS IT WISHES TO COMMUNICATE WITH IN A
> CCITT DEFINED CALL REQUEST FORMAT (8 DIGITS ), 10 DIGITS IF USING
> 9TH AND 10TH DIGIT SUBADDRESSING ON A LOGICAL CHANNEL (LCN)DATAPAC
> THEN ROUTES THE CALL REQUEST TO THE APPROPRIATE DESTINATION
> (NATIONAL OR INTERNATIONAL) AND AWAITS A CCITT DEFINED CALL ACCEPT
> PACKET. IF THIS OCCURS, THE ACCEPT PACKET IS TRANSMITTED BACK TO THE
> ORIGINATING HOST AND BOTH HOSTS MAY NOW EXCHANGE CCITT DEFINED DATA
> PACKETS.  THIS IS CALLED A SWITCHED VIRTUAL CALL (SVC); PERMANENT
> VIRTUAL CALLS (PVC'S) ARE ALSO OFFERED.  AT THE END OF THE SESSION,
> EITHER HOST CAN TERMINATE THE SVC BY TRANSMITTING A CCITT DEFINED
> CLEAR REQUEST PACKET.  UP TO 255 SVC'S MAY BE SUPPORTED
> SIMULTANEOUSLY.

So it looks like I was right to invent a CONNECT message

## Cross platform issues ##

**There will be no #ifdef or `configure`!**

Herb at the MS team are working hard at implementing the C++1x
standards and standard lib, but, have given up on C11.  Herb says that
if you want C11 on Visual Studio, you should just use the C++ compiler
and then skip the features of C11 that aren't in C++11.

The plan is to make code run on GNU `gcc` > 4.8.x and on Microsoft
`cl` > 11.0. With `gcc`, we're compiling as C11.  With `cl`, we're
compiling as C++11.  Also, the intention is that, _for code that I
write_, we strictly use C11.

Required `gcc` compile flags

* `-std=c11`

Required `cl` flags

* `/TP` - compile in C++ mode

### bool ###

Unfortunately, the native `bool` can't be used without ifdeferey.

C11 only defines `bool` if `<stdbool.h>` is included.  The `bool` is a macro that expands to `_Bool`.  `true` and `false` are macros that expand to 1.  When defined `__bool_true_false_are_defined` is a macro equal to 1.

`cl` has `bool` in the default C++ language but doesn't have `<stdbool.h>`.  It is one byte.

From [MSDN](http://msdn.microsoft.com/en-us/library/vstudio/tf4dy80a.aspx)

> In Visual C++4.2, the Standard C++ header files contained a typedef that equated bool with int. In Visual C++ 5.0 and later, bool is implemented as a built-in type with a size of 1 byte. That means that for Visual C++ 4.2, a call of sizeof(bool) yields 4, while in Visual C++ 5.0 and later, the same call yields 1. This can cause memory corruption problems if you have defined structure members of type bool in Visual C++ 4.2 and are mixing object files (OBJ) and/or DLLs built with the 4.2 and 5.0 or later compilers.

> The __BOOL_DEFINED macro can be used to wrap code that is dependent on whether or not bool is supported.

## Abbreviations ##

Any abbreviated word should go in this list.

| Abbreviation | Meaning |
|--------------|----------------------- |
| tput         | throughput (bits/sec)
| iodir        | incoming calls barred, outgoing calls barred, bidirectional calls ok
| dx           | diagnostic
| actn         | action
| enq          | enquiry / request
| ack          | positive request acknowledgement or confirmation
| nak          | negative request acknowledgement

## Address Formats ##

Need to identify each worker with a short text string

### Safe ASCII ###

Keep it simple.

### X.121 Addresses ###

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

### StringPrep / SASLPrep addresses ###

X.121 is probably too restrictve, even for this kind of nonsense.

Probably should just have a fixed-length UTF-8 string that passes rules like SASLPrep.

GNU libidn could be used for that.

There's also GNU SASL if I want simple authentication / OpenID.

## ELF ##

`objdump -h -s <file>` lets me check the contents of .rodata to make sure my constant data is going in the right place

`strip -s <file>` strips all symbol and relocation info

`strip -R .comment <file>` removes the compiler version from the elf
