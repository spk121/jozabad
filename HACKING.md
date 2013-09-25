# HACKING NOTES #

Lessons learned and info to remember.

## Logging ##

ERR is for fatals.

WARN is for things that are fatal in debug, and just critical in release.

INFO is for messages that are sent and for how they are dispatched.  These should just before message is dispatched in the broker or just after it is sent by the broker.

NOTE is for every change of internal state.  These should go just after each internal state change.

TRACE is for entering / leaving critical functions.

## Cross platform issues ##

**There will be no #ifdef or `configure`!**

Herb at the MS team are working hard at implementing the C++1x standards and standard lib, but, have given up on C11.  Herb says that if you want C11 on Visual Studio, you should just use the C++ compiler and then skip the features of C11 that aren't in C++11. 

The plan is to make code run on GNU `gcc` > 4.8.x and on Microsoft `cl` > 11.0. With `gcc`, we're compiling as C11.  With `cl`, we're compiling as C++11.  Also, the intention is that, _for code that I write_, we strictly use C11.

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
