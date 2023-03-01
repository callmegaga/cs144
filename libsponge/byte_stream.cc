#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&.../* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity)
    : _error(false), _end(false), _total_write(0), _total_read(0), _size(capacity) {
    //    DUMMY_CODE(capacity);
}

size_t ByteStream::write(const string &data) {
    //    DUMMY_CODE(data);
    size_t max_write_count = _size - _pipe.size();
    size_t write_count = 0;
    auto it = data.begin();
    while (write_count < max_write_count && it != data.end()) {
        write_count++;
        _total_write++;
        _pipe.push_back(*it);
        it++;
    }
    return write_count;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    //    DUMMY_CODE(len);
    string result{};
    size_t count = len;
    auto it = _pipe.begin();
    while (count && it != _pipe.end()) {
        result.push_back(*it);
        it++;
        count--;
    }
    return result;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    // DUMMY_CODE(len);
    size_t count = len;
    while (count && !_pipe.empty()) {
        _pipe.pop_front();
        _total_read++;
        count--;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    //    DUMMY_CODE(len);
    string result{};
    size_t count = len;
    while (count && !_pipe.empty()) {
        result.push_back(_pipe.front());
        _pipe.pop_front();
        _total_read++;
        count--;
    }
    return result;
}

void ByteStream::end_input() { _end = true; }

bool ByteStream::input_ended() const { return _end; }

size_t ByteStream::buffer_size() const { return _pipe.size(); }

bool ByteStream::buffer_empty() const { return _pipe.empty(); }

bool ByteStream::eof() const { return _end && _pipe.empty(); }

size_t ByteStream::bytes_written() const { return _total_write; }

size_t ByteStream::bytes_read() const { return _total_read; }

size_t ByteStream::remaining_capacity() const { return _size - _pipe.size(); }
