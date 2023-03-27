#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    DUMMY_CODE(seg);
    _time_since_last_segment_received = 0;

    if (seg.header().rst) {
        _active = false;
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        return;
    }

    _receiver.segment_received(seg);
    if (_sender.next_seqno_absolute() == 0 && _receiver.ackno().has_value() && !_receiver.stream_out().input_ended()) {
        connect();
        return;
    }

    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
        if (_receiver.ackno().has_value() && !_receiver.stream_out().input_ended() && !_sender.stream_in().eof() &&
            _sender.next_seqno_absolute() > _sender.bytes_in_flight()) {
            _sender.fill_window();
            SendAllSegments();
        }
    }

    if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }

    if (!_linger_after_streams_finish && _receiver.stream_out().input_ended() && _sender.stream_in().eof() &&
        _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 && _sender.bytes_in_flight() == 0) {
        _active = false;
    }
    if (!(seg.header().ack && seg.length_in_sequence_space() == 0)) {
        _sender.send_empty_segment();
        SendAllSegments();
    }
}

bool TCPConnection::active() const { return _active; }

size_t TCPConnection::write(const string &data) {
    DUMMY_CODE(data);
    size_t write_size = _sender.stream_in().write(data);
    _sender.fill_window();
    SendAllSegments();
    return write_size;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick);
    _time_since_last_segment_received += ms_since_last_tick;

    _sender.tick(ms_since_last_tick);

    if (_sender.consecutive_retransmissions() > _cfg.MAX_RETX_ATTEMPTS) {
        _receiver.stream_out().set_error();
        _sender.stream_in().set_error();
        _active = false;
        _linger_after_streams_finish = false;

        TCPSegment segment{};
        segment.header().rst = true;
        segment.header().ackno = _receiver.ackno().value();
        segment.header().win = _receiver.window_size();
        _segments_out.push(segment);
    } else {
        SendAllSegments();
    }

    if (_time_since_last_segment_received >= _cfg.rt_timeout * 10) {
        if (_sender.stream_in().eof() && _sender.next_seqno_absolute() == _sender.stream_in().bytes_written() + 2 &&
            _sender.bytes_in_flight() == 0 && _receiver.stream_out().input_ended() && _linger_after_streams_finish) {
            _active = false;
            _linger_after_streams_finish = false;
        }
    }
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window();
    SendAllSegments();
}

void TCPConnection::connect() {
    _sender.fill_window();
    SendAllSegments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";

            // Your code here: need to send a RST segment to the peer
            TCPSegment segment{};
            segment.header().rst = true;
            segment.header().ackno = _receiver.ackno().value();
            segment.header().win = _receiver.window_size();
            _segments_out.push(segment);
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

void TCPConnection::SendAllSegments() {
    while (!_sender.segments_out().empty()) {
        TCPSegment segment = _sender.segments_out().front();
        _sender.segments_out().pop();
        if (_receiver.ackno().has_value()) {
            segment.header().ack = true;
            segment.header().ackno = _receiver.ackno().value();
            segment.header().win = _receiver.window_size();
        }
        _segments_out.push(segment);
    }
}