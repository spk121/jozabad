#include "process.hxx"
#include "Peer.hxx"
#include "Poll.hxx"
#include "Channel.hxx"
// This is the heart of it all.
// All static variables are to be housed here


PeerStore peers;
ChannelStore channels;
Poll poll;

int dispatch(Msg& msg)
{
    int ret;

 do_more:
    ret = 0;

    // First, check to see if this message modifies the state of the
    // channel store.  Most messages do this.
    ret = channels.dispatch(msg);

    // Else, check to see if this message modifies the state of the
    // peer store.  Only connection and disconnection messages do
    // this.
    if (ret == 0)
        ret = peers.dispatch(msg);

    // Else, check to see if this is a message that doesn't modify
    // either the channel store or the peer store.  Some broker
    // status messages, which are TBD, may do this.
    if (ret == 0)
        ret = poll.dispatch(msg);

    if (ret == 0) {
        // unexpected message from unknown peer
    }
    if (ret == 1)
        goto do_more;
    return 0;
}

int main()
{
    poll.initialize(true, "tcp://*:5555");
    poll.start();
    return 0;
}
