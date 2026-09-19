#pragma once

#include "QrCode.hpp"
#include "QrTopology.hpp"
#include "QrShapeRenderer.hpp"
#include "QrEyeRenderer.hpp"
#include <string>
#include <vector>

namespace qrcodegen {

enum class GradientType {
    NONE = 0,
    LINEAR,
    RADIAL
};

struct ColorStop {
    double offset = 0.0;
    std::string color;
    double opacity = 1.0;
};

struct GradientOptions {
    GradientType type = GradientType::NONE;
    double rotationDegrees = 0.0;
    std::vector<ColorStop> stops;
};

struct SvgLogoOptions {
    std::string dataUri;
    LogoShape shape = LogoShape::NONE;
    double scale = 0.22;
    double paddingRatio = 0.04;
    double borderRadiusRatio = 0.05;
};

struct SvgRenderConfig {
    double canvasSize = 400.0;
    double margin = 20.0;
    std::string backgroundColor = "#FFFFFF";
    std::string moduleColor = "#000000";
    ModuleStyle moduleStyle = ModuleStyle::SQUARE;
    double moduleScale = 1.0;

    EyeStyle eyeTopLeft;
    EyeStyle eyeTopRight;
    EyeStyle eyeBottomLeft;

    GradientOptions moduleGradient;
    GradientOptions backgroundGradient;

    SvgLogoOptions logo;
};

class QrSvgBuilder final {
public:
    static std::string build(const QrCode &qr, const SvgRenderConfig &config);

private:
    static std::string buildGradients(const SvgRenderConfig &config, const std::string &idPrefix);
    static std::string buildBackground(const SvgRenderConfig &config, double canvasSize, const std::string &bgFill);
    static std::string buildLogo(const SvgLogoOptions &logo, double canvasSize, double margin, double matrixSize, double cellSize);
};

}
