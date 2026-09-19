#include "BitBuffer.hpp"
#include <stdexcept>

namespace qrcodegen {

BitBuffer::BitBuffer() {}

void BitBuffer::appendBits(std::uint32_t val, int len) {
    if (len < 0 || len > 31 || val >> len != 0) {
        throw std::invalid_argument("Value out of range");
    }
    for (int i = len - 1; i >= 0; i--) {
        this->push_back(((val >> i) & 1) != 0);
    }
}

}
