#include "QrSvgBuilder.hpp"
#include <cmath>
#include <iomanip>
#include <sstream>

namespace qrcodegen {

static std::string formatNumber(double value) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(3) << value;
    std::string s = ss.str();
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (!s.empty() && s.back() == '.') {
        s.pop_back();
    }
    return s;
}

std::string QrSvgBuilder::buildGradients(const SvgRenderConfig &config, const std::string &idPrefix) {
    std::ostringstream defs;
    bool hasDefs = false;

    if (config.moduleGradient.type != GradientType::NONE && !config.moduleGradient.stops.empty()) {
        hasDefs = true;
        std::string gradId = idPrefix + "_mod_grad";

        if (config.moduleGradient.type == GradientType::LINEAR) {
            double angleRad = (config.moduleGradient.rotationDegrees - 90.0) * M_PI / 180.0;
            double x1 = 50.0 + (50.0 * std::cos(angleRad));
            double y1 = 50.0 + (50.0 * std::sin(angleRad));
            double x2 = 50.0 - (50.0 * std::cos(angleRad));
            double y2 = 50.0 - (50.0 * std::sin(angleRad));

            defs << "    <linearGradient id=\"" << gradId << "\" x1=\"" << formatNumber(x1)
                 << "%\" y1=\"" << formatNumber(y1) << "%\" x2=\"" << formatNumber(x2)
                 << "%\" y2=\"" << formatNumber(y2) << "%\">\n";
        } else {
            defs << "    <radialGradient id=\"" << gradId << "\" cx=\"50%\" cy=\"50%\" r=\"50%\" fx=\"50%\" fy=\"50%\">\n";
        }

        for (const auto &stop : config.moduleGradient.stops) {
            defs << "      <stop offset=\"" << formatNumber(stop.offset * 100.0) << "%\" stop-color=\""
                 << stop.color << "\" stop-opacity=\"" << formatNumber(stop.opacity) << "\" />\n";
        }

        if (config.moduleGradient.type == GradientType::LINEAR) {
            defs << "    </linearGradient>\n";
        } else {
            defs << "    </radialGradient>\n";
        }
    }

    if (config.backgroundGradient.type != GradientType::NONE && !config.backgroundGradient.stops.empty()) {
        hasDefs = true;
        std::string gradId = idPrefix + "_bg_grad";

        if (config.backgroundGradient.type == GradientType::LINEAR) {
            double angleRad = (config.backgroundGradient.rotationDegrees - 90.0) * M_PI / 180.0;
            double x1 = 50.0 + (50.0 * std::cos(angleRad));
            double y1 = 50.0 + (50.0 * std::sin(angleRad));
            double x2 = 50.0 - (50.0 * std::cos(angleRad));
            double y2 = 50.0 - (50.0 * std::sin(angleRad));

            defs << "    <linearGradient id=\"" << gradId << "\" x1=\"" << formatNumber(x1)
                 << "%\" y1=\"" << formatNumber(y1) << "%\" x2=\"" << formatNumber(x2)
                 << "%\" y2=\"" << formatNumber(y2) << "%\">\n";
        } else {
            defs << "    <radialGradient id=\"" << gradId << "\" cx=\"50%\" cy=\"50%\" r=\"50%\" fx=\"50%\" fy=\"50%\">\n";
        }

        for (const auto &stop : config.backgroundGradient.stops) {
            defs << "      <stop offset=\"" << formatNumber(stop.offset * 100.0) << "%\" stop-color=\""
                 << stop.color << "\" stop-opacity=\"" << formatNumber(stop.opacity) << "\" />\n";
        }

        if (config.backgroundGradient.type == GradientType::LINEAR) {
            defs << "    </linearGradient>\n";
        } else {
            defs << "    </radialGradient>\n";
        }
    }

    if (!hasDefs) {
        return "";
    }

    return "  <defs>\n" + defs.str() + "  </defs>\n";
}

std::string QrSvgBuilder::buildBackground(const SvgRenderConfig &config, double canvasSize, const std::string &bgFill) {
    if (bgFill == "transparent" || bgFill.empty()) {
        return "";
    }
    return "  <rect width=\"" + formatNumber(canvasSize) + "\" height=\"" + formatNumber(canvasSize) +
           "\" fill=\"" + bgFill + "\" />\n";
}

std::string QrSvgBuilder::buildLogo(
    const SvgLogoOptions &logo,
    double canvasSize,
    double margin,
    double matrixSize,
    double cellSize
) {
    if (logo.dataUri.empty() || logo.scale <= 0.0) {
        return "";
    }

    double contentSize = canvasSize - (margin * 2.0);
    double logoDim = contentSize * logo.scale;
    double center = canvasSize / 2.0;
    double logoX = center - (logoDim / 2.0);
    double logoY = center - (logoDim / 2.0);

    std::ostringstream ss;

    if (logo.paddingRatio > 0.0) {
        double padding = contentSize * logo.paddingRatio;
        double padDim = logoDim + (padding * 2.0);
        double padX = center - (padDim / 2.0);
        double padY = center - (padDim / 2.0);

        if (logo.shape == LogoShape::CIRCLE) {
            double radius = padDim / 2.0;
            ss << "  <circle cx=\"" << formatNumber(center) << "\" cy=\"" << formatNumber(center)
               << "\" r=\"" << formatNumber(radius) << "\" fill=\"#FFFFFF\" />\n";
        } else {
            double radius = padDim * logo.borderRadiusRatio;
            ss << "  <rect x=\"" << formatNumber(padX) << "\" y=\"" << formatNumber(padY)
               << "\" width=\"" << formatNumber(padDim) << "\" height=\"" << formatNumber(padDim)
               << "\" rx=\"" << formatNumber(radius) << "\" ry=\"" << formatNumber(radius)
               << "\" fill=\"#FFFFFF\" />\n";
        }
    }

    ss << "  <image href=\"" << logo.dataUri << "\" x=\"" << formatNumber(logoX)
       << "\" y=\"" << formatNumber(logoY) << "\" width=\"" << formatNumber(logoDim)
       << "\" height=\"" << formatNumber(logoDim) << "\" preserveAspectRatio=\"xMidYMid meet\" />\n";

    return ss.str();
}

std::string QrSvgBuilder::build(const QrCode &qr, const SvgRenderConfig &config) {
    LogoOptions topoLogoOpts;
    topoLogoOpts.shape = config.logo.shape;
    topoLogoOpts.scale = config.logo.scale;
    topoLogoOpts.paddingRatio = config.logo.paddingRatio;
    topoLogoOpts.borderRadiusRatio = config.logo.borderRadiusRatio;

    QrTopology topology(qr, topoLogoOpts);
    int matrixSize = topology.getSize();

    double rawSize = config.canvasSize;
    double rawMargin = config.margin;
    if (rawMargin * 2.0 >= rawSize) {
        rawMargin = 0.0;
    }

    double viewportSize = rawSize - (rawMargin * 2.0);
    double cellSize = viewportSize / static_cast<double>(matrixSize);
    double offsetX = rawMargin;
    double offsetY = rawMargin;

    std::string idPrefix = "qr_" + std::to_string(matrixSize);
    std::string bgFill = (config.backgroundGradient.type != GradientType::NONE) ?
        "url(#" + idPrefix + "_bg_grad)" : config.backgroundColor;
    std::string moduleFill = (config.moduleGradient.type != GradientType::NONE) ?
        "url(#" + idPrefix + "_mod_grad)" : config.moduleColor;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
        << "viewBox=\"0 0 " << formatNumber(rawSize) << " " << formatNumber(rawSize) << "\" "
        << "width=\"" << formatNumber(rawSize) << "\" height=\"" << formatNumber(rawSize) << "\" "
        << "shape-rendering=\"geometricPrecision\">\n";

    svg << buildGradients(config, idPrefix);
    svg << buildBackground(config, rawSize, bgFill);

    svg << "  <g fill=\"" << moduleFill << "\">\n";
    for (int y = 0; y < matrixSize; y++) {
        for (int x = 0; x < matrixSize; x++) {
            if (topology.shouldRenderDataModule(x, y)) {
                NeighborMask mask = topology.getNeighborMask(x, y);
                std::string moduleSvg = QrShapeRenderer::renderSvgModule(
                    config.moduleStyle,
                    x, y,
                    cellSize,
                    offsetX, offsetY,
                    mask,
                    config.moduleScale
                );
                svg << "    " << moduleSvg << "\n";
            }
        }
    }
    svg << "  </g>\n";

    svg << "  <g id=\"positioning_eyes\">\n";
    svg << "    " << QrEyeRenderer::renderSvgEye(EyePosition::TOP_LEFT, matrixSize, cellSize, offsetX, offsetY, config.eyeTopLeft) << "\n";
    svg << "    " << QrEyeRenderer::renderSvgEye(EyePosition::TOP_RIGHT, matrixSize, cellSize, offsetX, offsetY, config.eyeTopRight) << "\n";
    svg << "    " << QrEyeRenderer::renderSvgEye(EyePosition::BOTTOM_LEFT, matrixSize, cellSize, offsetX, offsetY, config.eyeBottomLeft) << "\n";
    svg << "  </g>\n";

    svg << buildLogo(config.logo, rawSize, rawMargin, matrixSize, cellSize);

    svg << "</svg>";
    return svg.str();
}

}
