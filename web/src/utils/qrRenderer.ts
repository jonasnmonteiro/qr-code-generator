export type ModuleStyle = 'square' | 'dots' | 'rounded' | 'extra-rounded' | 'classy' | 'classy-rounded' | 'fluid';

export interface ColorStop {
  offset: number;
  color: string;
  opacity?: number;
}

export interface GradientOptions {
  type: 'none' | 'linear' | 'radial';
  rotationDegrees: number;
  colorStart: string;
  colorEnd: string;
}

export interface EyeCornerRadii {
  topLeft: number;
  topRight: number;
  bottomRight: number;
  bottomLeft: number;
}

export interface EyeConfig {
  outerRadii: EyeCornerRadii;
  innerRadii: EyeCornerRadii;
  outerColor: string;
  innerColor: string;
}

export interface QRStudioState {
  text: string;
  ecLevel: 'L' | 'M' | 'Q' | 'H';
  canvasSize: number;
  margin: number;
  backgroundColor: string;
  moduleColor: string;
  moduleStyle: ModuleStyle;
  moduleScale: number;
  gradient: GradientOptions;
  eyeTopLeft: EyeConfig;
  eyeTopRight: EyeConfig;
  eyeBottomLeft: EyeConfig;
  logo: {
    shape: 'none' | 'square' | 'rounded-rect' | 'circle';
    scale: number;
    paddingRatio: number;
    borderRadiusRatio: number;
    dataUri: string;
  };
}

function makeBitMatrix(text: string): boolean[][] {
  const hash = Array.from(text).reduce((acc, char) => (acc * 31 + char.charCodeAt(0)) % 1000000007, 42);
  const size = 29;
  const grid: boolean[][] = Array.from({ length: size }, () => Array(size).fill(false));

  for (let y = 0; y < size; y++) {
    for (let x = 0; x < size; x++) {
      const isFinder = (x < 8 && (y < 8 || y >= size - 8)) || (x >= size - 8 && y < 8);
      const isTiming = (x === 6 || y === 6);
      if (isFinder || isTiming) continue;

      const pseudoRandom = Math.sin(x * 12.9898 + y * 78.233 + hash) * 43758.5453;
      grid[y][x] = (pseudoRandom - Math.floor(pseudoRandom)) > 0.48;
    }
  }

  const markFinder = (sx: number, sy: number) => {
    for (let dy = 0; dy < 7; dy++) {
      for (let dx = 0; dx < 7; dx++) {
        const dist = Math.max(Math.abs(dx - 3), Math.abs(dy - 3));
        grid[sy + dy][sx + dx] = (dist !== 2);
      }
    }
  };

  markFinder(0, 0);
  markFinder(size - 7, 0);
  markFinder(0, size - 7);

  for (let i = 8; i < size - 8; i++) {
    grid[6][i] = i % 2 === 0;
    grid[i][6] = i % 2 === 0;
  }

  return grid;
}

export function buildClientSvg(state: QRStudioState): string {
  const matrix = makeBitMatrix(state.text || 'https://github.com/jonasnmonteiro/qr-code-generator');
  const matrixSize = matrix.length;
  const rawSize = state.canvasSize || 400;
  const margin = state.margin ?? 20;
  const viewport = rawSize - margin * 2;
  const cellSize = viewport / matrixSize;

  const isFinder = (x: number, y: number) =>
    (x < 8 && y < 8) || (x >= matrixSize - 8 && y < 8) || (x < 8 && y >= matrixSize - 8);

  const isOccluded = (x: number, y: number) => {
    if (state.logo.shape === 'none' || state.logo.scale <= 0) return false;
    const totalScale = state.logo.scale + state.logo.paddingRatio * 2;
    const half = (matrixSize * totalScale) / 2;
    const center = matrixSize / 2;
    if (state.logo.shape === 'circle') {
      const dx = x + 0.5 - center;
      const dy = y + 0.5 - center;
      return (dx * dx + dy * dy) <= (half * half);
    }
    return x >= center - half && x <= center + half && y >= center - half && y <= center + half;
  };

  const isDark = (x: number, y: number) => {
    if (x < 0 || x >= matrixSize || y < 0 || y >= matrixSize) return false;
    if (isFinder(x, y) || isOccluded(x, y)) return false;
    return matrix[y][x];
  };

  let defs = '';
  let moduleFill = state.moduleColor;
  let bgFill = state.backgroundColor;

  if (state.gradient.type !== 'none' && state.gradient.colorStart && state.gradient.colorEnd) {
    const angleRad = (state.gradient.rotationDegrees - 90) * Math.PI / 180;
    const x1 = 50 + 50 * Math.cos(angleRad);
    const y1 = 50 + 50 * Math.sin(angleRad);
    const x2 = 50 - 50 * Math.cos(angleRad);
    const y2 = 50 - 50 * Math.sin(angleRad);

    defs += `
      <defs>
        <linearGradient id="mod_grad" x1="${x1.toFixed(1)}%" y1="${y1.toFixed(1)}%" x2="${x2.toFixed(1)}%" y2="${y2.toFixed(1)}%">
          <stop offset="0%" stop-color="${state.gradient.colorStart}" />
          <stop offset="100%" stop-color="${state.gradient.colorEnd}" />
        </linearGradient>
      </defs>`;
    moduleFill = 'url(#mod_grad)';
  }

  let paths = '';
  for (let y = 0; y < matrixSize; y++) {
    for (let x = 0; x < matrixSize; x++) {
      if (!isDark(x, y)) continue;

      const px = margin + x * cellSize;
      const py = margin + y * cellSize;
      const north = isDark(x, y - 1);
      const south = isDark(x, y + 1);
      const east = isDark(x + 1, y);
      const west = isDark(x - 1, y);
      const deg = (north ? 1 : 0) + (south ? 1 : 0) + (east ? 1 : 0) + (west ? 1 : 0);

      if (state.moduleStyle === 'dots' || (deg === 0 && state.moduleStyle !== 'square')) {
        const r = (cellSize / 2) * state.moduleScale;
        paths += `<circle cx="${(px + cellSize / 2).toFixed(2)}" cy="${(py + cellSize / 2).toFixed(2)}" r="${r.toFixed(2)}" />\n`;
      } else if (state.moduleStyle === 'fluid' || state.moduleStyle === 'rounded' || state.moduleStyle === 'extra-rounded') {
        const r = (cellSize / 2).toFixed(2);
        const rTL = (!north && !west) ? r : '0';
        const rTR = (!north && !east) ? r : '0';
        const rBR = (!south && !east) ? r : '0';
        const rBL = (!south && !west) ? r : '0';

        paths += `<path d="M ${(px + +rTL).toFixed(2)} ${py.toFixed(2)} ` +
          `H ${(px + cellSize - +rTR).toFixed(2)} ` +
          (+rTR > 0 ? `A ${rTR} ${rTR} 0 0 1 ${(px + cellSize).toFixed(2)} ${(py + +rTR).toFixed(2)} ` : '') +
          `V ${(py + cellSize - +rBR).toFixed(2)} ` +
          (+rBR > 0 ? `A ${rBR} ${rBR} 0 0 1 ${(px + cellSize - +rBR).toFixed(2)} ${(py + cellSize).toFixed(2)} ` : '') +
          `H ${(px + +rBL).toFixed(2)} ` +
          (+rBL > 0 ? `A ${rBL} ${rBL} 0 0 1 ${px.toFixed(2)} ${(py + cellSize - +rBL).toFixed(2)} ` : '') +
          `V ${(py + +rTL).toFixed(2)} ` +
          (+rTL > 0 ? `A ${rTL} ${rTL} 0 0 1 ${(px + +rTL).toFixed(2)} ${py.toFixed(2)} ` : '') +
          `Z" />\n`;
      } else {
        paths += `<rect x="${px.toFixed(2)}" y="${py.toFixed(2)}" width="${cellSize.toFixed(2)}" height="${cellSize.toFixed(2)}" />\n`;
      }
    }
  }

  const renderEyeSvg = (cx: number, cy: number, eye: EyeConfig) => {
    const ox = margin + cx * cellSize;
    const oy = margin + cy * cellSize;
    const size = 7 * cellSize;
    const rOut = (eye.outerRadii.topLeft * cellSize).toFixed(2);
    const rIn = (eye.innerRadii.topLeft * cellSize).toFixed(2);

    return `
      <rect x="${ox.toFixed(2)}" y="${oy.toFixed(2)}" width="${size.toFixed(2)}" height="${size.toFixed(2)}" rx="${rOut}" ry="${rOut}" fill="${eye.outerColor || state.moduleColor}" />
      <rect x="${(ox + cellSize).toFixed(2)}" y="${(oy + cellSize).toFixed(2)}" width="${(5 * cellSize).toFixed(2)}" height="${(5 * cellSize).toFixed(2)}" rx="${rOut}" ry="${rOut}" fill="${state.backgroundColor}" />
      <rect x="${(ox + 2 * cellSize).toFixed(2)}" y="${(oy + 2 * cellSize).toFixed(2)}" width="${(3 * cellSize).toFixed(2)}" height="${(3 * cellSize).toFixed(2)}" rx="${rIn}" ry="${rIn}" fill="${eye.innerColor || state.moduleColor}" />
    `;
  };

  let logoSvg = '';
  if (state.logo.shape !== 'none' && state.logo.scale > 0 && state.logo.dataUri) {
    const content = rawSize - margin * 2;
    const logoSize = content * state.logo.scale;
    const center = rawSize / 2;
    const logoX = center - logoSize / 2;
    const logoY = center - logoSize / 2;

    if (state.logo.paddingRatio > 0) {
      const pad = content * state.logo.paddingRatio;
      const padSize = logoSize + pad * 2;
      const padX = center - padSize / 2;
      const padY = center - padSize / 2;
      if (state.logo.shape === 'circle') {
        logoSvg += `<circle cx="${center.toFixed(2)}" cy="${center.toFixed(2)}" r="${(padSize / 2).toFixed(2)}" fill="#FFFFFF" />\n`;
      } else {
        const rx = (padSize * state.logo.borderRadiusRatio).toFixed(2);
        logoSvg += `<rect x="${padX.toFixed(2)}" y="${padY.toFixed(2)}" width="${padSize.toFixed(2)}" height="${padSize.toFixed(2)}" rx="${rx}" fill="#FFFFFF" />\n`;
      }
    }

    logoSvg += `<image href="${state.logo.dataUri}" x="${logoX.toFixed(2)}" y="${logoY.toFixed(2)}" width="${logoSize.toFixed(2)}" height="${logoSize.toFixed(2)}" preserveAspectRatio="xMidYMid meet" />\n`;
  }

  return `<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 ${rawSize} ${rawSize}" width="${rawSize}" height="${rawSize}" shape-rendering="geometricPrecision">
    ${defs}
    <rect width="${rawSize}" height="${rawSize}" fill="${bgFill}" />
    <g fill="${moduleFill}">
      ${paths}
    </g>
    <g id="eyes">
      ${renderEyeSvg(0, 0, state.eyeTopLeft)}
      ${renderEyeSvg(matrixSize - 7, 0, state.eyeTopRight)}
      ${renderEyeSvg(0, matrixSize - 7, state.eyeBottomLeft)}
    </g>
    ${logoSvg}
  </svg>`;
}
