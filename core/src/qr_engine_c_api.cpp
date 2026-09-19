#include "qr_engine_c_api.h"
#include "QrCode.hpp"
#include "QrTopology.hpp"
#include "QrShapeRenderer.hpp"
#include "QrEyeRenderer.hpp"
#include "QrSvgBuilder.hpp"
#include "QrImageProcessor.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using namespace qrcodegen;

static QrCode::Ecc parseEccC(int ecl) {
    switch (ecl) {
        case 0:  return QrCode::Ecc::LOW;
        case 1:  return QrCode::Ecc::MEDIUM;
        case 2:  return QrCode::Ecc::QUARTILE;
        case 3:  return QrCode::Ecc::HIGH;
        default: return QrCode::Ecc::MEDIUM;
    }
}

static ModuleStyle parseModuleStyleC(int style) {
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

static LogoShape parseLogoShapeC(int shape) {
    switch (shape) {
        case 0:  return LogoShape::NONE;
        case 1:  return LogoShape::SQUARE;
        case 2:  return LogoShape::ROUNDED_RECT;
        case 3:  return LogoShape::CIRCLE;
        default: return LogoShape::NONE;
    }
}

static GradientType parseGradientTypeC(int gradType) {
    switch (gradType) {
        case 0:  return GradientType::NONE;
        case 1:  return GradientType::LINEAR;
        case 2:  return GradientType::RADIAL;
        default: return GradientType::NONE;
    }
}

static SvgRenderConfig buildConfigFromC(const C_QrConfig *config) {
    SvgRenderConfig cfg;
    cfg.canvasSize = (config->canvas_size > 0.0) ? config->canvas_size : 400.0;
    cfg.margin = (config->margin >= 0.0) ? config->margin : 20.0;
    cfg.backgroundColor = config->background_color ? config->background_color : "#FFFFFF";
    cfg.moduleColor = config->module_color ? config->module_color : "#000000";
    cfg.moduleStyle = parseModuleStyleC(config->module_style);
    cfg.moduleScale = (config->module_scale > 0.0) ? config->module_scale : 1.0;

    cfg.logo.shape = parseLogoShapeC(config->logo_shape);
    cfg.logo.scale = config->logo_scale;
    cfg.logo.paddingRatio = config->logo_padding_ratio;
    cfg.logo.dataUri = config->logo_data_uri ? config->logo_data_uri : "";

    cfg.moduleGradient.type = parseGradientTypeC(config->gradient_type);
    cfg.moduleGradient.rotationDegrees = config->gradient_rotation;
    if (cfg.moduleGradient.type != GradientType::NONE &&
        config->gradient_color_start && config->gradient_color_end) {
        cfg.moduleGradient.stops = {
            {0.0, config->gradient_color_start, 1.0},
            {1.0, config->gradient_color_end, 1.0}
        };
    }

    EyeStyle baseEye;
    baseEye.outerRadii = CornerRadii::all(config->eye_radius_outer);
    baseEye.innerRadii = CornerRadii::all(config->eye_radius_inner);
    baseEye.outerColor = config->eye_color_outer ? config->eye_color_outer : cfg.moduleColor;
    baseEye.innerColor = config->eye_color_inner ? config->eye_color_inner : cfg.moduleColor;

    cfg.eyeTopLeft = baseEye;
    cfg.eyeTopRight = baseEye;
    cfg.eyeBottomLeft = baseEye;

    return cfg;
}

extern "C" {

C_StringResult qr_generate_svg_c(const C_QrConfig *config) {
    C_StringResult result = {nullptr, 0, nullptr};
    if (!config || !config->text) {
        result.error_code = 1;
        result.error_message = "Invalid configuration or null text payload";
        return result;
    }

    try {
        QrCode qr = QrCode::encodeText(config->text, parseEccC(config->error_correction_level));
        SvgRenderConfig cfg = buildConfigFromC(config);
        std::string svg = QrSvgBuilder::build(qr, cfg);

        char *buffer = static_cast<char*>(std::malloc(svg.size() + 1));
        std::memcpy(buffer, svg.c_str(), svg.size() + 1);

        result.svg_string = buffer;
        result.error_code = 0;
    } catch (const std::exception &e) {
        result.error_code = 2;
        result.error_message = strdup(e.what());
    }

    return result;
}

C_BinaryResult qr_generate_png_c(const C_QrConfig *config) {
    C_BinaryResult result = {nullptr, 0, 0, nullptr};
    if (!config || !config->text) {
        result.error_code = 1;
        result.error_message = "Invalid configuration or null text payload";
        return result;
    }

    try {
        QrCode qr = QrCode::encodeText(config->text, parseEccC(config->error_correction_level));
        SvgRenderConfig cfg = buildConfigFromC(config);
        ImageBuffer raster = QrImageProcessor::renderRasterQr(qr, cfg);
        std::vector<uint8_t> pngBytes = QrImageProcessor::encodePng(raster);

        uint8_t *buffer = static_cast<uint8_t*>(std::malloc(pngBytes.size()));
        std::memcpy(buffer, pngBytes.data(), pngBytes.size());

        result.data = buffer;
        result.size = pngBytes.size();
        result.error_code = 0;
    } catch (const std::exception &e) {
        result.error_code = 2;
        result.error_message = strdup(e.what());
    }

    return result;
}

void qr_free_string_c(char *str) {
    if (str) {
        std::free(str);
    }
}

void qr_free_binary_c(uint8_t *buffer) {
    if (buffer) {
        std::free(buffer);
    }
}

}
