#include "QrTopology.hpp"
#include <algorithm>
#include <cmath>

namespace qrcodegen {

QrTopology::QrTopology(const QrCode &qr, const LogoOptions &logoOpts) :
    size(qr.getSize()) {
    initializeGrids(qr, logoOpts);
}

int QrTopology::getSize() const {
    return size;
}

bool QrTopology::isDark(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size) {
        return false;
    }
    return darkGrid[y][x];
}

bool QrTopology::isFinderZone(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size) {
        return false;
    }
    return finderGrid[y][x];
}

bool QrTopology::isTimingZone(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size) {
        return false;
    }
    if (isFinderZone(x, y)) {
        return false;
    }
    return (x == 6 || y == 6);
}

bool QrTopology::isAlignmentZone(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size || isFinderZone(x, y)) {
        return false;
    }
    for (int ay : alignmentPositions) {
        for (int ax : alignmentPositions) {
            if ((ax == 6 && ay == 6) ||
                (ax == 6 && ay == alignmentPositions.back()) ||
                (ax == alignmentPositions.back() && ay == 6)) {
                continue;
            }
            if (std::abs(x - ax) <= 2 && std::abs(y - ay) <= 2) {
                return true;
            }
        }
    }
    return false;
}

bool QrTopology::isLogoOccluded(int x, int y) const {
    if (x < 0 || x >= size || y < 0 || y >= size) {
        return false;
    }
    return logoOcclusionGrid[y][x];
}

bool QrTopology::shouldRenderDataModule(int x, int y) const {
    if (!isDark(x, y)) {
        return false;
    }
    if (isFinderZone(x, y)) {
        return false;
    }
    if (isLogoOccluded(x, y)) {
        return false;
    }
    return true;
}

NeighborMask QrTopology::getNeighborMask(int x, int y) const {
    NeighborMask mask;
    mask.north = shouldRenderDataModule(x, y - 1);
    mask.south = shouldRenderDataModule(x, y + 1);
    mask.east  = shouldRenderDataModule(x + 1, y);
    mask.west  = shouldRenderDataModule(x - 1, y);

    mask.northWest = shouldRenderDataModule(x - 1, y - 1);
    mask.northEast = shouldRenderDataModule(x + 1, y - 1);
    mask.southWest = shouldRenderDataModule(x - 1, y + 1);
    mask.southEast = shouldRenderDataModule(x + 1, y + 1);

    mask.orthogonalCount = (mask.north ? 1 : 0) +
                           (mask.south ? 1 : 0) +
                           (mask.east  ? 1 : 0) +
                           (mask.west  ? 1 : 0);
    return mask;
}

void QrTopology::calculateAlignmentPositions(int version) {
    alignmentPositions.clear();
    if (version <= 1) {
        return;
    }
    int numAlign = version / 7 + 1;
    int step = (version == 32) ? 26 : (version * 4 + numAlign * 2 + 1) / (numAlign * 2 - 2) * 2;
    alignmentPositions.push_back(6);
    for (int pos = size - 7; alignmentPositions.size() < static_cast<std::size_t>(numAlign); pos -= step) {
        alignmentPositions.insert(alignmentPositions.begin() + 1, pos);
    }
}

void QrTopology::initializeGrids(const QrCode &qr, const LogoOptions &logoOpts) {
    darkGrid = std::vector<std::vector<bool>>(size, std::vector<bool>(size, false));
    finderGrid = std::vector<std::vector<bool>>(size, std::vector<bool>(size, false));
    logoOcclusionGrid = std::vector<std::vector<bool>>(size, std::vector<bool>(size, false));

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            darkGrid[y][x] = qr.getModule(x, y);
        }
    }

    auto markFinder = [this](int startX, int startY) {
        for (int dy = 0; dy < 8; dy++) {
            for (int dx = 0; dx < 8; dx++) {
                int px = startX + dx;
                int py = startY + dy;
                if (px >= 0 && px < size && py >= 0 && py < size) {
                    finderGrid[py][px] = true;
                }
            }
        }
    };

    markFinder(0, 0);
    markFinder(size - 8, 0);
    markFinder(0, size - 8);

    calculateAlignmentPositions(qr.getVersion());

    if (logoOpts.shape != LogoShape::NONE && logoOpts.scale > 0.0) {
        double totalScale = std::clamp(logoOpts.scale + (logoOpts.paddingRatio * 2.0), 0.05, 0.45);
        double logoCells = size * totalScale;
        double center = size / 2.0;
        double halfSize = logoCells / 2.0;

        double x0 = center - halfSize;
        double x1 = center + halfSize;
        double y0 = center - halfSize;
        double y1 = center + halfSize;

        if (logoOpts.shape == LogoShape::CIRCLE) {
            double radiusSq = halfSize * halfSize;
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    double cx = (x + 0.5) - center;
                    double cy = (y + 0.5) - center;
                    if ((cx * cx + cy * cy) <= radiusSq) {
                        logoOcclusionGrid[y][x] = true;
                    }
                }
            }
        } else {
            for (int y = 0; y < size; y++) {
                for (int x = 0; x < size; x++) {
                    double cellMinX = x;
                    double cellMaxX = x + 1.0;
                    double cellMinY = y;
                    double cellMaxY = y + 1.0;

                    if (cellMaxX > x0 && cellMinX < x1 && cellMaxY > y0 && cellMinY < y1) {
                        logoOcclusionGrid[y][x] = true;
                    }
                }
            }
        }
    }
}

}
