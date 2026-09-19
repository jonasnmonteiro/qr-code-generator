#pragma once

#include "QrTopology.hpp"
#include <string>
#include <vector>

namespace qrcodegen {

enum class ModuleStyle {
    SQUARE = 0,
    DOTS,
    ROUNDED,
    EXTRA_ROUNDED,
    CLASSY,
    CLASSY_ROUNDED,
    FLUID
};

struct Point2D {
    double x;
    double y;
};

struct CellPathCommand {
    enum class Type {
        MOVE_TO,
        LINE_TO,
        ARC_TO,
        BEZIER_CURVE_TO,
        CLOSE_PATH
    };

    Type type;
    Point2D p1;
    Point2D p2;
    Point2D p3;
    double radius = 0.0;
};

class QrShapeRenderer final {
public:
    static std::string renderSvgModule(
        ModuleStyle style,
        int cellX,
        int cellY,
        double cellSize,
        double offsetX,
        double offsetY,
        const NeighborMask &mask,
        double moduleScale = 1.0
    );

    static std::vector<CellPathCommand> getCellCommands(
        ModuleStyle style,
        double x,
        double y,
        double size,
        const NeighborMask &mask,
        double scale = 1.0
    );

private:
    static std::string renderSquare(double x, double y, double size, double scale);
    static std::string renderDot(double x, double y, double size, double scale);
    static std::string renderRounded(double x, double y, double size, const NeighborMask &mask, double scale);
    static std::string renderExtraRounded(double x, double y, double size, const NeighborMask &mask, double scale);
    static std::string renderClassy(double x, double y, double size, const NeighborMask &mask, double scale);
    static std::string renderClassyRounded(double x, double y, double size, const NeighborMask &mask, double scale);
    static std::string renderFluid(double x, double y, double size, const NeighborMask &mask);
};

}
