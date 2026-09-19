#include "QrSegment.hpp"
#include <climits>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace qrcodegen {

const QrSegment::Mode QrSegment::Mode::NUMERIC     (0x1, 10, 12, 14);
const QrSegment::Mode QrSegment::Mode::ALPHANUMERIC(0x2,  9, 11, 13);
const QrSegment::Mode QrSegment::Mode::BYTE        (0x4,  8, 16, 16);
const QrSegment::Mode QrSegment::Mode::KANJI       (0x8,  8, 10, 12);
const QrSegment::Mode QrSegment::Mode::ECI         (0x7,  0,  0,  0);

QrSegment::Mode::Mode(int mode, int cc0, int cc1, int cc2) :
    modeBits(mode),
    numBitsCharCount{cc0, cc1, cc2} {}

int QrSegment::Mode::getModeBits() const {
    return modeBits;
}

int QrSegment::Mode::numCharCountBits(int version) const {
    return numBitsCharCount[(version + 7) / 17];
}

static const char *ALPHANUMERIC_CHARSET = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:";

QrSegment QrSegment::makeBytes(const std::vector<std::uint8_t> &data) {
    if (data.size() > static_cast<unsigned int>(INT_MAX)) {
        throw std::length_error("Data too long");
    }
    BitBuffer bb;
    for (std::uint8_t b : data) {
        bb.appendBits(b, 8);
    }
    return QrSegment(Mode::BYTE, static_cast<int>(data.size()), std::move(bb));
}

QrSegment QrSegment::makeNumeric(const char *digits) {
    std::size_t len = std::strlen(digits);
    if (len > static_cast<unsigned int>(INT_MAX)) {
        throw std::length_error("Data too long");
    }
    BitBuffer bb;
    for (std::size_t i = 0; i < len; ) {
        int n = static_cast<int>(std::min(static_cast<std::size_t>(3), len - i));
        std::uint32_t val = 0;
        for (int j = 0; j < n; j++, i++) {
            char c = digits[i];
            if (c < '0' || c > '9') {
                throw std::invalid_argument("String contains non-numeric characters");
            }
            val = val * 10 + static_cast<std::uint32_t>(c - '0');
        }
        bb.appendBits(val, n * 3 + 1);
    }
    return QrSegment(Mode::NUMERIC, static_cast<int>(len), std::move(bb));
}

QrSegment QrSegment::makeAlphanumeric(const char *text) {
    std::size_t len = std::strlen(text);
    if (len > static_cast<unsigned int>(INT_MAX)) {
        throw std::length_error("Data too long");
    }
    BitBuffer bb;
    for (std::size_t i = 0; i < len; ) {
        std::uint32_t val = 0;
        int n = static_cast<int>(std::min(static_cast<std::size_t>(2), len - i));
        for (int j = 0; j < n; j++, i++) {
            const char *match = std::strchr(ALPHANUMERIC_CHARSET, text[i]);
            if (match == nullptr) {
                throw std::invalid_argument("String contains unencodable characters in alphanumeric mode");
            }
            val = val * 45 + static_cast<std::uint32_t>(match - ALPHANUMERIC_CHARSET);
        }
        bb.appendBits(val, n * 5 + 1);
    }
    return QrSegment(Mode::ALPHANUMERIC, static_cast<int>(len), std::move(bb));
}

std::vector<QrSegment> QrSegment::makeSegments(const char *text) {
    std::vector<QrSegment> result;
    if (std::strlen(text) == 0) {
        return result;
    }
    if (isNumeric(text)) {
        result.push_back(makeNumeric(text));
    } else if (isAlphanumeric(text)) {
        result.push_back(makeAlphanumeric(text));
    } else {
        std::vector<std::uint8_t> bytes;
        for (const char *p = text; *p != '\0'; p++) {
            bytes.push_back(static_cast<std::uint8_t>(*p));
        }
        result.push_back(makeBytes(bytes));
    }
    return result;
}

QrSegment QrSegment::makeEci(long assignVal) {
    BitBuffer bb;
    if (assignVal < 0) {
        throw std::invalid_argument("ECI assignment value out of range");
    } else if (assignVal < (1 << 7)) {
        bb.appendBits(static_cast<std::uint32_t>(assignVal), 8);
    } else if (assignVal < (1 << 14)) {
        bb.appendBits(2, 2);
        bb.appendBits(static_cast<std::uint32_t>(assignVal), 14);
    } else if (assignVal < 1000000L) {
        bb.appendBits(6, 3);
        bb.appendBits(static_cast<std::uint32_t>(assignVal), 21);
    } else {
        throw std::invalid_argument("ECI assignment value out of range");
    }
    return QrSegment(Mode::ECI, 0, std::move(bb));
}

bool QrSegment::isNumeric(const char *text) {
    for (; *text != '\0'; text++) {
        char c = *text;
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

bool QrSegment::isAlphanumeric(const char *text) {
    for (; *text != '\0'; text++) {
        if (std::strchr(ALPHANUMERIC_CHARSET, *text) == nullptr) {
            return false;
        }
    }
    return true;
}

QrSegment::QrSegment(const Mode &md, int numCh, const std::vector<bool> &dt) :
    mode(&md),
    numChars(numCh),
    data(dt) {
    if (numCh < 0) {
        throw std::invalid_argument("Invalid value");
    }
}

QrSegment::QrSegment(const Mode &md, int numCh, std::vector<bool> &&dt) :
    mode(&md),
    numChars(numCh),
    data(std::move(dt)) {
    if (numCh < 0) {
        throw std::invalid_argument("Invalid value");
    }
}

const QrSegment::Mode &QrSegment::getMode() const {
    return *mode;
}

int QrSegment::getNumChars() const {
    return numChars;
}

const std::vector<bool> &QrSegment::getData() const {
    return data;
}

int QrSegment::getTotalBits(const std::vector<QrSegment> &segs, int version) {
    int result = 0;
    for (const QrSegment &seg : segs) {
        int ccbits = seg.mode->numCharCountBits(version);
        if (seg.numChars >= (1L << ccbits)) {
            return -1;
        }
        if (INT_MAX - result < 4 + ccbits) {
            return -1;
        }
        result += 4 + ccbits;
        if (INT_MAX - result < static_cast<int>(seg.data.size())) {
            return -1;
        }
        result += static_cast<int>(seg.data.size());
    }
    return result;
}

}
