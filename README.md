# Jozabad

Jozabad is a broker and a client library that implements a custom hub-and-spoke communcation protocol.

The basic goals of the protocol are these:

* A node can connect to or disconnect from the hub at any time.
* A node can request that the hub create a persistent, exclusive, asynchronous, bilateral communcation channel with any other idle node willing to communcate.
* The communication between connected node pairs is stateful (not REST).  It allows for Telnet-like protocols.
* The hub prevents nodes from discovering one-another's IP address and port.
* It has safeguards to allow multiple connections to fairly share a low-speed (384 kbit/sec) DSL link.
** Each node can use a flow control scheme to limit the amount and rate of information that it receives.
** The hub itself can also limit the amount and rate of information passing through it.

The name "Jozabad" refers to a talented archer that fought with the biblical King David.  The image the archer is a metaphor for hub-to-node information transmission.

## References

* Jozabad is inspired by the ITU's X.25 protocol: specifically its message types and state machine.  It is not a true implementation of X.25 over TCP, which is known as XOT.
* It uses ZeroMQ sockets to reduce book-keeping.
