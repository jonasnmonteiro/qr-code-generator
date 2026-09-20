import QRCode from 'qrcode';

export type ModuleStyle = 'square' | 'dots' | 'rounded' | 'extra-rounded' | 'classy' | 'classy-rounded' | 'fluid';

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

export function buildClientSvg(state: QRStudioState): string {
  const text = state.text?.trim() ? state.text : 'https://github.com/jonasnmonteiro/qr-code-generator';
  
  let qr: any;
  try {
    qr = QRCode.create(text, { errorCorrectionLevel: state.ecLevel || 'M' });
  } catch (err) {
    qr = QRCode.create(text, { errorCorrectionLevel: 'L' });
  }

  const matrixSize = qr.modules.size;
  const rawSize = state.canvasSize || 450;
  const margin = state.margin ?? 28;
  const viewport = rawSize - margin * 2;
  const cellSize = viewport / matrixSize;

  const isFinder = (x: number, y: number) =>
    (x < 8 && y < 8) || (x >= matrixSize - 8 && y < 8) || (x < 8 && y >= matrixSize - 8);

  const isOccluded = (x: number, y: number) => {
    if (state.logo.shape === 'none' || state.logo.scale <= 0 || !state.logo.dataUri) return false;
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
    return qr.modules.get(x, y) === 1;
  };

  let defs = '';
  let moduleFill = state.moduleColor;
  const bgFill = state.backgroundColor || '#FFFFFF';

  if (state.gradient.type === 'linear' && state.gradient.colorStart && state.gradient.colorEnd) {
    const angleRad = ((state.gradient.rotationDegrees || 0) - 90) * Math.PI / 180;
    const x1 = (50 + 50 * Math.cos(angleRad)).toFixed(1);
    const y1 = (50 + 50 * Math.sin(angleRad)).toFixed(1);
    const x2 = (50 - 50 * Math.cos(angleRad)).toFixed(1);
    const y2 = (50 - 50 * Math.sin(angleRad)).toFixed(1);

    defs += `
      <defs>
        <linearGradient id="qr_grad" x1="${x1}%" y1="${y1}%" x2="${x2}%" y2="${y2}%">
          <stop offset="0%" stop-color="${state.gradient.colorStart}" />
          <stop offset="100%" stop-color="${state.gradient.colorEnd}" />
        </linearGradient>
      </defs>`;
    moduleFill = 'url(#qr_grad)';
  } else if (state.gradient.type === 'radial' && state.gradient.colorStart && state.gradient.colorEnd) {
    defs += `
      <defs>
        <radialGradient id="qr_grad" cx="50%" cy="50%" r="50%">
          <stop offset="0%" stop-color="${state.gradient.colorStart}" />
          <stop offset="100%" stop-color="${state.gradient.colorEnd}" />
        </radialGradient>
      </defs>`;
    moduleFill = 'url(#qr_grad)';
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

      if (state.moduleStyle === 'dots') {
        const r = (cellSize / 2) * 0.94;
        paths += `<circle cx="${(px + cellSize / 2).toFixed(2)}" cy="${(py + cellSize / 2).toFixed(2)}" r="${r.toFixed(2)}" />\n`;
      } else if (state.moduleStyle === 'fluid' || state.moduleStyle === 'rounded' || state.moduleStyle === 'extra-rounded') {
        const radiusVal = (state.moduleStyle === 'extra-rounded' || (!north && !south && !east && !west))
          ? (cellSize * 0.46)
          : (cellSize * 0.38);
        const rTL = (!north && !west) ? radiusVal : 0;
        const rTR = (!north && !east) ? radiusVal : 0;
        const rBR = (!south && !east) ? radiusVal : 0;
        const rBL = (!south && !west) ? radiusVal : 0;

        paths += `<path d="M ${(px + rTL).toFixed(2)} ${py.toFixed(2)} ` +
          `H ${(px + cellSize - rTR).toFixed(2)} ` +
          (rTR > 0 ? `A ${rTR.toFixed(2)} ${rTR.toFixed(2)} 0 0 1 ${(px + cellSize).toFixed(2)} ${(py + rTR).toFixed(2)} ` : '') +
          `V ${(py + cellSize - rBR).toFixed(2)} ` +
          (rBR > 0 ? `A ${rBR.toFixed(2)} ${rBR.toFixed(2)} 0 0 1 ${(px + cellSize - rBR).toFixed(2)} ${(py + cellSize).toFixed(2)} ` : '') +
          `H ${(px + rBL).toFixed(2)} ` +
          (rBL > 0 ? `A ${rBL.toFixed(2)} ${rBL.toFixed(2)} 0 0 1 ${px.toFixed(2)} ${(py + cellSize - rBL).toFixed(2)} ` : '') +
          `V ${(py + rTL).toFixed(2)} ` +
          (rTL > 0 ? `A ${rTL.toFixed(2)} ${rTL.toFixed(2)} 0 0 1 ${(px + rTL).toFixed(2)} ${py.toFixed(2)} ` : '') +
          `Z" />\n`;
      } else if (state.moduleStyle === 'classy') {
        const chamfer = (cellSize * 0.35).toFixed(2);
        paths += `<path d="M ${(px + +chamfer).toFixed(2)} ${py.toFixed(2)} ` +
          `H ${(px + cellSize).toFixed(2)} ` +
          `V ${(py + cellSize - +chamfer).toFixed(2)} ` +
          `L ${(px + cellSize - +chamfer).toFixed(2)} ${(py + cellSize).toFixed(2)} ` +
          `H ${px.toFixed(2)} ` +
          `V ${(py + +chamfer).toFixed(2)} ` +
          `Z" />\n`;
      } else if (state.moduleStyle === 'classy-rounded') {
        const r = (cellSize * 0.38).toFixed(2);
        paths += `<path d="M ${(px + +r).toFixed(2)} ${py.toFixed(2)} ` +
          `H ${(px + cellSize).toFixed(2)} ` +
          `V ${(py + cellSize - +r).toFixed(2)} ` +
          `A ${r} ${r} 0 0 1 ${(px + cellSize - +r).toFixed(2)} ${(py + cellSize).toFixed(2)} ` +
          `H ${px.toFixed(2)} ` +
          `V ${(py + +r).toFixed(2)} ` +
          `A ${r} ${r} 0 0 1 ${(px + +r).toFixed(2)} ${py.toFixed(2)} ` +
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
    const rOut = Math.min(eye.outerRadii.topLeft * cellSize, size / 2).toFixed(2);
    const innerSize = 5 * cellSize;
    const rInnerCut = Math.max(0, +rOut - cellSize).toFixed(2);
    const pupilSize = 3 * cellSize;
    const rPupil = Math.min(eye.innerRadii.topLeft * cellSize, pupilSize / 2).toFixed(2);

    const outColor = eye.outerColor || state.moduleColor;
    const inColor = eye.innerColor || state.moduleColor;

    return `
      <g>
        <rect x="${ox.toFixed(2)}" y="${oy.toFixed(2)}" width="${size.toFixed(2)}" height="${size.toFixed(2)}" rx="${rOut}" ry="${rOut}" fill="${outColor}" />
        <rect x="${(ox + cellSize).toFixed(2)}" y="${(oy + cellSize).toFixed(2)}" width="${innerSize.toFixed(2)}" height="${innerSize.toFixed(2)}" rx="${rInnerCut}" ry="${rInnerCut}" fill="${bgFill}" />
        <rect x="${(ox + 2 * cellSize).toFixed(2)}" y="${(oy + 2 * cellSize).toFixed(2)}" width="${pupilSize.toFixed(2)}" height="${pupilSize.toFixed(2)}" rx="${rPupil}" ry="${rPupil}" fill="${inColor}" />
      </g>
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
        logoSvg += `<circle cx="${center.toFixed(2)}" cy="${center.toFixed(2)}" r="${(padSize / 2).toFixed(2)}" fill="${bgFill}" />\n`;
      } else {
        const rx = (padSize * state.logo.borderRadiusRatio).toFixed(2);
        logoSvg += `<rect x="${padX.toFixed(2)}" y="${padY.toFixed(2)}" width="${padSize.toFixed(2)}" height="${padSize.toFixed(2)}" rx="${rx}" fill="${bgFill}" />\n`;
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
