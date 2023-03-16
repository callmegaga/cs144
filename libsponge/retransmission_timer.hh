//
// Created by wy on 2023/3/10.
//

#ifndef SPONGE_RETRANSMISSION_TIMER_H
#define SPONGE_RETRANSMISSION_TIMER_H
#include <cstdint>
#include <stddef.h>

class ReTransmissionTimer {
    unsigned int _time_out;
    uint64_t _segment_no;
    bool _is_zero_window;

  public:
    void tick(const size_t ms_since_last_tick);

    void reset(const size_t time_out);

    bool isReSend() const;

    uint64_t segment_no(void) const;

    bool isZeroWin() { return _is_zero_window; }

    ReTransmissionTimer(unsigned int time_out, uint64_t segment_no, bool is_zero_window)
        : _time_out(time_out), _segment_no(segment_no), _is_zero_window(is_zero_window) {}
};
#endif  // SPONGE_RETRANSMISSION_TIMER_H
