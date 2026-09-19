#pragma once

#include "QrCode.hpp"
#include <cstdint>
#include <vector>

namespace qrcodegen {

enum class LogoShape {
    NONE = 0,
    SQUARE,
    ROUNDED_RECT,
    CIRCLE
};

struct LogoOptions {
    LogoShape shape = LogoShape::NONE;
    double scale = 0.22;
    double paddingRatio = 0.04;
    double borderRadiusRatio = 0.05;
};

struct NeighborMask {
    bool north = false;
    bool south = false;
    bool east = false;
    bool west = false;
    bool northWest = false;
    bool northEast = false;
    bool southWest = false;
    bool southEast = false;
    int orthogonalCount = 0;
};

class QrTopology final {
public:
    QrTopology(const QrCode &qr, const LogoOptions &logoOpts = LogoOptions{});

    int getSize() const;
    bool isDark(int x, int y) const;
    bool isFinderZone(int x, int y) const;
    bool isTimingZone(int x, int y) const;
    bool isAlignmentZone(int x, int y) const;
    bool isLogoOccluded(int x, int y) const;
    bool shouldRenderDataModule(int x, int y) const;

    NeighborMask getNeighborMask(int x, int y) const;

private:
    int size;
    std::vector<std::vector<bool>> darkGrid;
    std::vector<std::vector<bool>> finderGrid;
    std::vector<std::vector<bool>> logoOcclusionGrid;
    std::vector<int> alignmentPositions;

    void initializeGrids(const QrCode &qr, const LogoOptions &logoOpts);
    void calculateAlignmentPositions(int version);
};

}
