#pragma once

#include <string>
#include <vector>

namespace qrcodegen {

struct CornerRadii {
    double topLeft = 0.0;
    double topRight = 0.0;
    double bottomRight = 0.0;
    double bottomLeft = 0.0;

    static CornerRadii all(double r) {
        return {r, r, r, r};
    }
};

struct EyeStyle {
    CornerRadii outerRadii;
    CornerRadii innerRadii;
    std::string outerColor = "#000000";
    std::string innerColor = "#000000";
};

enum class EyePosition {
    TOP_LEFT = 0,
    TOP_RIGHT,
    BOTTOM_LEFT
};

class QrEyeRenderer final {
public:
    static std::string renderSvgEye(
        EyePosition position,
        int matrixSize,
        double cellSize,
        double offsetX,
        double offsetY,
        const EyeStyle &style
    );

    static std::string renderRoundedRectPath(
        double x,
        double y,
        double width,
        double height,
        const CornerRadii &radii
    );

    static std::string renderHollowFramePath(
        double x,
        double y,
        double outerSize,
        double borderWidth,
        const CornerRadii &outerRadii,
        const CornerRadii &innerRadii
    );
};

}
