#include "QrShapeRenderer.hpp"
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

std::string QrShapeRenderer::renderSvgModule(
    ModuleStyle style,
    int cellX,
    int cellY,
    double cellSize,
    double offsetX,
    double offsetY,
    const NeighborMask &mask,
    double moduleScale
) {
    double x = offsetX + (cellX * cellSize);
    double y = offsetY + (cellY * cellSize);

    switch (style) {
        case ModuleStyle::DOTS:
            return renderDot(x, y, cellSize, moduleScale);
        case ModuleStyle::ROUNDED:
            return renderRounded(x, y, cellSize, mask, moduleScale);
        case ModuleStyle::EXTRA_ROUNDED:
            return renderExtraRounded(x, y, cellSize, mask, moduleScale);
        case ModuleStyle::CLASSY:
            return renderClassy(x, y, cellSize, mask, moduleScale);
        case ModuleStyle::CLASSY_ROUNDED:
            return renderClassyRounded(x, y, cellSize, mask, moduleScale);
        case ModuleStyle::FLUID:
            return renderFluid(x, y, cellSize, mask);
        case ModuleStyle::SQUARE:
        default:
            return renderSquare(x, y, cellSize, moduleScale);
    }
}

std::string QrShapeRenderer::renderSquare(double x, double y, double size, double scale) {
    if (scale < 0.999) {
        double offset = (1.0 - scale) * 0.5 * size;
        double s = size * scale;
        return "<rect x=\"" + formatNumber(x + offset) + "\" y=\"" + formatNumber(y + offset) +
               "\" width=\"" + formatNumber(s) + "\" height=\"" + formatNumber(s) + "\" />";
    }
    return "<rect x=\"" + formatNumber(x) + "\" y=\"" + formatNumber(y) +
           "\" width=\"" + formatNumber(size) + "\" height=\"" + formatNumber(size) + "\" />";
}

std::string QrShapeRenderer::renderDot(double x, double y, double size, double scale) {
    double r = (size / 2.0) * scale;
    double cx = x + (size / 2.0);
    double cy = y + (size / 2.0);
    return "<circle cx=\"" + formatNumber(cx) + "\" cy=\"" + formatNumber(cy) +
           "\" r=\"" + formatNumber(r) + "\" />";
}

std::string QrShapeRenderer::renderRounded(double x, double y, double size, const NeighborMask &mask, double scale) {
    if (mask.orthogonalCount == 0) {
        return renderDot(x, y, size, scale);
    }
    if (mask.orthogonalCount > 2 || (mask.north && mask.south) || (mask.east && mask.west)) {
        return renderSquare(x, y, size, 1.0);
    }

    double r = size / 2.0;
    double rTL = 0, rTR = 0, rBR = 0, rBL = 0;

    if (mask.orthogonalCount == 1) {
        if (mask.north) {
            rBL = r; rBR = r;
        } else if (mask.south) {
            rTL = r; rTR = r;
        } else if (mask.east) {
            rTL = r; rBL = r;
        } else if (mask.west) {
            rTR = r; rBR = r;
        }
    } else if (mask.orthogonalCount == 2) {
        if (mask.south && mask.east) {
            rTL = r;
        } else if (mask.south && mask.west) {
            rTR = r;
        } else if (mask.north && mask.west) {
            rBR = r;
        } else if (mask.north && mask.east) {
            rBL = r;
        }
    }

    std::ostringstream path;
    path << "<path d=\"";
    path << "M " << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    path << "H " << formatNumber(x + size - rTR) << " ";
    if (rTR > 0) path << "A " << formatNumber(rTR) << " " << formatNumber(rTR) << " 0 0 1 " << formatNumber(x + size) << " " << formatNumber(y + rTR) << " ";
    path << "V " << formatNumber(y + size - rBR) << " ";
    if (rBR > 0) path << "A " << formatNumber(rBR) << " " << formatNumber(rBR) << " 0 0 1 " << formatNumber(x + size - rBR) << " " << formatNumber(y + size) << " ";
    path << "H " << formatNumber(x + rBL) << " ";
    if (rBL > 0) path << "A " << formatNumber(rBL) << " " << formatNumber(rBL) << " 0 0 1 " << formatNumber(x) << " " << formatNumber(y + size - rBL) << " ";
    path << "V " << formatNumber(y + rTL) << " ";
    if (rTL > 0) path << "A " << formatNumber(rTL) << " " << formatNumber(rTL) << " 0 0 1 " << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    path << "Z\" />";

    return path.str();
}

std::string QrShapeRenderer::renderExtraRounded(double x, double y, double size, const NeighborMask &mask, double scale) {
    if (mask.orthogonalCount == 0) {
        return renderDot(x, y, size, scale);
    }
    if (mask.orthogonalCount > 2 || (mask.north && mask.south) || (mask.east && mask.west)) {
        return renderSquare(x, y, size, 1.0);
    }

    double r = size * 0.75;
    double rTL = 0, rTR = 0, rBR = 0, rBL = 0;

    if (mask.orthogonalCount == 1) {
        if (mask.north) {
            rBL = size / 2.0; rBR = size / 2.0;
        } else if (mask.south) {
            rTL = size / 2.0; rTR = size / 2.0;
        } else if (mask.east) {
            rTL = size / 2.0; rBL = size / 2.0;
        } else if (mask.west) {
            rTR = size / 2.0; rBR = size / 2.0;
        }
    } else if (mask.orthogonalCount == 2) {
        if (mask.south && mask.east) {
            rTL = r;
        } else if (mask.south && mask.west) {
            rTR = r;
        } else if (mask.north && mask.west) {
            rBR = r;
        } else if (mask.north && mask.east) {
            rBL = r;
        }
    }

    std::ostringstream path;
    path << "<path d=\"";
    path << "M " << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    path << "H " << formatNumber(x + size - rTR) << " ";
    if (rTR > 0) path << "A " << formatNumber(rTR) << " " << formatNumber(rTR) << " 0 0 1 " << formatNumber(x + size) << " " << formatNumber(y + rTR) << " ";
    path << "V " << formatNumber(y + size - rBR) << " ";
    if (rBR > 0) path << "A " << formatNumber(rBR) << " " << formatNumber(rBR) << " 0 0 1 " << formatNumber(x + size - rBR) << " " << formatNumber(y + size) << " ";
    path << "H " << formatNumber(x + rBL) << " ";
    if (rBL > 0) path << "A " << formatNumber(rBL) << " " << formatNumber(rBL) << " 0 0 1 " << formatNumber(x) << " " << formatNumber(y + size - rBL) << " ";
    path << "V " << formatNumber(y + rTL) << " ";
    if (rTL > 0) path << "A " << formatNumber(rTL) << " " << formatNumber(rTL) << " 0 0 1 " << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    path << "Z\" />";

    return path.str();
}

std::string QrShapeRenderer::renderClassy(double x, double y, double size, const NeighborMask &mask, double scale) {
    if (mask.orthogonalCount == 0) {
        double r = size / 2.0;
        std::ostringstream path;
        path << "<path d=\"M " << formatNumber(x + r) << " " << formatNumber(y)
             << " L " << formatNumber(x + size) << " " << formatNumber(y + r)
             << " L " << formatNumber(x + r) << " " << formatNumber(y + size)
             << " L " << formatNumber(x) << " " << formatNumber(y + r)
             << " Z\" />";
        return path.str();
    }

    double cut = size * 0.4;
    double cTL = (!mask.north && !mask.west) ? cut : 0.0;
    double cBR = (!mask.south && !mask.east) ? cut : 0.0;

    std::ostringstream path;
    path << "<path d=\"";
    path << "M " << formatNumber(x + cTL) << " " << formatNumber(y) << " ";
    path << "L " << formatNumber(x + size) << " " << formatNumber(y) << " ";
    path << "L " << formatNumber(x + size) << " " << formatNumber(y + size - cBR) << " ";
    path << "L " << formatNumber(x + size - cBR) << " " << formatNumber(y + size) << " ";
    path << "L " << formatNumber(x) << " " << formatNumber(y + size) << " ";
    path << "L " << formatNumber(x) << " " << formatNumber(y + cTL) << " ";
    path << "Z\" />";

    return path.str();
}

std::string QrShapeRenderer::renderClassyRounded(double x, double y, double size, const NeighborMask &mask, double scale) {
    if (mask.orthogonalCount == 0) {
        return renderDot(x, y, size, scale);
    }

    double r = size * 0.45;
    double rTL = (!mask.north && !mask.west) ? r : 0.0;
    double rBR = (!mask.south && !mask.east) ? r : 0.0;

    std::ostringstream path;
    path << "<path d=\"";
    path << "M " << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    path << "H " << formatNumber(x + size) << " ";
    path << "V " << formatNumber(y + size - rBR) << " ";
    if (rBR > 0) path << "A " << formatNumber(rBR) << " " << formatNumber(rBR) << " 0 0 1 " << formatNumber(x + size - rBR) << " " << formatNumber(y + size) << " ";
    path << "H " << formatNumber(x) << " ";
    path << "V " << formatNumber(y + rTL) << " ";
    if (rTL > 0) path << "A " << formatNumber(rTL) << " " << formatNumber(rTL) << " 0 0 1 " << formatNumber(x + rTL) << " " << formatNumber(y) << " ";
    path << "Z\" />";

    return path.str();
}

std::string QrShapeRenderer::renderFluid(double x, double y, double size, const NeighborMask &mask) {
    double half = size / 2.0;
    double r = half;

    bool roundTL = !mask.north && !mask.west;
    bool roundTR = !mask.north && !mask.east;
    bool roundBR = !mask.south && !mask.east;
    bool roundBL = !mask.south && !mask.west;

    std::ostringstream path;
    path << "<path d=\"";
    path << "M " << formatNumber(x + (roundTL ? r : 0)) << " " << formatNumber(y) << " ";
    path << "H " << formatNumber(x + size - (roundTR ? r : 0)) << " ";
    if (roundTR) path << "A " << formatNumber(r) << " " << formatNumber(r) << " 0 0 1 " << formatNumber(x + size) << " " << formatNumber(y + r) << " ";
    path << "V " << formatNumber(y + size - (roundBR ? r : 0)) << " ";
    if (roundBR) path << "A " << formatNumber(r) << " " << formatNumber(r) << " 0 0 1 " << formatNumber(x + size - r) << " " << formatNumber(y + size) << " ";
    path << "H " << formatNumber(x + (roundBL ? r : 0)) << " ";
    if (roundBL) path << "A " << formatNumber(r) << " " << formatNumber(r) << " 0 0 1 " << formatNumber(x) << " " << formatNumber(y + size - r) << " ";
    path << "V " << formatNumber(y + (roundTL ? r : 0)) << " ";
    if (roundTL) path << "A " << formatNumber(r) << " " << formatNumber(r) << " 0 0 1 " << formatNumber(x + r) << " " << formatNumber(y) << " ";
    path << "Z\" />";

    return path.str();
}

std::vector<CellPathCommand> QrShapeRenderer::getCellCommands(
    ModuleStyle style,
    double x,
    double y,
    double size,
    const NeighborMask &mask,
    double scale
) {
    std::vector<CellPathCommand> commands;
    double half = size / 2.0;

    if (style == ModuleStyle::DOTS || (mask.orthogonalCount == 0 && style != ModuleStyle::SQUARE)) {
        commands.push_back({CellPathCommand::Type::MOVE_TO, {x + half, y + half}, {}, {}, half * scale});
        return commands;
    }

    commands.push_back({CellPathCommand::Type::MOVE_TO, {x, y}, {}, {}});
    commands.push_back({CellPathCommand::Type::LINE_TO, {x + size, y}, {}, {}});
    commands.push_back({CellPathCommand::Type::LINE_TO, {x + size, y + size}, {}, {}});
    commands.push_back({CellPathCommand::Type::LINE_TO, {x, y + size}, {}, {}});
    commands.push_back({CellPathCommand::Type::CLOSE_PATH, {}, {}, {}});

    return commands;
}

}
