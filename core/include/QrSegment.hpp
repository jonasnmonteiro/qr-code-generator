#pragma once

#include "BitBuffer.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace qrcodegen {

class QrSegment final {
public:
    class Mode final {
    public:
        static const Mode NUMERIC;
        static const Mode ALPHANUMERIC;
        static const Mode BYTE;
        static const Mode KANJI;
        static const Mode ECI;

        int getModeBits() const;
        int numCharCountBits(int version) const;

    private:
        int modeBits;
        int numBitsCharCount[3];
        Mode(int mode, int cc0, int cc1, int cc2);
    };

    static QrSegment makeBytes(const std::vector<std::uint8_t> &data);
    static QrSegment makeNumeric(const char *digits);
    static QrSegment makeAlphanumeric(const char *text);
    static std::vector<QrSegment> makeSegments(const char *text);
    static QrSegment makeEci(long assignVal);

    static bool isNumeric(const char *text);
    static bool isAlphanumeric(const char *text);

    QrSegment(const Mode &md, int numCh, const std::vector<bool> &dt);
    QrSegment(const Mode &md, int numCh, std::vector<bool> &&dt);

    const Mode &getMode() const;
    int getNumChars() const;
    const std::vector<bool> &getData() const;

    static int getTotalBits(const std::vector<QrSegment> &segs, int version);

private:
    const Mode *mode;
    int numChars;
    std::vector<bool> data;
};

}
