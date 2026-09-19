#pragma once

#include <cstdint>
#include <vector>

namespace qrcodegen {

class BitBuffer final : public std::vector<bool> {
public:
    BitBuffer();
    void appendBits(std::uint32_t val, int len);
};

}
