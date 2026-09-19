#pragma once

#include "QrCode.hpp"
#include "QrTopology.hpp"
#include "QrShapeRenderer.hpp"
#include "QrEyeRenderer.hpp"
#include "QrSvgBuilder.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace qrcodegen {

struct ColorRGBA {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    static ColorRGBA fromHex(const std::string &hex);
    std::string toHex() const;
};

class ImageBuffer final {
public:
    int width = 0;
    int height = 0;
    std::vector<std::uint8_t> pixels;

    ImageBuffer() = default;
    ImageBuffer(int w, int h, ColorRGBA fill = {255, 255, 255, 255});

    ColorRGBA getPixel(int x, int y) const;
    void setPixel(int x, int y, ColorRGBA color);
    void clear(ColorRGBA fill = {255, 255, 255, 255});
};

class QrImageProcessor final {
public:
    static ImageBuffer resizeBilinear(const ImageBuffer &src, int targetWidth, int targetHeight);
    static void blendImage(ImageBuffer &dst, const ImageBuffer &src, int dstX, int dstY, double opacity = 1.0);
    static void applyDimming(ImageBuffer &canvas, ColorRGBA dimColor, double opacity);

    static ColorRGBA calculateAverageColor(const ImageBuffer &img);
    static ColorRGBA calculateOptimalContrastColor(const ImageBuffer &background, bool preferDark = true);
    static double calculateLuminance(ColorRGBA c);
    static double calculateContrastRatio(ColorRGBA c1, ColorRGBA c2);

    static void fillRect(ImageBuffer &dst, int x, int y, int w, int h, ColorRGBA color);
    static void fillCircle(ImageBuffer &dst, double cx, double cy, double radius, ColorRGBA color);
    static void fillRoundedRect(ImageBuffer &dst, double x, double y, double w, double h, const CornerRadii &radii, ColorRGBA color);
    static void fillHollowFrame(ImageBuffer &dst, double x, double y, double outerSize, double borderWidth,
                                const CornerRadii &outerRadii, const CornerRadii &innerRadii, ColorRGBA color);

    static ImageBuffer renderRasterQr(const QrCode &qr, const SvgRenderConfig &config, const ImageBuffer *backgroundImage = nullptr, const ImageBuffer *logoImage = nullptr);
    static std::vector<std::uint8_t> encodePng(const ImageBuffer &img);
};

}
