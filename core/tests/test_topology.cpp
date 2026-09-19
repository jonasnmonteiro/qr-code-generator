#include "QrCode.hpp"
#include "QrTopology.hpp"
#include "QrShapeRenderer.hpp"
#include <cassert>
#include <iostream>
#include <string>

using namespace qrcodegen;

int main() {
    QrCode qr = QrCode::encodeText("https://github.com/jonasnmonteiro/qr-code-generator", QrCode::Ecc::MEDIUM);
    assert(qr.getSize() > 0);

    LogoOptions logoOpts;
    logoOpts.shape = LogoShape::CIRCLE;
    logoOpts.scale = 0.2;
    logoOpts.paddingRatio = 0.05;

    QrTopology topology(qr, logoOpts);
    assert(topology.getSize() == qr.getSize());

    int size = topology.getSize();
    assert(topology.isFinderZone(0, 0) == true);
    assert(topology.isFinderZone(6, 6) == true);
    assert(topology.isFinderZone(size - 1, 0) == true);
    assert(topology.isFinderZone(0, size - 1) == true);

    int center = size / 2;
    assert(topology.isLogoOccluded(center, center) == true);

    int nonOccludedDataModules = 0;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (topology.shouldRenderDataModule(x, y)) {
                nonOccludedDataModules++;
                NeighborMask mask = topology.getNeighborMask(x, y);

                for (auto style : {
                    ModuleStyle::SQUARE,
                    ModuleStyle::DOTS,
                    ModuleStyle::ROUNDED,
                    ModuleStyle::EXTRA_ROUNDED,
                    ModuleStyle::CLASSY,
                    ModuleStyle::CLASSY_ROUNDED,
                    ModuleStyle::FLUID
                }) {
                    std::string svgPath = QrShapeRenderer::renderSvgModule(
                        style, x, y, 10.0, 20.0, 20.0, mask, 1.0
                    );
                    assert(!svgPath.empty());
                }
            }
        }
    }

    assert(nonOccludedDataModules > 0);
    std::cout << "All Block 2 topology and shape renderer assertions passed successfully!" << std::endl;
    return 0;
}
