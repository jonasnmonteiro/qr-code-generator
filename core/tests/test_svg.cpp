#include "QrCode.hpp"
#include "QrSvgBuilder.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace qrcodegen;

int main() {
    QrCode qr = QrCode::encodeText("https://github.com/jonasnmonteiro/qr-code-generator", QrCode::Ecc::HIGH);

    SvgRenderConfig config;
    config.canvasSize = 512.0;
    config.margin = 24.0;
    config.backgroundColor = "#FFFFFF";
    config.moduleStyle = ModuleStyle::FLUID;

    config.moduleGradient.type = GradientType::LINEAR;
    config.moduleGradient.rotationDegrees = 45.0;
    config.moduleGradient.stops = {
        {0.0, "#4F46E5", 1.0},
        {1.0, "#06B6D4", 1.0}
    };

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
    config.logo.dataUri = "data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxMCIgaGVpZ2h0PSIxMCI+PC9zdmc+";

    std::string svg = QrSvgBuilder::build(qr, config);

    assert(!svg.empty());
    assert(svg.find("<svg") != std::string::npos);
    assert(svg.find("linearGradient") != std::string::npos);
    assert(svg.find("positioning_eyes") != std::string::npos);
    assert(svg.find("</svg>") != std::string::npos);

    std::cout << "All Block 3 SvgBuilder and EyeRenderer assertions passed successfully!" << std::endl;
    return 0;
}
