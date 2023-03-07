#include "wrapping_integers.hh"

#include <math.h>

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    DUMMY_CODE(n, isn);
    uint64_t mod = 1ll << 32;
    ::uint32_t seq_no = (n + static_cast<::uint64_t>(isn.raw_value())) % mod;
    return WrappingInt32{seq_no};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    DUMMY_CODE(n, isn, checkpoint);
    ::uint64_t abs_no = n - isn;
    ::int32_t raw_abs = n - isn;
    ::uint64_t mod = 1ll << 32;

    if (raw_abs < 0) {
        abs_no = raw_abs + mod;
    }

    ::uint32_t times = checkpoint / mod;
    ::uint64_t same_round_abs_no = abs_no + mod * times;
    ::uint64_t bigger_round_abs_no = abs_no + mod * (times + 1);
    ::uint64_t less_round_abs_no = abs_no + (mod * (times - 1));

    ::uint64_t distance_same_round_abs_no = abs(static_cast<long long>(same_round_abs_no - checkpoint));
    ::uint64_t distance_bigger_round_abs_no = abs(static_cast<long long>(bigger_round_abs_no - checkpoint));
    ::uint64_t distance_less_round_abs_no = abs(static_cast<long long>(less_round_abs_no - checkpoint));

    if (times >= 1) {
        if (distance_same_round_abs_no < distance_bigger_round_abs_no) {
            if (distance_same_round_abs_no < distance_less_round_abs_no) {
                return same_round_abs_no;
            } else {
                return less_round_abs_no;
            }
        } else {
            return bigger_round_abs_no;
        }
    } else {
        if (distance_same_round_abs_no < distance_bigger_round_abs_no) {
            return same_round_abs_no;
        } else {
            return bigger_round_abs_no;
        }
    }
}