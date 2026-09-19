#pragma once

#include "BitBuffer.hpp"
#include "QrSegment.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace qrcodegen {

class QrCode final {
public:
    enum class Ecc {
        LOW = 0,
        MEDIUM,
        QUARTILE,
        HIGH,
    };

    static const int MIN_VERSION = 1;
    static const int MAX_VERSION = 40;

    static QrCode encodeText(const char *text, Ecc ecl);
    static QrCode encodeBinary(const std::vector<std::uint8_t> &data, Ecc ecl);
    static QrCode encodeSegments(const std::vector<QrSegment> &segs, Ecc ecl,
        int minVersion = 1, int maxVersion = 40, int mask = -1, bool boostEcl = true);

    int getVersion() const;
    int getSize() const;
    Ecc getErrorCorrectionLevel() const;
    int getMask() const;
    bool getModule(int x, int y) const;

    QrCode(int ver, Ecc ecl, const std::vector<std::uint8_t> &dataCodewords, int msk);

    static const std::int8_t ECC_CODEWORDS_PER_BLOCK[4][41];
    static const std::int8_t NUM_ERROR_CORRECTION_BLOCKS[4][41];

private:
    int version;
    int size;
    Ecc errorCorrectionLevel;
    int mask;
    std::vector<std::vector<bool>> modules;
    std::vector<std::vector<bool>> isFunction;

    void drawFunctionPatterns();
    void drawFormatBits(int msk);
    void drawVersion();
    void drawFinderPattern(int x, int y);
    void drawAlignmentPattern(int x, int y);
    void setFunctionModule(int x, int y, bool isDark);
    bool module(int x, int y, bool isDark) const;

    std::vector<std::uint8_t> addEccAndInterleave(const std::vector<std::uint8_t> &data) const;
    void drawCodewords(const std::vector<std::uint8_t> &data);
    void applyMask(int msk);
    long getPenaltyScore() const;

    static bool getBit(long val, int i);
};

}
