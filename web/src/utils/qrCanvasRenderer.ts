import QRCode from 'qrcode';
import { QRStudioState } from './qrRenderer';

export function renderQrToCanvas(
  canvas: HTMLCanvasElement,
  state: QRStudioState,
  targetSize = 1024
): void {
  const text = state.text?.trim() ? state.text : 'https://github.com/jonasnmonteiro/qr-code-generator';

  let qr: any;
  try {
    qr = QRCode.create(text, { errorCorrectionLevel: state.ecLevel || 'M' });
  } catch (err) {
    qr = QRCode.create(text, { errorCorrectionLevel: 'L' });
  }

  const matrixSize = qr.modules.size;
  canvas.width = targetSize;
  canvas.height = targetSize;
  const ctx = canvas.getContext('2d');
  if (!ctx) return;

  const marginRatio = (state.margin ?? 28) / (state.canvasSize || 450);
  const margin = targetSize * marginRatio;
  const viewport = targetSize - margin * 2;
  const cellSize = viewport / matrixSize;

  // Background
  ctx.fillStyle = state.backgroundColor || '#FFFFFF';
  ctx.fillRect(0, 0, targetSize, targetSize);

  // Gradient or solid module fill
  let moduleFillStyle: string | CanvasGradient = state.moduleColor;
  if (state.gradient.type === 'linear' && state.gradient.colorStart && state.gradient.colorEnd) {
    const angleRad = ((state.gradient.rotationDegrees || 0) - 90) * Math.PI / 180;
    const cx = targetSize / 2;
    const cy = targetSize / 2;
    const r = targetSize / 2;
    const x0 = cx - r * Math.cos(angleRad);
    const y0 = cy - r * Math.sin(angleRad);
    const x1 = cx + r * Math.cos(angleRad);
    const y1 = cy + r * Math.sin(angleRad);
    const grad = ctx.createLinearGradient(x0, y0, x1, y1);
    grad.addColorStop(0, state.gradient.colorStart);
    grad.addColorStop(1, state.gradient.colorEnd);
    moduleFillStyle = grad;
  } else if (state.gradient.type === 'radial' && state.gradient.colorStart && state.gradient.colorEnd) {
    const cx = targetSize / 2;
    const cy = targetSize / 2;
    const grad = ctx.createRadialGradient(cx, cy, 0, cx, cy, targetSize / 2);
    grad.addColorStop(0, state.gradient.colorStart);
    grad.addColorStop(1, state.gradient.colorEnd);
    moduleFillStyle = grad;
  }

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

  // Draw modules
  ctx.fillStyle = moduleFillStyle;
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
        ctx.beginPath();
        ctx.arc(px + cellSize / 2, py + cellSize / 2, r, 0, Math.PI * 2);
        ctx.fill();
      } else if (state.moduleStyle === 'fluid' || state.moduleStyle === 'rounded' || state.moduleStyle === 'extra-rounded') {
        const rad = (state.moduleStyle === 'extra-rounded' || (!north && !south && !east && !west))
          ? (cellSize * 0.46)
          : (cellSize * 0.38);
        const rTL = (!north && !west) ? rad : 0;
        const rTR = (!north && !east) ? rad : 0;
        const rBR = (!south && !east) ? rad : 0;
        const rBL = (!south && !west) ? rad : 0;

        ctx.beginPath();
        ctx.moveTo(px + rTL, py);
        ctx.lineTo(px + cellSize - rTR, py);
        if (rTR > 0) ctx.arcTo(px + cellSize, py, px + cellSize, py + rTR, rTR);
        ctx.lineTo(px + cellSize, py + cellSize - rBR);
        if (rBR > 0) ctx.arcTo(px + cellSize, py + cellSize, px + cellSize - rBR, py + cellSize, rBR);
        ctx.lineTo(px + rBL, py + cellSize);
        if (rBL > 0) ctx.arcTo(px, py + cellSize, px, py + cellSize - rBL, rBL);
        ctx.lineTo(px, py + rTL);
        if (rTL > 0) ctx.arcTo(px, py, px + rTL, py, rTL);
        ctx.closePath();
        ctx.fill();
      } else if (state.moduleStyle === 'classy') {
        const chamfer = cellSize * 0.35;
        ctx.beginPath();
        ctx.moveTo(px + chamfer, py);
        ctx.lineTo(px + cellSize, py);
        ctx.lineTo(px + cellSize, py + cellSize - chamfer);
        ctx.lineTo(px + cellSize - chamfer, py + cellSize);
        ctx.lineTo(px, py + cellSize);
        ctx.lineTo(px, py + chamfer);
        ctx.closePath();
        ctx.fill();
      } else if (state.moduleStyle === 'classy-rounded') {
        const r = cellSize * 0.38;
        ctx.beginPath();
        ctx.moveTo(px + r, py);
        ctx.lineTo(px + cellSize, py);
        ctx.lineTo(px + cellSize, py + cellSize - r);
        ctx.arcTo(px + cellSize, py + cellSize, px + cellSize - r, py + cellSize, r);
        ctx.lineTo(px, py + cellSize);
        ctx.lineTo(px, py + r);
        ctx.arcTo(px, py, px + r, py, r);
        ctx.closePath();
        ctx.fill();
      } else {
        ctx.fillRect(px, py, cellSize, cellSize);
      }
    }
  }

  // Draw Eyes
  const drawEye = (cx: number, cy: number, outerR: number, innerR: number, outColor: string, inColor: string) => {
    const ox = margin + cx * cellSize;
    const oy = margin + cy * cellSize;
    const size = 7 * cellSize;
    const innerSize = 5 * cellSize;
    const pupilSize = 3 * cellSize;

    const rOut = Math.min(outerR * cellSize, size / 2);
    const rInnerCut = Math.max(0, rOut - cellSize);
    const rPupil = Math.min(innerR * cellSize, pupilSize / 2);

    // Outer rect
    ctx.fillStyle = outColor || state.moduleColor;
    drawRoundedRect(ctx, ox, oy, size, size, rOut);

    // Cutout (background)
    ctx.fillStyle = state.backgroundColor || '#FFFFFF';
    drawRoundedRect(ctx, ox + cellSize, oy + cellSize, innerSize, innerSize, rInnerCut);

    // Pupil
    ctx.fillStyle = inColor || state.moduleColor;
    drawRoundedRect(ctx, ox + 2 * cellSize, oy + 2 * cellSize, pupilSize, pupilSize, rPupil);
  };

  drawEye(0, 0, state.eyeTopLeft.outerRadii.topLeft, state.eyeTopLeft.innerRadii.topLeft, state.eyeTopLeft.outerColor, state.eyeTopLeft.innerColor);
  drawEye(matrixSize - 7, 0, state.eyeTopRight.outerRadii.topLeft, state.eyeTopRight.innerRadii.topLeft, state.eyeTopRight.outerColor, state.eyeTopRight.innerColor);
  drawEye(0, matrixSize - 7, state.eyeBottomLeft.outerRadii.topLeft, state.eyeBottomLeft.innerRadii.topLeft, state.eyeBottomLeft.outerColor, state.eyeBottomLeft.innerColor);
}

function drawRoundedRect(
  ctx: CanvasRenderingContext2D,
  x: number,
  y: number,
  w: number,
  h: number,
  r: number
): void {
  ctx.beginPath();
  ctx.moveTo(x + r, y);
  ctx.lineTo(x + w - r, y);
  if (r > 0) ctx.arcTo(x + w, y, x + w, y + r, r);
  ctx.lineTo(x + w, y + h - r);
  if (r > 0) ctx.arcTo(x + w, y + h, x + w - r, y + h, r);
  ctx.lineTo(x + r, y + h);
  if (r > 0) ctx.arcTo(x, y + h, x, y + h - r, r);
  ctx.lineTo(x, y + r);
  if (r > 0) ctx.arcTo(x, y, x + r, y, r);
  ctx.closePath();
  ctx.fill();
}
