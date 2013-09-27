# Jozabad

*Jozabad* is a broker and client library of a custom hub-and-spoke communication protocol.  *Workers* connect to a *broker* and send messages to the *broker*.  The *broker* joins pairs of workers together in *channels* and forwards messages between connected workers.

Jozabad is thus, basically, like a telephone exchange.  It is inspired by the ITU's X.25 protocol.

The basic goals of the protocol are these:


The *broker* ensures that
* messages are valid
* data rates are obeyed

A *worker* can
* connect / disconnect to the *broker* at will
* get a list of other workers from the broker.
* request a *channel* with any other idle worker

The *channel*
* is exclusive -- a worker can have only one channel at a time
* is persistent -- it exists as long as the workers want
* is stateful
* is pseudonymous -- the workers only know one-anothers' pseudonym

The future goals of the protocol are these: 

* The protocol will use future ZeroMQ security protocols to authenticate workers and encrypt traffic.

The name "Jozabad" refers to a talented archer that fought with the biblical King David.  The image the archer is a metaphor for hub-to-node information transmission.

*Jozabad* is the lowest level of a three layer application stack.
* *Jozabad* handles low-level communication.
* *Johanan* is a packet assembler and disassembler, expressing a stream-like interface (fprintf, fgetc) for intra-worker communcation based on *Jozabad* messaging.
* *Jahaziel* is a text protocol with in-band escapes for colors, graphics, and audio, and a rendering engine for that protocol.

## Documentation

The documentation, such as it is
https://github.com/spk121/jozabad/blob/master/doc/jozabad.pdf?raw=true
## References

* Jozabad is inspired by the ITU's X.25 protocol: specifically its message types and state machine.  It is not a true implementation of X.25 over TCP, which is known as XOT.
* It uses ZeroMQ sockets to do all the heavy lifting.
