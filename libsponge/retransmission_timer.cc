//
// Created by wy on 2023/3/10.
//

#include "retransmission_timer.hh"

void ReTransmissionTimer::tick(const size_t ms_since_last_tick) {
    if (_time_out <= ms_since_last_tick) {
        _time_out = 0;
    } else {
        _time_out = (_time_out - ms_since_last_tick);
    }
}

inline bool ReTransmissionTimer::isReSend() const { return _time_out == 0; }

inline void ReTransmissionTimer::reset(const size_t time_out) { _time_out = time_out; }

inline uint64_t ReTransmissionTimer::segment_no() const { return _segment_no; }