#include "tcp_receiver.hh"
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);
    TCPHeader header = seg.header();
    auto payload = seg.payload().copy();
    WrappingInt32 payload_first_index = header.seqno;

    if (!_syn_received && !header.syn) {
        return;
    }

    if (!_syn_received && header.syn) {
        _isn = WrappingInt32(header.seqno);
        _syn_received = true;
        payload_first_index = payload_first_index + 1;
    }

    if (header.fin) {
        _fin_received = true;
    }

    ::uint64_t abs_no = unwrap(payload_first_index, _isn, stream_out().bytes_written());

    _reassembler.push_substring(payload, abs_no - 1, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_received) {
        return nullopt;
    }

    if (_fin_received && stream_out().input_ended()) {
        return wrap(stream_out().bytes_written() + 1 + 1, _isn);
    } else {
        return wrap(stream_out().bytes_written() + 1, _isn);
    }
}

size_t TCPReceiver::window_size() const { return stream_out().remaining_capacity(); }
