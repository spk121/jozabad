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
    
    // The CALL_REQUEST message is a special case because it requires
    // both PeerStore and ChannelStore.
    if (msg.is_a(JOZA_MSG_CALL_REQUEST)) {
        pair<int,string> out = peers.do_call_request_step_1(msg);
        if (out.first == 0)
            ret = 0;
        else if (out.first == 1)
            ret = channels.do_call_request_step_2(msg, out.second);
        else if (out.first == 2)
            ret = 2;
        else 
            abort();
            
    }

    // Else, check to see if this message modifies the state of the
    // channel store.  Most messages do this.
    if (ret == 0)
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
