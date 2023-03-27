#include "stream_reassembler.hh"

#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _next_index(0), _buff{}, _eof_index(-1) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    DUMMY_CODE(data, index, eof);
    size_t _index = index;
    string no_overlap_data = data;
    size_t unacceptable_index = _next_index + _capacity - _output.buffer_size();

    if (eof) {
        _eof_index = index + data.size();
    }

    if (_next_index > index) {
        if (index + data.size() >= _next_index) {
            no_overlap_data = no_overlap_data.substr(_next_index - _index, no_overlap_data.size());
            _index = _next_index;
        } else {
            return;
        }
    }

    size_t _buff_index = _index;
    for (auto c : no_overlap_data) {
        if (_buff_index < unacceptable_index) {
            _buff[_buff_index] = c;
            _buff_index++;
        }
    }

    string new_str{};
    while (_buff.count(_next_index)) {
        new_str.push_back(_buff[_next_index]);
        _buff.erase(_next_index);
        _next_index++;
    }

    if (!new_str.empty()) {
        _output.write(new_str);
    }

    if (empty() && _next_index == _eof_index) {
        _output.end_input();
    }
}

size_t StreamReassembler::unassembled_bytes() const { return _buff.size(); }

bool StreamReassembler::empty() const { return _buff.empty(); }
