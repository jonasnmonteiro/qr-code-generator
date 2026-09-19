#include "QrCode.hpp"
#include "QrTopology.hpp"
#include "QrShapeRenderer.hpp"
#include "QrEyeRenderer.hpp"
#include "QrSvgBuilder.hpp"
#include "QrImageProcessor.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;
using namespace qrcodegen;

static QrCode::Ecc parseEcc(int ecl) {
    switch (ecl) {
        case 0:  return QrCode::Ecc::LOW;
        case 1:  return QrCode::Ecc::MEDIUM;
        case 2:  return QrCode::Ecc::QUARTILE;
        case 3:  return QrCode::Ecc::HIGH;
        default: return QrCode::Ecc::MEDIUM;
    }
}

static ModuleStyle parseModuleStyle(int style) {
    switch (style) {
        case 0:  return ModuleStyle::SQUARE;
        case 1:  return ModuleStyle::DOTS;
        case 2:  return ModuleStyle::ROUNDED;
        case 3:  return ModuleStyle::EXTRA_ROUNDED;
        case 4:  return ModuleStyle::CLASSY;
        case 5:  return ModuleStyle::CLASSY_ROUNDED;
        case 6:  return ModuleStyle::FLUID;
        default: return ModuleStyle::SQUARE;
    }
}

static LogoShape parseLogoShape(int shape) {
    switch (shape) {
        case 0:  return LogoShape::NONE;
        case 1:  return LogoShape::SQUARE;
        case 2:  return LogoShape::ROUNDED_RECT;
        case 3:  return LogoShape::CIRCLE;
        default: return LogoShape::NONE;
    }
}

static GradientType parseGradientType(int gradType) {
    switch (gradType) {
        case 0:  return GradientType::NONE;
        case 1:  return GradientType::LINEAR;
        case 2:  return GradientType::RADIAL;
        default: return GradientType::NONE;
    }
}

class WasmQrEngine {
public:
    static std::string generateSvg(
        const std::string &text,
        int ecl,
        double size,
        double margin,
        const std::string &bgColor,
        const std::string &moduleColor,
        int moduleStyle,
        double moduleScale,
        int logoShape,
        double logoScale,
        double logoPadding,
        const std::string &logoDataUri,
        int gradType,
        double gradRotation,
        const std::string &gradColorStart,
        const std::string &gradColorEnd,
        double eyeRadiusOuter,
        double eyeRadiusInner,
        const std::string &eyeColorOuter,
        const std::string &eyeColorInner
    ) {
        QrCode qr = QrCode::encodeText(text.c_str(), parseEcc(ecl));

        SvgRenderConfig config;
        config.canvasSize = size;
        config.margin = margin;
        config.backgroundColor = bgColor;
        config.moduleColor = moduleColor;
        config.moduleStyle = parseModuleStyle(moduleStyle);
        config.moduleScale = moduleScale;

        config.logo.shape = parseLogoShape(logoShape);
        config.logo.scale = logoScale;
        config.logo.paddingRatio = logoPadding;
        config.logo.dataUri = logoDataUri;

        config.moduleGradient.type = parseGradientType(gradType);
        config.moduleGradient.rotationDegrees = gradRotation;
        if (config.moduleGradient.type != GradientType::NONE && !gradColorStart.empty() && !gradColorEnd.empty()) {
            config.moduleGradient.stops = {
                {0.0, gradColorStart, 1.0},
                {1.0, gradColorEnd, 1.0}
            };
        }

        EyeStyle baseEye;
        baseEye.outerRadii = CornerRadii::all(eyeRadiusOuter);
        baseEye.innerRadii = CornerRadii::all(eyeRadiusInner);
        baseEye.outerColor = eyeColorOuter.empty() ? moduleColor : eyeColorOuter;
        baseEye.innerColor = eyeColorInner.empty() ? moduleColor : eyeColorInner;

        config.eyeTopLeft = baseEye;
        config.eyeTopRight = baseEye;
        config.eyeBottomLeft = baseEye;

        return QrSvgBuilder::build(qr, config);
    }

    static val generatePng(
        const std::string &text,
        int ecl,
        int size,
        int margin,
        const std::string &bgColor,
        const std::string &moduleColor,
        int moduleStyle,
        double eyeRadiusOuter,
        double eyeRadiusInner,
        const std::string &eyeColorOuter,
        const std::string &eyeColorInner
    ) {
        QrCode qr = QrCode::encodeText(text.c_str(), parseEcc(ecl));

        SvgRenderConfig config;
        config.canvasSize = size;
        config.margin = margin;
        config.backgroundColor = bgColor;
        config.moduleColor = moduleColor;
        config.moduleStyle = parseModuleStyle(moduleStyle);

        EyeStyle baseEye;
        baseEye.outerRadii = CornerRadii::all(eyeRadiusOuter);
        baseEye.innerRadii = CornerRadii::all(eyeRadiusInner);
        baseEye.outerColor = eyeColorOuter.empty() ? moduleColor : eyeColorOuter;
        baseEye.innerColor = eyeColorInner.empty() ? moduleColor : eyeColorInner;

        config.eyeTopLeft = baseEye;
        config.eyeTopRight = baseEye;
        config.eyeBottomLeft = baseEye;

        ImageBuffer raster = QrImageProcessor::renderRasterQr(qr, config);
        std::vector<std::uint8_t> png = QrImageProcessor::encodePng(raster);

        return val(typed_memory_view(png.size(), png.data()));
    }

    static std::string calculateOptimalModuleColor(const std::string &bgHex) {
        ColorRGBA bg = ColorRGBA::fromHex(bgHex);
        ImageBuffer dummy(10, 10, bg);
        ColorRGBA optimal = QrImageProcessor::calculateOptimalContrastColor(dummy, true);
        return optimal.toHex();
    }
};

EMSCRIPTEN_BINDINGS(QrEngineModule) {
    class_<WasmQrEngine>("WasmQrEngine")
        .class_function("generateSvg", &WasmQrEngine::generateSvg)
        .class_function("generatePng", &WasmQrEngine::generatePng)
        .class_function("calculateOptimalModuleColor", &WasmQrEngine::calculateOptimalModuleColor);
}

#endif
