#include "tcp_sender.hh"

#include "retransmission_timer.cc"
#include "tcp_config.hh"

#include <iostream>
#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity)
    , _current_retransmission_timeout{retx_timeout} {}

uint64_t TCPSender::bytes_in_flight() const { return _inflight_count; }

void TCPSender::fill_window() {
    if (_fin_send) {
        return;
    }

    if (!_syn_send) {
        TCPSegment segment{};
        segment.header().syn = true;
        segment.header().seqno = _isn;
        _segments_out.push(segment);
        _next_seqno += segment.length_in_sequence_space();
        _inflight_count += segment.length_in_sequence_space();
        _syn_send = true;
        _out_segments.insert({_next_seqno, segment});
        ReTransmissionTimer a{_current_retransmission_timeout, _next_seqno, _is_zero_window};
        _timers.push_back(a);
    } else {
        if (!stream_in().buffer_empty() && _window_size) {
            while (!stream_in().buffer_empty() && _window_size) {
                TCPSegment segment{};
                size_t payload_length =
                    _window_size > TCPConfig::MAX_PAYLOAD_SIZE ? TCPConfig::MAX_PAYLOAD_SIZE : _window_size;
                payload_length =
                    stream_in().buffer_size() > payload_length ? payload_length : stream_in().buffer_size();
                if (_window_size != 0)
                    _window_size -= payload_length;
                Buffer payload = Buffer{stream_in().read(payload_length)};
                segment.header().seqno = wrap(_next_seqno, _isn);
                segment.payload() = payload;
                if (stream_in().eof() && _window_size) {
                    segment.header().fin = true;
                    _fin_send = true;
                }
                _segments_out.push(segment);
                _next_seqno += segment.length_in_sequence_space();
                _inflight_count += segment.length_in_sequence_space();
                _out_segments.insert({_next_seqno, segment});
                ReTransmissionTimer a{_current_retransmission_timeout, _next_seqno, _is_zero_window};
                _timers.push_back(a);
            }
        } else {
            if (stream_in().eof() && !_fin_send && _window_size) {
                TCPSegment segment{};
                segment.header().fin = true;
                segment.header().seqno = wrap(_next_seqno, _isn);
                _segments_out.push(segment);
                _next_seqno += segment.length_in_sequence_space();
                _inflight_count += segment.length_in_sequence_space();

                _out_segments.insert({_next_seqno, segment});
                ReTransmissionTimer a{_current_retransmission_timeout, _next_seqno, _is_zero_window};
                _timers.push_back(a);
                _fin_send = true;
            }
        }
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    DUMMY_CODE(ackno, window_size);
    ::uint64_t ack_no_absolute = unwrap(ackno, _isn, _next_seqno);

    if (ack_no_absolute > _next_seqno) {
        return;
    }

    _window_size = window_size;

    if (window_size == 0) {
        _window_size = 1;
        _is_zero_window = true;
    } else {
        _is_zero_window = false;
    }

    for (auto i = _out_segments.begin(); i != _out_segments.end() && i->first <= ack_no_absolute;) {
        if (i->second.header().syn || i->second.header().fin) {
            _inflight_count -= 1;
        }
        _inflight_count -= i->second.payload().size();
        _out_segments.erase(i++);
    }

    while (!_timers.empty() && _timers.front().segment_no() <= ack_no_absolute) {
        _timers.pop_front();
    }

    _consecutive_retransmission_count = 0;
    _current_retransmission_timeout = _initial_retransmission_timeout;

    if (ack_no_absolute != _last_ack_no) {
        for (auto &time : _timers) {
            time.reset(_current_retransmission_timeout);
        }
    }

    if (_inflight_count > _window_size) {
        _window_size = 0;
    }

    _last_ack_no = ack_no_absolute;

    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    DUMMY_CODE(ms_since_last_tick);
    bool have_retransmission = false;
    for (auto &time : _timers) {
        time.tick(ms_since_last_tick);
        if (time.isReSend() && !have_retransmission) {
            have_retransmission = true;
            _segments_out.push(_out_segments[time.segment_no()]);

            if (!time.isZeroWin()) {
                _consecutive_retransmission_count++;
                _current_retransmission_timeout *= 2;
            }
            time.reset(_current_retransmission_timeout);
        }

        if (have_retransmission) {
            time.reset(_current_retransmission_timeout);
        }
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _consecutive_retransmission_count; }

void TCPSender::send_empty_segment() {
    TCPSegment segment{};
    segment.header().seqno = wrap(_next_seqno, _isn);
    _segments_out.push(segment);
}
