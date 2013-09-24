# Jozabad

*Jozabad* is a broker and a client library that implements a custom hub-and-spoke communication protocol for Telnet-like or chat-like applications.  A set of one or more *workers* connect to the *broker* and send messages to the *broker*.  The *broker* joins pairs of workers together in *channels*, and forwards message between connected workers.

Jozabad is thus, basically, like a telephone exchange.  It is inspired by the ITU's X.25 protocol.

The basic goals of the protocol are these:

* A *worker* can connect to or disconnect from the *broker* at any time.
* A worker can request a list of connected workers from the broker.
* A worker can request that the broker create a persistent, exclusive, asynchronous, bilateral communication *channel* with any other idle worker willing to communcate.
* The *channel* between connected worker pairs is stateful (not REST).
* The broker prevents workers from discovering one-another's IP address and port.
* The broker ensures that every message a worker receives has valid a valid format, state, and doesn't exceed an agreed-upon data rate.

The future goals of the protocol are these: 

* The protocol will use future ZeroMQ security protocols to authenticate workers and encrypt traffic.

The name "Jozabad" refers to a talented archer that fought with the biblical King David.  The image the archer is a metaphor for hub-to-node information transmission.

*Jozabad* is the lowest level of a three layer application stack.
* *Jozabad* handles low-level communication.
* *Johanan* is a packet assembler and disassembler, automatically sending and receiving data from I/O buffers when required.
* *Jahaziel* is a text protocol with in-band escapes for colors, graphics, and audio.  It also has a sample rendering engine.

## References

* Jozabad is inspired by the ITU's X.25 protocol: specifically its message types and state machine.  It is not a true implementation of X.25 over TCP, which is known as XOT.
* It uses ZeroMQ sockets to reduce book-keeping.
