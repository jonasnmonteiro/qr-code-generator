#include "QrImageProcessor.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

namespace qrcodegen {

static std::uint8_t parseHexPair(const char *p) {
    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };
    return static_cast<std::uint8_t>((hexVal(p[0]) << 4) | hexVal(p[1]));
}

ColorRGBA ColorRGBA::fromHex(const std::string &hex) {
    if (hex.empty()) {
        return {0, 0, 0, 255};
    }
    std::string s = hex;
    if (s[0] == '#') {
        s = s.substr(1);
    }
    if (s.size() == 3) {
        std::string expanded;
        for (char c : s) {
            expanded += c;
            expanded += c;
        }
        s = expanded;
    }
    if (s.size() == 6) {
        return {
            parseHexPair(s.c_str()),
            parseHexPair(s.c_str() + 2),
            parseHexPair(s.c_str() + 4),
            255
        };
    }
    if (s.size() == 8) {
        return {
            parseHexPair(s.c_str()),
            parseHexPair(s.c_str() + 2),
            parseHexPair(s.c_str() + 4),
            parseHexPair(s.c_str() + 6)
        };
    }
    return {0, 0, 0, 255};
}

std::string ColorRGBA::toHex() const {
    char buf[10];
    std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", r, g, b);
    return std::string(buf);
}

ImageBuffer::ImageBuffer(int w, int h, ColorRGBA fill) :
    width(w), height(h), pixels(w * h * 4, 0) {
    clear(fill);
}

ColorRGBA ImageBuffer::getPixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return {0, 0, 0, 0};
    }
    int idx = (y * width + x) * 4;
    return {pixels[idx], pixels[idx + 1], pixels[idx + 2], pixels[idx + 3]};
}

void ImageBuffer::setPixel(int x, int y, ColorRGBA color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    int idx = (y * width + x) * 4;
    pixels[idx]     = color.r;
    pixels[idx + 1] = color.g;
    pixels[idx + 2] = color.b;
    pixels[idx + 3] = color.a;
}

void ImageBuffer::clear(ColorRGBA fill) {
    for (int i = 0; i < width * height; i++) {
        pixels[i * 4]     = fill.r;
        pixels[i * 4 + 1] = fill.g;
        pixels[i * 4 + 2] = fill.b;
        pixels[i * 4 + 3] = fill.a;
    }
}

ImageBuffer QrImageProcessor::resizeBilinear(const ImageBuffer &src, int targetWidth, int targetHeight) {
    ImageBuffer dst(targetWidth, targetHeight);
    if (src.width == 0 || src.height == 0 || targetWidth == 0 || targetHeight == 0) {
        return dst;
    }

    double scaleX = static_cast<double>(src.width) / targetWidth;
    double scaleY = static_cast<double>(src.height) / targetHeight;

    for (int y = 0; y < targetHeight; y++) {
        double srcY = (y + 0.5) * scaleY - 0.5;
        int y0 = std::clamp(static_cast<int>(std::floor(srcY)), 0, src.height - 1);
        int y1 = std::clamp(y0 + 1, 0, src.height - 1);
        double dy = srcY - y0;

        for (int x = 0; x < targetWidth; x++) {
            double srcX = (x + 0.5) * scaleX - 0.5;
            int x0 = std::clamp(static_cast<int>(std::floor(srcX)), 0, src.width - 1);
            int x1 = std::clamp(x0 + 1, 0, src.width - 1);
            double dx = srcX - x0;

            ColorRGBA c00 = src.getPixel(x0, y0);
            ColorRGBA c10 = src.getPixel(x1, y0);
            ColorRGBA c01 = src.getPixel(x0, y1);
            ColorRGBA c11 = src.getPixel(x1, y1);

            double w00 = (1.0 - dx) * (1.0 - dy);
            double w10 = dx * (1.0 - dy);
            double w01 = (1.0 - dx) * dy;
            double w11 = dx * dy;

            ColorRGBA out;
            out.r = static_cast<std::uint8_t>(c00.r * w00 + c10.r * w10 + c01.r * w01 + c11.r * w11);
            out.g = static_cast<std::uint8_t>(c00.g * w00 + c10.g * w10 + c01.g * w01 + c11.g * w11);
            out.b = static_cast<std::uint8_t>(c00.b * w00 + c10.b * w10 + c01.b * w01 + c11.b * w11);
            out.a = static_cast<std::uint8_t>(c00.a * w00 + c10.a * w10 + c01.a * w01 + c11.a * w11);

            dst.setPixel(x, y, out);
        }
    }
    return dst;
}

void QrImageProcessor::blendImage(ImageBuffer &dst, const ImageBuffer &src, int dstX, int dstY, double opacity) {
    double alphaMod = std::clamp(opacity, 0.0, 1.0);

    for (int sy = 0; sy < src.height; sy++) {
        int dy = dstY + sy;
        if (dy < 0 || dy >= dst.height) continue;

        for (int sx = 0; sx < src.width; sx++) {
            int dx = dstX + sx;
            if (dx < 0 || dx >= dst.width) continue;

            ColorRGBA s = src.getPixel(sx, sy);
            ColorRGBA d = dst.getPixel(dx, dy);

            double sa = (s.a / 255.0) * alphaMod;
            double da = d.a / 255.0;

            double outA = sa + da * (1.0 - sa);
            if (outA > 0.001) {
                double outR = (s.r * sa + d.r * da * (1.0 - sa)) / outA;
                double outG = (s.g * sa + d.g * da * (1.0 - sa)) / outA;
                double outB = (s.b * sa + d.b * da * (1.0 - sa)) / outA;

                dst.setPixel(dx, dy, {
                    static_cast<std::uint8_t>(std::clamp(outR, 0.0, 255.0)),
                    static_cast<std::uint8_t>(std::clamp(outG, 0.0, 255.0)),
                    static_cast<std::uint8_t>(std::clamp(outB, 0.0, 255.0)),
                    static_cast<std::uint8_t>(std::clamp(outA * 255.0, 0.0, 255.0))
                });
            }
        }
    }
}

void QrImageProcessor::applyDimming(ImageBuffer &canvas, ColorRGBA dimColor, double opacity) {
    ImageBuffer dim(canvas.width, canvas.height, dimColor);
    blendImage(canvas, dim, 0, 0, opacity);
}

double QrImageProcessor::calculateLuminance(ColorRGBA c) {
    auto toLinear = [](double channel) {
        channel /= 255.0;
        return (channel <= 0.03928) ? (channel / 12.92) : std::pow((channel + 0.055) / 1.055, 2.4);
    };
    return 0.2126 * toLinear(c.r) + 0.7152 * toLinear(c.g) + 0.0722 * toLinear(c.b);
}

double QrImageProcessor::calculateContrastRatio(ColorRGBA c1, ColorRGBA c2) {
    double l1 = calculateLuminance(c1);
    double l2 = calculateLuminance(c2);
    double lighter = std::max(l1, l2);
    double darker  = std::min(l1, l2);
    return (lighter + 0.05) / (darker + 0.05);
}

ColorRGBA QrImageProcessor::calculateAverageColor(const ImageBuffer &img) {
    if (img.width == 0 || img.height == 0) {
        return {255, 255, 255, 255};
    }
    double sumR = 0, sumG = 0, sumB = 0, count = 0;
    int step = std::max(1, (img.width * img.height) / 10000);

    for (int i = 0; i < img.width * img.height; i += step) {
        sumR += img.pixels[i * 4];
        sumG += img.pixels[i * 4 + 1];
        sumB += img.pixels[i * 4 + 2];
        count += 1.0;
    }

    return {
        static_cast<std::uint8_t>(sumR / count),
        static_cast<std::uint8_t>(sumG / count),
        static_cast<std::uint8_t>(sumB / count),
        255
    };
}

ColorRGBA QrImageProcessor::calculateOptimalContrastColor(const ImageBuffer &background, bool preferDark) {
    ColorRGBA avg = calculateAverageColor(background);
    double lum = calculateLuminance(avg);

    if (lum > 0.45) {
        int r = std::max(0, static_cast<int>(avg.r * 0.15));
        int g = std::max(0, static_cast<int>(avg.g * 0.15));
        int b = std::max(0, static_cast<int>(avg.b * 0.15));
        return {static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), 255};
    } else {
        int r = std::min(255, static_cast<int>(255 - (255 - avg.r) * 0.15));
        int g = std::min(255, static_cast<int>(255 - (255 - avg.g) * 0.15));
        int b = std::min(255, static_cast<int>(255 - (255 - avg.b) * 0.15));
        return {static_cast<std::uint8_t>(r), static_cast<std::uint8_t>(g), static_cast<std::uint8_t>(b), 255};
    }
}

void QrImageProcessor::fillRect(ImageBuffer &dst, int x, int y, int w, int h, ColorRGBA color) {
    int minX = std::max(0, x);
    int maxX = std::min(dst.width, x + w);
    int minY = std::max(0, y);
    int maxY = std::min(dst.height, y + h);

    for (int py = minY; py < maxY; py++) {
        for (int px = minX; px < maxX; px++) {
            dst.setPixel(px, py, color);
        }
    }
}

void QrImageProcessor::fillCircle(ImageBuffer &dst, double cx, double cy, double radius, ColorRGBA color) {
    int minX = std::max(0, static_cast<int>(std::floor(cx - radius)));
    int maxX = std::min(dst.width, static_cast<int>(std::ceil(cx + radius + 1)));
    int minY = std::max(0, static_cast<int>(std::floor(cy - radius)));
    int maxY = std::min(dst.height, static_cast<int>(std::ceil(cy + radius + 1)));
    double rSq = radius * radius;

    for (int py = minY; py < maxY; py++) {
        for (int px = minX; px < maxX; px++) {
            double dx = (px + 0.5) - cx;
            double dy = (py + 0.5) - cy;
            double distSq = dx * dx + dy * dy;
            if (distSq <= rSq) {
                dst.setPixel(px, py, color);
            }
        }
    }
}

void QrImageProcessor::fillRoundedRect(ImageBuffer &dst, double x, double y, double w, double h, const CornerRadii &radii, ColorRGBA color) {
    int minX = std::max(0, static_cast<int>(std::floor(x)));
    int maxX = std::min(dst.width, static_cast<int>(std::ceil(x + w)));
    int minY = std::max(0, static_cast<int>(std::floor(y)));
    int maxY = std::min(dst.height, static_cast<int>(std::ceil(y + h)));

    double maxR = std::min(w, h) / 2.0;
    double rTL = std::clamp(radii.topLeft, 0.0, maxR);
    double rTR = std::clamp(radii.topRight, 0.0, maxR);
    double rBR = std::clamp(radii.bottomRight, 0.0, maxR);
    double rBL = std::clamp(radii.bottomLeft, 0.0, maxR);

    for (int py = minY; py < maxY; py++) {
        double cy = py + 0.5;
        for (int px = minX; px < maxX; px++) {
            double cx = px + 0.5;
            bool inside = true;

            if (cx < x + rTL && cy < y + rTL) {
                double dx = cx - (x + rTL);
                double dy = cy - (y + rTL);
                inside = (dx * dx + dy * dy <= rTL * rTL);
            } else if (cx > x + w - rTR && cy < y + rTR) {
                double dx = cx - (x + w - rTR);
                double dy = cy - (y + rTR);
                inside = (dx * dx + dy * dy <= rTR * rTR);
            } else if (cx > x + w - rBR && cy > y + h - rBR) {
                double dx = cx - (x + w - rBR);
                double dy = cy - (y + h - rBR);
                inside = (dx * dx + dy * dy <= rBR * rBR);
            } else if (cx < x + rBL && cy > y + h - rBL) {
                double dx = cx - (x + rBL);
                double dy = cy - (y + h - rBL);
                inside = (dx * dx + dy * dy <= rBL * rBL);
            }

            if (inside) {
                dst.setPixel(px, py, color);
            }
        }
    }
}

void QrImageProcessor::fillHollowFrame(
    ImageBuffer &dst,
    double x,
    double y,
    double outerSize,
    double borderWidth,
    const CornerRadii &outerRadii,
    const CornerRadii &innerRadii,
    ColorRGBA color
) {
    fillRoundedRect(dst, x, y, outerSize, outerSize, outerRadii, color);
    fillRoundedRect(dst, x + borderWidth, y + borderWidth, outerSize - borderWidth * 2.0, outerSize - borderWidth * 2.0, innerRadii, {0, 0, 0, 0});
}

ImageBuffer QrImageProcessor::renderRasterQr(
    const QrCode &qr,
    const SvgRenderConfig &config,
    const ImageBuffer *backgroundImage,
    const ImageBuffer *logoImage
) {
    int targetDim = static_cast<int>(config.canvasSize);
    ImageBuffer canvas(targetDim, targetDim, ColorRGBA::fromHex(config.backgroundColor));

    if (backgroundImage && backgroundImage->width > 0) {
        ImageBuffer scaledBg = resizeBilinear(*backgroundImage, targetDim, targetDim);
        blendImage(canvas, scaledBg, 0, 0, 1.0);
    }

    LogoOptions logoOpts;
    logoOpts.shape = config.logo.shape;
    logoOpts.scale = config.logo.scale;
    logoOpts.paddingRatio = config.logo.paddingRatio;
    logoOpts.borderRadiusRatio = config.logo.borderRadiusRatio;

    QrTopology topology(qr, logoOpts);
    int matrixSize = topology.getSize();
    double margin = config.margin;
    double viewportSize = config.canvasSize - (margin * 2.0);
    double cellSize = viewportSize / matrixSize;

    ColorRGBA modColor = ColorRGBA::fromHex(config.moduleColor);

    for (int y = 0; y < matrixSize; y++) {
        for (int x = 0; x < matrixSize; x++) {
            if (topology.shouldRenderDataModule(x, y)) {
                double px = margin + x * cellSize;
                double py = margin + y * cellSize;
                NeighborMask mask = topology.getNeighborMask(x, y);

                if (config.moduleStyle == ModuleStyle::DOTS || (mask.orthogonalCount == 0 && config.moduleStyle != ModuleStyle::SQUARE)) {
                    fillCircle(canvas, px + cellSize / 2.0, py + cellSize / 2.0, (cellSize / 2.0) * config.moduleScale, modColor);
                } else if (config.moduleStyle == ModuleStyle::FLUID || config.moduleStyle == ModuleStyle::ROUNDED || config.moduleStyle == ModuleStyle::EXTRA_ROUNDED) {
                    double r = cellSize * 0.45;
                    CornerRadii radii;
                    radii.topLeft     = (!mask.north && !mask.west) ? r : 0.0;
                    radii.topRight    = (!mask.north && !mask.east) ? r : 0.0;
                    radii.bottomRight = (!mask.south && !mask.east) ? r : 0.0;
                    radii.bottomLeft  = (!mask.south && !mask.west) ? r : 0.0;
                    fillRoundedRect(canvas, px, py, cellSize, cellSize, radii, modColor);
                } else {
                    fillRect(canvas, static_cast<int>(px), static_cast<int>(py), static_cast<int>(std::ceil(cellSize)), static_cast<int>(std::ceil(cellSize)), modColor);
                }
            }
        }
    }

    auto renderEye = [&](EyePosition pos, const EyeStyle &style) {
        double cellX = (pos == EyePosition::TOP_RIGHT) ? (matrixSize - 7) : 0.0;
        double cellY = (pos == EyePosition::BOTTOM_LEFT) ? (matrixSize - 7) : 0.0;
        double ox = margin + cellX * cellSize;
        double oy = margin + cellY * cellSize;

        CornerRadii outerR = {
            style.outerRadii.topLeft * cellSize,
            style.outerRadii.topRight * cellSize,
            style.outerRadii.bottomRight * cellSize,
            style.outerRadii.bottomLeft * cellSize
        };
        CornerRadii innerR = {
            style.innerRadii.topLeft * cellSize,
            style.innerRadii.topRight * cellSize,
            style.innerRadii.bottomRight * cellSize,
            style.innerRadii.bottomLeft * cellSize
        };

        fillRoundedRect(canvas, ox, oy, 7.0 * cellSize, 7.0 * cellSize, outerR, ColorRGBA::fromHex(style.outerColor));
        fillRoundedRect(canvas, ox + cellSize, oy + cellSize, 5.0 * cellSize, 5.0 * cellSize, outerR, ColorRGBA::fromHex(config.backgroundColor));
        fillRoundedRect(canvas, ox + 2.0 * cellSize, oy + 2.0 * cellSize, 3.0 * cellSize, 3.0 * cellSize, innerR, ColorRGBA::fromHex(style.innerColor));
    };

    renderEye(EyePosition::TOP_LEFT, config.eyeTopLeft);
    renderEye(EyePosition::TOP_RIGHT, config.eyeTopRight);
    renderEye(EyePosition::BOTTOM_LEFT, config.eyeBottomLeft);

    if (logoImage && logoImage->width > 0 && config.logo.scale > 0.0) {
        double contentSize = config.canvasSize - margin * 2.0;
        double logoDim = contentSize * config.logo.scale;
        double center = config.canvasSize / 2.0;
        double logoX = center - logoDim / 2.0;
        double logoY = center - logoDim / 2.0;

        if (config.logo.paddingRatio > 0.0) {
            double pad = contentSize * config.logo.paddingRatio;
            double padDim = logoDim + pad * 2.0;
            if (config.logo.shape == LogoShape::CIRCLE) {
                fillCircle(canvas, center, center, padDim / 2.0, {255, 255, 255, 255});
            } else {
                fillRoundedRect(canvas, center - padDim / 2.0, center - padDim / 2.0, padDim, padDim, CornerRadii::all(padDim * config.logo.borderRadiusRatio), {255, 255, 255, 255});
            }
        }

        ImageBuffer scaledLogo = resizeBilinear(*logoImage, static_cast<int>(logoDim), static_cast<int>(logoDim));
        blendImage(canvas, scaledLogo, static_cast<int>(logoX), static_cast<int>(logoY), 1.0);
    }

    return canvas;
}

static std::uint32_t calculateCrc32(const std::uint8_t *data, std::size_t len) {
    std::uint32_t crc = 0xFFFFFFFF;
    for (std::size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int k = 0; k < 8; k++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

static void appendChunk(std::vector<std::uint8_t> &out, const char *type, const std::uint8_t *data, std::size_t len) {
    std::uint32_t lengthBe = __builtin_bswap32(static_cast<std::uint32_t>(len));
    const std::uint8_t *lenBytes = reinterpret_cast<const std::uint8_t*>(&lengthBe);
    out.insert(out.end(), lenBytes, lenBytes + 4);

    std::size_t typeStart = out.size();
    out.insert(out.end(), type, type + 4);
    if (len > 0) {
        out.insert(out.end(), data, data + len);
    }

    std::uint32_t crc = calculateCrc32(&out[typeStart], len + 4);
    std::uint32_t crcBe = __builtin_bswap32(crc);
    const std::uint8_t *crcBytes = reinterpret_cast<const std::uint8_t*>(&crcBe);
    out.insert(out.end(), crcBytes, crcBytes + 4);
}

std::vector<std::uint8_t> QrImageProcessor::encodePng(const ImageBuffer &img) {
    std::vector<std::uint8_t> png = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

    std::uint8_t ihdr[13];
    std::uint32_t wBe = __builtin_bswap32(img.width);
    std::uint32_t hBe = __builtin_bswap32(img.height);
    std::memcpy(&ihdr[0], &wBe, 4);
    std::memcpy(&ihdr[4], &hBe, 4);
    ihdr[8] = 8;
    ihdr[9] = 6;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;
    appendChunk(png, "IHDR", ihdr, sizeof(ihdr));

    std::vector<std::uint8_t> rawScanlines;
    rawScanlines.reserve((img.width * 4 + 1) * img.height);
    for (int y = 0; y < img.height; y++) {
        rawScanlines.push_back(0);
        const std::uint8_t *rowStart = &img.pixels[y * img.width * 4];
        rawScanlines.insert(rawScanlines.end(), rowStart, rowStart + (img.width * 4));
    }

    std::vector<std::uint8_t> idat;
    idat.push_back(0x78);
    idat.push_back(0x01);

    std::size_t remaining = rawScanlines.size();
    std::size_t offset = 0;
    while (remaining > 0) {
        std::size_t blockSize = std::min(remaining, static_cast<std::size_t>(65535));
        bool isLast = (remaining == blockSize);
        idat.push_back(isLast ? 0x01 : 0x00);
        std::uint16_t len16 = static_cast<std::uint16_t>(blockSize);
        std::uint16_t nlen16 = ~len16;
        idat.push_back(len16 & 0xFF);
        idat.push_back((len16 >> 8) & 0xFF);
        idat.push_back(nlen16 & 0xFF);
        idat.push_back((nlen16 >> 8) & 0xFF);

        idat.insert(idat.end(), rawScanlines.begin() + offset, rawScanlines.begin() + offset + blockSize);
        offset += blockSize;
        remaining -= blockSize;
    }

    std::uint32_t s1 = 1, s2 = 0;
    for (std::uint8_t b : rawScanlines) {
        s1 = (s1 + b) % 65521;
        s2 = (s2 + s1) % 65521;
    }
    std::uint32_t adler = (s2 << 16) | s1;
    std::uint32_t adlerBe = __builtin_bswap32(adler);
    const std::uint8_t *adlerBytes = reinterpret_cast<const std::uint8_t*>(&adlerBe);
    idat.insert(idat.end(), adlerBytes, adlerBytes + 4);

    appendChunk(png, "IDAT", idat.data(), idat.size());
    appendChunk(png, "IEND", nullptr, 0);

    return png;
}

}
