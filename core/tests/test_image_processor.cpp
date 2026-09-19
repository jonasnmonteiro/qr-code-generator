#include "QrCode.hpp"
#include "QrImageProcessor.hpp"
#include <cassert>
#include <iostream>
#include <vector>

using namespace qrcodegen;

int main() {
    ColorRGBA white = {255, 255, 255, 255};
    ColorRGBA black = {0, 0, 0, 255};
    double contrast = QrImageProcessor::calculateContrastRatio(white, black);
    assert(contrast > 20.0);

    ImageBuffer bg(200, 200, {240, 240, 250, 255});
    ColorRGBA optimalDark = QrImageProcessor::calculateOptimalContrastColor(bg, true);
    assert(QrImageProcessor::calculateLuminance(optimalDark) < 0.1);

    ImageBuffer logo(50, 50, {255, 0, 0, 255});
    ImageBuffer resizedLogo = QrImageProcessor::resizeBilinear(logo, 100, 100);
    assert(resizedLogo.width == 100 && resizedLogo.height == 100);

    QrCode qr = QrCode::encodeText("https://github.com/jonasnmonteiro/qr-code-generator", QrCode::Ecc::HIGH);

    SvgRenderConfig config;
    config.canvasSize = 256.0;
    config.margin = 16.0;
    config.backgroundColor = "#FFFFFF";
    config.moduleColor = optimalDark.toHex();
    config.moduleStyle = ModuleStyle::FLUID;

    config.eyeTopLeft.outerRadii = CornerRadii::all(3.5);
    config.eyeTopLeft.innerRadii = CornerRadii::all(1.5);
    config.eyeTopLeft.outerColor = "#4F46E5";
    config.eyeTopLeft.innerColor = "#06B6D4";

    config.eyeTopRight.outerRadii = {3.5, 0.0, 3.5, 0.0};
    config.eyeTopRight.innerRadii = {1.5, 0.0, 1.5, 0.0};
    config.eyeTopRight.outerColor = "#4F46E5";
    config.eyeTopRight.innerColor = "#06B6D4";

    config.eyeBottomLeft.outerRadii = {0.0, 3.5, 0.0, 3.5};
    config.eyeBottomLeft.innerRadii = {0.0, 1.5, 0.0, 1.5};
    config.eyeBottomLeft.outerColor = "#4F46E5";
    config.eyeBottomLeft.innerColor = "#06B6D4";

    config.logo.shape = LogoShape::CIRCLE;
    config.logo.scale = 0.22;
    config.logo.paddingRatio = 0.04;

    ImageBuffer raster = QrImageProcessor::renderRasterQr(qr, config, &bg, &resizedLogo);
    assert(raster.width == 256 && raster.height == 256);

    std::vector<std::uint8_t> pngBytes = QrImageProcessor::encodePng(raster);
    assert(pngBytes.size() > 100);
    assert(pngBytes[0] == 0x89 && pngBytes[1] == 0x50 && pngBytes[2] == 0x4E && pngBytes[3] == 0x47);

    std::cout << "All Block 4 ImageProcessor raster and PNG assertions passed successfully!" << std::endl;
    return 0;
}
