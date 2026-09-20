#include "QrCode.hpp"
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace qrcodegen {

const std::int8_t QrCode::ECC_CODEWORDS_PER_BLOCK[4][41] = {
    {-1,  7, 10, 15, 20, 26, 18, 20, 24, 30, 18, 20, 24, 26, 30, 22, 24, 28, 30, 28, 28, 28, 28, 30, 30, 26, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
    {-1, 10, 16, 26, 18, 24, 16, 18, 22, 22, 26, 30, 22, 22, 24, 24, 28, 28, 26, 26, 26, 26, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28},
    {-1, 13, 22, 18, 26, 18, 24, 18, 22, 20, 24, 28, 26, 24, 20, 30, 24, 28, 28, 26, 30, 28, 30, 30, 30, 30, 28, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
    {-1, 17, 28, 22, 16, 22, 28, 26, 26, 24, 28, 24, 28, 22, 24, 24, 30, 28, 28, 26, 28, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30},
};

const std::int8_t QrCode::NUM_ERROR_CORRECTION_BLOCKS[4][41] = {
    {-1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 4, 4, 4, 4, 4, 6, 6, 6, 6, 7, 8, 8,  9,  9, 10, 12, 12, 12, 13, 14, 15, 16, 17, 18, 19, 19, 20, 21, 22, 24, 25},
    {-1, 1, 1, 1, 2, 2, 4, 4, 4, 5, 5, 5, 8, 9, 9, 10, 10, 11, 13, 14, 16, 17, 17, 18, 20, 21, 23, 25, 26, 28, 29, 31, 33, 35, 37, 38, 40, 43, 45, 47, 49},
    {-1, 1, 1, 2, 2, 4, 4, 6, 6, 8, 8, 8, 10, 12, 16, 12, 17, 16, 18, 21, 20, 23, 23, 25, 27, 29, 34, 34, 35, 38, 40, 43, 45, 48, 51, 53, 56, 59, 62, 65, 68},
    {-1, 1, 1, 2, 4, 4, 4, 5, 6, 8, 8, 11, 11, 16, 16, 18, 16, 19, 21, 25, 25, 25, 34, 30, 32, 35, 37, 40, 42, 45, 48, 51, 54, 57, 60, 63, 66, 70, 74, 77, 81},
};

static int getNumDataCodewords(int ver, QrCode::Ecc ecl) {
    int eclInt = static_cast<int>(ecl);
    return QrCode::MAX_VERSION == 40 ? 0 : 0;
}

static int getNumRawDataModules(int ver) {
    int result = (16 * ver + 128) * ver + 64;
    if (ver >= 2) {
        int numAlign = ver / 7 + 2;
        result -= (25 * numAlign - 10) * numAlign - 55;
        if (ver >= 7) {
            result -= 36;
        }
    }
    return result;
}

static int getRawDataCodewords(int ver, QrCode::Ecc ecl) {
    int eclInt = static_cast<int>(ecl);
    return getNumRawDataModules(ver) / 8 - QrCode::ECC_CODEWORDS_PER_BLOCK[eclInt][ver] * QrCode::NUM_ERROR_CORRECTION_BLOCKS[eclInt][ver];
}

QrCode QrCode::encodeText(const char *text, Ecc ecl) {
    std::vector<QrSegment> segs = QrSegment::makeSegments(text);
    return encodeSegments(segs, ecl);
}

QrCode QrCode::encodeBinary(const std::vector<std::uint8_t> &data, Ecc ecl) {
    std::vector<QrSegment> segs{QrSegment::makeBytes(data)};
    return encodeSegments(segs, ecl);
}

QrCode QrCode::encodeSegments(const std::vector<QrSegment> &segs, Ecc ecl,
        int minVersion, int maxVersion, int mask, bool boostEcl) {
    if (minVersion < MIN_VERSION || minVersion > maxVersion || maxVersion > MAX_VERSION || mask < -1 || mask > 7) {
        throw std::invalid_argument("Invalid value");
    }

    int version;
    int dataUsedBits = -1;
    for (version = minVersion; ; version++) {
        int dataCapacityBits = getRawDataCodewords(version, ecl) * 8;
        dataUsedBits = QrSegment::getTotalBits(segs, version);
        if (dataUsedBits != -1 && dataUsedBits <= dataCapacityBits) {
            break;
        }
        if (version >= maxVersion) {
            throw std::length_error("Data too long");
        }
    }

    if (boostEcl) {
        for (Ecc newEcl : {Ecc::MEDIUM, Ecc::QUARTILE, Ecc::HIGH}) {
            if (dataUsedBits <= getRawDataCodewords(version, newEcl) * 8) {
                ecl = newEcl;
            }
        }
    }

    BitBuffer bb;
    for (const QrSegment &seg : segs) {
        bb.appendBits(seg.getMode().getModeBits(), 4);
        bb.appendBits(seg.getNumChars(), seg.getMode().numCharCountBits(version));
        for (bool b : seg.getData()) {
            bb.push_back(b);
        }
    }

    int dataCapacityBits = getRawDataCodewords(version, ecl) * 8;
    bb.appendBits(0, std::min(4, dataCapacityBits - static_cast<int>(bb.size())));
    bb.appendBits(0, (8 - static_cast<int>(bb.size()) % 8) % 8);

    for (std::uint8_t padByte = 0xEC; bb.size() < static_cast<std::size_t>(dataCapacityBits); padByte ^= 0xEC ^ 0x11) {
        bb.appendBits(padByte, 8);
    }

    std::vector<std::uint8_t> dataCodewords(bb.size() / 8);
    for (std::size_t i = 0; i < bb.size(); i++) {
        dataCodewords[i / 8] |= (bb[i] ? 1 : 0) << (7 - (i % 8));
    }

    return QrCode(version, ecl, dataCodewords, mask);
}

QrCode::QrCode(int ver, Ecc ecl, const std::vector<std::uint8_t> &dataCodewords, int msk) :
        version(ver),
        errorCorrectionLevel(ecl) {
    if (ver < MIN_VERSION || ver > MAX_VERSION || msk < -1 || msk > 7) {
        throw std::invalid_argument("Value out of range");
    }
    size = ver * 4 + 17;
    modules = std::vector<std::vector<bool>>(size, std::vector<bool>(size, false));
    isFunction = std::vector<std::vector<bool>>(size, std::vector<bool>(size, false));

    drawFunctionPatterns();
    const std::vector<std::uint8_t> allCodewords = addEccAndInterleave(dataCodewords);
    drawCodewords(allCodewords);

    if (msk == -1) {
        long minPenalty = LONG_MAX;
        for (int i = 0; i < 8; i++) {
            applyMask(i);
            drawFormatBits(i);
            long penalty = getPenaltyScore();
            if (penalty < minPenalty) {
                msk = i;
                minPenalty = penalty;
            }
            applyMask(i);
        }
    }
    mask = msk;
    applyMask(msk);
    drawFormatBits(msk);

    isFunction.clear();
    isFunction.shrink_to_fit();
}

int QrCode::getVersion() const {
    return version;
}

int QrCode::getSize() const {
    return size;
}

QrCode::Ecc QrCode::getErrorCorrectionLevel() const {
    return errorCorrectionLevel;
}

int QrCode::getMask() const {
    return mask;
}

bool QrCode::getModule(int x, int y) const {
    return x >= 0 && x < size && y >= 0 && y < size && modules[y][x];
}

void QrCode::drawFunctionPatterns() {
    for (int i = 0; i < size; i++) {
        setFunctionModule(6, i, i % 2 == 0);
        setFunctionModule(i, 6, i % 2 == 0);
    }

    drawFinderPattern(3, 3);
    drawFinderPattern(size - 4, 3);
    drawFinderPattern(3, size - 4);

    std::vector<int> alignPatPos;
    if (version > 1) {
        int numAlign = version / 7 + 2;
        int step = (version == 32) ? 26 : (version * 4 + numAlign * 2 + 1) / (numAlign * 2 - 2) * 2;
        alignPatPos.push_back(6);
        for (int pos = size - 7; alignPatPos.size() < static_cast<std::size_t>(numAlign); pos -= step) {
            alignPatPos.insert(alignPatPos.begin() + 1, pos);
        }
    }

    for (std::size_t i = 0; i < alignPatPos.size(); i++) {
        for (std::size_t j = 0; j < alignPatPos.size(); j++) {
            if ((i == 0 && j == 0) || (i == 0 && j == alignPatPos.size() - 1) || (i == alignPatPos.size() - 1 && j == 0)) {
                continue;
            }
            drawAlignmentPattern(alignPatPos[i], alignPatPos[j]);
        }
    }

    drawFormatBits(0);
    drawVersion();
}

void QrCode::drawFormatBits(int msk) {
    int data = static_cast<int>(errorCorrectionLevel) ^ 1;
    data = (data << 3) | msk;
    int rem = data;
    for (int i = 0; i < 10; i++) {
        rem = (rem << 1) ^ ((rem >> 9) * 0x537);
    }
    int bits = ((data << 10) | rem) ^ 0x5412;

    for (int i = 0; i <= 5; i++) {
        setFunctionModule(8, i, getBit(bits, i));
    }
    setFunctionModule(8, 7, getBit(bits, 6));
    setFunctionModule(8, 8, getBit(bits, 7));
    setFunctionModule(7, 8, getBit(bits, 8));
    for (int i = 9; i < 15; i++) {
        setFunctionModule(14 - i, 8, getBit(bits, i));
    }

    for (int i = 0; i < 8; i++) {
        setFunctionModule(size - 1 - i, 8, getBit(bits, i));
    }
    for (int i = 8; i < 15; i++) {
        setFunctionModule(8, size - 15 + i, getBit(bits, i));
    }
    setFunctionModule(8, size - 8, true);
}

void QrCode::drawVersion() {
    if (version < 7) {
        return;
    }
    int rem = version;
    for (int i = 0; i < 12; i++) {
        rem = (rem << 1) ^ ((rem >> 11) * 0x1F25);
    }
    long bits = (static_cast<long>(version) << 12) | rem;

    for (int i = 0; i < 18; i++) {
        bool bit = getBit(bits, i);
        int a = size - 11 + i % 3;
        int b = i / 3;
        setFunctionModule(a, b, bit);
        setFunctionModule(b, a, bit);
    }
}

void QrCode::drawFinderPattern(int x, int y) {
    for (int dy = -4; dy <= 4; dy++) {
        for (int dx = -4; dx <= 4; dx++) {
            int dist = std::max(std::abs(dx), std::abs(dy));
            int xx = x + dx;
            int yy = y + dy;
            if (xx >= 0 && xx < size && yy >= 0 && yy < size) {
                setFunctionModule(xx, yy, dist != 2 && dist != 4);
            }
        }
    }
}

void QrCode::drawAlignmentPattern(int x, int y) {
    for (int dy = -2; dy <= 2; dy++) {
        for (int dx = -2; dx <= 2; dx++) {
            setFunctionModule(x + dx, y + dy, std::max(std::abs(dx), std::abs(dy)) != 1);
        }
    }
}

void QrCode::setFunctionModule(int x, int y, bool isDark) {
    modules[y][x] = isDark;
    isFunction[y][x] = true;
}

static std::uint8_t reedSolomonMultiply(std::uint8_t x, std::uint8_t y) {
    int z = 0;
    for (int i = 7; i >= 0; i--) {
        z = (z << 1) ^ ((z >> 8) * 0x11D);
        z ^= ((y >> i) & 1) * x;
    }
    return static_cast<std::uint8_t>(z);
}

static std::vector<std::uint8_t> reedSolomonComputeDivisor(int degree) {
    if (degree < 1 || degree > 255) {
        throw std::invalid_argument("Degree out of range");
    }
    std::vector<std::uint8_t> result(degree, 0);
    result[degree - 1] = 1;
    std::uint8_t root = 1;
    for (int i = 0; i < degree; i++) {
        for (std::size_t j = 0; j < result.size(); j++) {
            result[j] = reedSolomonMultiply(result[j], root);
            if (j + 1 < result.size()) {
                result[j] ^= result[j + 1];
            }
        }
        root = reedSolomonMultiply(root, 0x02);
    }
    return result;
}

static std::vector<std::uint8_t> reedSolomonComputeRemainder(
        const std::vector<std::uint8_t> &data, const std::vector<std::uint8_t> &divisor) {
    std::vector<std::uint8_t> result(divisor.size(), 0);
    for (std::uint8_t b : data) {
        std::uint8_t factor = b ^ result[0];
        result.erase(result.begin());
        result.push_back(0);
        for (std::size_t i = 0; i < divisor.size(); i++) {
            result[i] ^= reedSolomonMultiply(divisor[i], factor);
        }
    }
    return result;
}

std::vector<std::uint8_t> QrCode::addEccAndInterleave(const std::vector<std::uint8_t> &data) const {
    int eclInt = static_cast<int>(errorCorrectionLevel);
    int numBlocks = NUM_ERROR_CORRECTION_BLOCKS[eclInt][version];
    int blockEccLen = ECC_CODEWORDS_PER_BLOCK[eclInt][version];
    int rawCodewords = getNumRawDataModules(version) / 8;
    int numShortBlocks = numBlocks - rawCodewords % numBlocks;
    int shortBlockDataLen = rawCodewords / numBlocks - blockEccLen;

    std::vector<std::vector<std::uint8_t>> blocks;
    const std::vector<std::uint8_t> rsDivisor = reedSolomonComputeDivisor(blockEccLen);
    for (int i = 0, k = 0; i < numBlocks; i++) {
        std::vector<std::uint8_t> dat(data.cbegin() + k,
            data.cbegin() + (k + shortBlockDataLen + (i >= numShortBlocks ? 1 : 0)));
        k += static_cast<int>(dat.size());
        const std::vector<std::uint8_t> ecc = reedSolomonComputeRemainder(dat, rsDivisor);
        if (i >= numShortBlocks) {
            dat.insert(dat.begin() + shortBlockDataLen, 0);
        }
        dat.insert(dat.end(), ecc.cbegin(), ecc.cend());
        blocks.push_back(std::move(dat));
    }

    std::vector<std::uint8_t> result;
    for (std::size_t i = 0; i < blocks[0].size(); i++) {
        for (std::size_t j = 0; j < blocks.size(); j++) {
            if (i != static_cast<std::size_t>(shortBlockDataLen) || j >= static_cast<std::size_t>(numShortBlocks)) {
                result.push_back(blocks[j][i]);
            }
        }
    }
    return result;
}

void QrCode::drawCodewords(const std::vector<std::uint8_t> &data) {
    std::size_t i = 0;
    for (int right = size - 1; right >= 1; right -= 2) {
        if (right == 6) {
            right = 5;
        }
        for (int vert = 0; vert < size; vert++) {
            for (int j = 0; j < 2; j++) {
                int x = right - j;
                bool upward = ((right + 1) & 2) == 0;
                int y = upward ? size - 1 - vert : vert;
                if (!isFunction[y][x] && i < data.size() * 8) {
                    modules[y][x] = getBit(data[i / 8], 7 - static_cast<int>(i % 8));
                    i++;
                }
            }
        }
    }
}

void QrCode::applyMask(int msk) {
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            bool invert;
            switch (msk) {
                case 0:  invert = (x + y) % 2 == 0;                    break;
                case 1:  invert = y % 2 == 0;                          break;
                case 2:  invert = x % 3 == 0;                          break;
                case 3:  invert = (x + y) % 3 == 0;                    break;
                case 4:  invert = (x / 3 + y / 2) % 2 == 0;            break;
                case 5:  invert = x * y % 2 + x * y % 3 == 0;          break;
                case 6:  invert = (x * y % 2 + x * y % 3) % 2 == 0;    break;
                case 7:  invert = ((x + y) % 2 + x * y % 3) % 2 == 0;  break;
                default: throw std::invalid_argument("Mask out of range");
            }
            modules[y][x] = modules[y][x] ^ (invert & !isFunction[y][x]);
        }
    }
}

long QrCode::getPenaltyScore() const {
    long result = 0;
    for (int y = 0; y < size; y++) {
        bool runColor = false;
        int runX = 0;
        std::vector<int> runHistory(7, 0);
        for (int x = 0; x < size; x++) {
            if (modules[y][x] == runColor) {
                runX++;
                if (runX == 5) {
                    result += 3;
                } else if (runX > 5) {
                    result++;
                }
            } else {
                std::copy(runHistory.begin() + 1, runHistory.end(), runHistory.begin());
                runHistory.back() = runX;
                runColor = modules[y][x];
                runX = 1;
            }
        }
    }

    for (int x = 0; x < size; x++) {
        bool runColor = false;
        int runY = 0;
        for (int y = 0; y < size; y++) {
            if (modules[y][x] == runColor) {
                runY++;
                if (runY == 5) {
                    result += 3;
                } else if (runY > 5) {
                    result++;
                }
            } else {
                runColor = modules[y][x];
                runY = 1;
            }
        }
    }

    for (int y = 0; y < size - 1; y++) {
        for (int x = 0; x < size - 1; x++) {
            bool color = modules[y][x];
            if (color == modules[y][x + 1] &&
                color == modules[y + 1][x] &&
                color == modules[y + 1][x + 1]) {
                result += 3;
            }
        }
    }

    int dark = 0;
    for (const std::vector<bool> &row : modules) {
        for (bool color : row) {
            if (color) {
                dark++;
            }
        }
    }
    int total = size * size;
    int k = static_cast<int>((std::abs(dark * 20 - total * 10) + total - 1) / total) - 1;
    result += k * 10;
    return result;
}

bool QrCode::getBit(long val, int i) {
    return ((val >> i) & 1) != 0;
}

}
