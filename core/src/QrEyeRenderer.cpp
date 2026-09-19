#include "QrEyeRenderer.hpp"
#include <algorithm>
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

std::string QrEyeRenderer::renderRoundedRectPath(
    double x,
    double y,
    double width,
    double height,
    const CornerRadii &radii
) {
    double maxRadius = std::min(width, height) / 2.0;
    double rTL = std::clamp(radii.topLeft, 0.0, maxRadius);
    double rTR = std::clamp(radii.topRight, 0.0, maxRadius);
    double rBR = std::clamp(radii.bottomRight, 0.0, maxRadius);
    double rBL = std::clamp(radii.bottomLeft, 0.0, maxRadius);

    std::ostringstream path;
    path << "M " << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    path << "H " << formatNumber(x + width - rTR) << " ";
    if (rTR > 0.0) {
        path << "A " << formatNumber(rTR) << " " << formatNumber(rTR) << " 0 0 1 "
             << formatNumber(x + width) << " " << formatNumber(y + rTR) << " ";
    }
    path << "V " << formatNumber(y + height - rBR) << " ";
    if (rBR > 0.0) {
        path << "A " << formatNumber(rBR) << " " << formatNumber(rBR) << " 0 0 1 "
             << formatNumber(x + width - rBR) << " " << formatNumber(y + height) << " ";
    }
    path << "H " << formatNumber(x + rBL) << " ";
    if (rBL > 0.0) {
        path << "A " << formatNumber(rBL) << " " << formatNumber(rBL) << " 0 0 1 "
             << formatNumber(x) << " " << formatNumber(y + height - rBL) << " ";
    }
    path << "V " << formatNumber(y + rTL) << " ";
    if (rTL > 0.0) {
        path << "A " << formatNumber(rTL) << " " << formatNumber(rTL) << " 0 0 1 "
             << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    }
    path << "Z";

    return path.str();
}

std::string QrEyeRenderer::renderHollowFramePath(
    double x,
    double y,
    double outerSize,
    double borderWidth,
    const CornerRadii &outerRadii,
    const CornerRadii &innerRadii
) {
    double innerX = x + borderWidth;
    double innerY = y + borderWidth;
    double innerSize = outerSize - (borderWidth * 2.0);

    std::string outerPath = renderRoundedRectPath(x, y, outerSize, outerSize, outerRadii);
    std::string innerPath = renderRoundedRectPath(innerX, innerY, innerSize, innerSize, innerRadii);

    return outerPath + " " + innerPath;
}

std::string QrEyeRenderer::renderSvgEye(
    EyePosition position,
    int matrixSize,
    double cellSize,
    double offsetX,
    double offsetY,
    const EyeStyle &style
) {
    double eyeCellX = 0.0;
    double eyeCellY = 0.0;

    switch (position) {
        case EyePosition::TOP_LEFT:
            eyeCellX = 0.0;
            eyeCellY = 0.0;
            break;
        case EyePosition::TOP_RIGHT:
            eyeCellX = matrixSize - 7.0;
            eyeCellY = 0.0;
            break;
        case EyePosition::BOTTOM_LEFT:
            eyeCellX = 0.0;
            eyeCellY = matrixSize - 7.0;
            break;
    }

    double originX = offsetX + (eyeCellX * cellSize);
    double originY = offsetY + (eyeCellY * cellSize);
    double frameSize = 7.0 * cellSize;
    double borderWidth = cellSize;

    CornerRadii outerR = {
        style.outerRadii.topLeft * cellSize,
        style.outerRadii.topRight * cellSize,
        style.outerRadii.bottomRight * cellSize,
        style.outerRadii.bottomLeft * cellSize
    };

    CornerRadii innerHoleR = {
        std::max(0.0, outerR.topLeft - borderWidth),
        std::max(0.0, outerR.topRight - borderWidth),
        std::max(0.0, outerR.bottomRight - borderWidth),
        std::max(0.0, outerR.bottomLeft - borderWidth)
    };

    CornerRadii centerNucleusR = {
        style.innerRadii.topLeft * cellSize,
        style.innerRadii.topRight * cellSize,
        style.innerRadii.bottomRight * cellSize,
        style.innerRadii.bottomLeft * cellSize
    };

    std::ostringstream svg;

    std::string framePathData = renderHollowFramePath(originX, originY, frameSize, borderWidth, outerR, innerHoleR);
    svg << "<path d=\"" << framePathData << "\" fill=\"" << style.outerColor << "\" fill-rule=\"evenodd\" />\n";

    double nucleusX = originX + (2.0 * cellSize);
    double nucleusY = originY + (2.0 * cellSize);
    double nucleusSize = 3.0 * cellSize;

    std::string nucleusPathData = renderRoundedRectPath(nucleusX, nucleusY, nucleusSize, nucleusSize, centerNucleusR);
    svg << "<path d=\"" << nucleusPathData << "\" fill=\"" << style.innerColor << "\" />";

    return svg.str();
}

}
