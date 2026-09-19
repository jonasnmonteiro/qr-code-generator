import React, { useState, useMemo, useRef } from 'react';
import {
  Sparkles,
  Download,
  Copy,
  Check,
  Upload,
  Layers,
  Palette,
  Eye,
  Sliders,
  Maximize,
  RefreshCw
} from 'lucide-react';
import { buildClientSvg, QRStudioState, ModuleStyle } from '../utils/qrRenderer';

const PRESETS: Array<{ name: string; state: Partial<QRStudioState> }> = [
  {
    name: 'Cyberpunk Neon',
    state: {
      backgroundColor: '#090D16',
      moduleStyle: 'fluid',
      gradient: {
        type: 'linear',
        rotationDegrees: 45,
        colorStart: '#EC4899',
        colorEnd: '#06B6D4',
      },
      eyeTopLeft: {
        outerRadii: { topLeft: 3.5, topRight: 3.5, bottomRight: 3.5, bottomLeft: 3.5 },
        innerRadii: { topLeft: 1.5, topRight: 1.5, bottomRight: 1.5, bottomLeft: 1.5 },
        outerColor: '#EC4899',
        innerColor: '#06B6D4',
      },
      eyeTopRight: {
        outerRadii: { topLeft: 3.5, topRight: 3.5, bottomRight: 3.5, bottomLeft: 3.5 },
        innerRadii: { topLeft: 1.5, topRight: 1.5, bottomRight: 1.5, bottomLeft: 1.5 },
        outerColor: '#EC4899',
        innerColor: '#06B6D4',
      },
      eyeBottomLeft: {
        outerRadii: { topLeft: 3.5, topRight: 3.5, bottomRight: 3.5, bottomLeft: 3.5 },
        innerRadii: { topLeft: 1.5, topRight: 1.5, bottomRight: 1.5, bottomLeft: 1.5 },
        outerColor: '#EC4899',
        innerColor: '#06B6D4',
      },
    },
  },
  {
    name: 'Emerald Luxe',
    state: {
      backgroundColor: '#061A14',
      moduleStyle: 'rounded',
      gradient: {
        type: 'linear',
        rotationDegrees: 135,
        colorStart: '#10B981',
        colorEnd: '#34D399',
      },
      eyeTopLeft: {
        outerRadii: { topLeft: 3.5, topRight: 0, bottomRight: 3.5, bottomLeft: 0 },
        innerRadii: { topLeft: 1.5, topRight: 0, bottomRight: 1.5, bottomLeft: 0 },
        outerColor: '#10B981',
        innerColor: '#34D399',
      },
      eyeTopRight: {
        outerRadii: { topLeft: 0, topRight: 3.5, bottomRight: 0, bottomLeft: 3.5 },
        innerRadii: { topLeft: 0, topRight: 1.5, bottomRight: 0, bottomLeft: 1.5 },
        outerColor: '#10B981',
        innerColor: '#34D399',
      },
      eyeBottomLeft: {
        outerRadii: { topLeft: 0, topRight: 3.5, bottomRight: 0, bottomLeft: 3.5 },
        innerRadii: { topLeft: 0, topRight: 1.5, bottomRight: 0, bottomLeft: 1.5 },
        outerColor: '#10B981',
        innerColor: '#34D399',
      },
    },
  },
  {
    name: 'Sunset Minimal',
    state: {
      backgroundColor: '#FFFFFF',
      moduleStyle: 'classy',
      gradient: {
        type: 'linear',
        rotationDegrees: 45,
        colorStart: '#F97316',
        colorEnd: '#DC2626',
      },
      eyeTopLeft: {
        outerRadii: { topLeft: 2.0, topRight: 2.0, bottomRight: 2.0, bottomLeft: 2.0 },
        innerRadii: { topLeft: 1.0, topRight: 1.0, bottomRight: 1.0, bottomLeft: 1.0 },
        outerColor: '#DC2626',
        innerColor: '#F97316',
      },
      eyeTopRight: {
        outerRadii: { topLeft: 2.0, topRight: 2.0, bottomRight: 2.0, bottomLeft: 2.0 },
        innerRadii: { topLeft: 1.0, topRight: 1.0, bottomRight: 1.0, bottomLeft: 1.0 },
        outerColor: '#DC2626',
        innerColor: '#F97316',
      },
      eyeBottomLeft: {
        outerRadii: { topLeft: 2.0, topRight: 2.0, bottomRight: 2.0, bottomLeft: 2.0 },
        innerRadii: { topLeft: 1.0, topRight: 1.0, bottomRight: 1.0, bottomLeft: 1.0 },
        outerColor: '#DC2626',
        innerColor: '#F97316',
      },
    },
  },
];

export default function QRStudio() {
  const [state, setState] = useState<QRStudioState>({
    text: 'https://github.com/jonasnmonteiro/qr-code-generator',
    ecLevel: 'M',
    canvasSize: 450,
    margin: 24,
    backgroundColor: '#090D16',
    moduleColor: '#6366F1',
    moduleStyle: 'fluid',
    moduleScale: 1.0,
    gradient: {
      type: 'linear',
      rotationDegrees: 45,
      colorStart: '#6366F1',
      colorEnd: '#06B6D4',
    },
    eyeTopLeft: {
      outerRadii: { topLeft: 3.5, topRight: 3.5, bottomRight: 3.5, bottomLeft: 3.5 },
      innerRadii: { topLeft: 1.5, topRight: 1.5, bottomRight: 1.5, bottomLeft: 1.5 },
      outerColor: '#6366F1',
      innerColor: '#06B6D4',
    },
    eyeTopRight: {
      outerRadii: { topLeft: 3.5, topRight: 3.5, bottomRight: 3.5, bottomLeft: 3.5 },
      innerRadii: { topLeft: 1.5, topRight: 1.5, bottomRight: 1.5, bottomLeft: 1.5 },
      outerColor: '#6366F1',
      innerColor: '#06B6D4',
    },
    eyeBottomLeft: {
      outerRadii: { topLeft: 3.5, topRight: 3.5, bottomRight: 3.5, bottomLeft: 3.5 },
      innerRadii: { topLeft: 1.5, topRight: 1.5, bottomRight: 1.5, bottomLeft: 1.5 },
      outerColor: '#6366F1',
      innerColor: '#06B6D4',
    },
    logo: {
      shape: 'none',
      scale: 0.22,
      paddingRatio: 0.04,
      borderRadiusRatio: 0.08,
      dataUri: '',
    },
  });

  const [activeTab, setActiveTab] = useState<'content' | 'style' | 'eyes' | 'colors' | 'logo'>('content');
  const [copied, setCopied] = useState(false);
  const fileInputRef = useRef<HTMLInputElement>(null);

  const svgOutput = useMemo(() => buildClientSvg(state), [state]);

  const handleLogoUpload = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;
    const reader = new FileReader();
    reader.onload = (event) => {
      const dataUri = event.target?.result as string;
      setState((prev) => ({
        ...prev,
        logo: { ...prev.logo, shape: 'circle', dataUri },
      }));
    };
    reader.readAsDataURL(file);
  };

  const downloadSvg = () => {
    const blob = new Blob([svgOutput], { type: 'image/svg+xml;charset=utf-8' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'qrcode.svg';
    link.click();
    URL.revokeObjectURL(url);
  };

  const downloadPng = (scaleMultiplier = 2) => {
    const canvas = document.createElement('canvas');
    const size = state.canvasSize * scaleMultiplier;
    canvas.width = size;
    canvas.height = size;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const img = new Image();
    const svgBlob = new Blob([svgOutput], { type: 'image/svg+xml;charset=utf-8' });
    const url = URL.createObjectURL(svgBlob);

    img.onload = () => {
      ctx.drawImage(img, 0, 0, size, size);
      const pngUrl = canvas.toDataURL('image/png');
      const link = document.createElement('a');
      link.href = pngUrl;
      link.download = `qrcode_${size}x${size}.png`;
      link.click();
      URL.revokeObjectURL(url);
    };
    img.src = url;
  };

  const copySvg = () => {
    navigator.clipboard.writeText(svgOutput);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  const applyPreset = (presetState: Partial<QRStudioState>) => {
    setState((prev) => ({
      ...prev,
      ...presetState,
    }));
  };

  return (
    <div style={{ maxWidth: '1280px', margin: '0 auto', padding: '32px 16px' }}>
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(340px, 1fr))', gap: '32px', alignItems: 'start' }}>
        
        {/* Controls Column */}
        <div className="glass-panel" style={{ padding: '24px' }}>
          <div style={{ display: 'flex', gap: '8px', borderBottom: '1px solid var(--border-glass)', paddingBottom: '16px', marginBottom: '24px', overflowX: 'auto' }}>
            <button className={activeTab === 'content' ? 'btn-primary' : 'btn-secondary'} onClick={() => setActiveTab('content')}>
              <Sliders size={16} /> Content
            </button>
            <button className={activeTab === 'style' ? 'btn-primary' : 'btn-secondary'} onClick={() => setActiveTab('style')}>
              <Layers size={16} /> Shapes
            </button>
            <button className={activeTab === 'colors' ? 'btn-primary' : 'btn-secondary'} onClick={() => setActiveTab('colors')}>
              <Palette size={16} /> Colors
            </button>
            <button className={activeTab === 'eyes' ? 'btn-primary' : 'btn-secondary'} onClick={() => setActiveTab('eyes')}>
              <Eye size={16} /> Eyes
            </button>
            <button className={activeTab === 'logo' ? 'btn-primary' : 'btn-secondary'} onClick={() => setActiveTab('logo')}>
              <Upload size={16} /> Logo
            </button>
          </div>

          {activeTab === 'content' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '18px' }}>
              <div>
                <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                  QR Code Text / URL
                </label>
                <input
                  type="text"
                  className="input-field"
                  value={state.text}
                  onChange={(e) => setState({ ...state, text: e.target.value })}
                  placeholder="https://example.com or any text"
                />
              </div>

              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '12px' }}>
                <div>
                  <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                    Error Correction
                  </label>
                  <select
                    className="input-field"
                    value={state.ecLevel}
                    onChange={(e) => setState({ ...state, ecLevel: e.target.value as any })}
                  >
                    <option value="L">L (7% Recovery)</option>
                    <option value="M">M (15% Recovery)</option>
                    <option value="Q">Q (25% Recovery)</option>
                    <option value="H">H (30% Recovery)</option>
                  </select>
                </div>
                <div>
                  <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                    Quiet Zone Margin: {state.margin}px
                  </label>
                  <input
                    type="range"
                    min="0"
                    max="60"
                    className="slider-custom"
                    value={state.margin}
                    onChange={(e) => setState({ ...state, margin: Number(e.target.value) })}
                  />
                </div>
              </div>

              <div>
                <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '10px' }}>
                  Quick Presets
                </label>
                <div style={{ display: 'flex', gap: '8px', flexWrap: 'wrap' }}>
                  {PRESETS.map((p) => (
                    <button key={p.name} className="btn-secondary" style={{ fontSize: '13px', padding: '6px 14px' }} onClick={() => applyPreset(p.state)}>
                      <Sparkles size={14} /> {p.name}
                    </button>
                  ))}
                </div>
              </div>
            </div>
          )}

          {activeTab === 'style' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
              <label style={{ fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)' }}>
                Module Geometry Style
              </label>
              <div style={{ display: 'grid', gridTemplateColumns: 'repeat(2, 1fr)', gap: '10px' }}>
                {(['square', 'dots', 'rounded', 'extra-rounded', 'classy', 'classy-rounded', 'fluid'] as ModuleStyle[]).map((style) => (
                  <button
                    key={style}
                    className={state.moduleStyle === style ? 'btn-primary' : 'btn-secondary'}
                    style={{ textTransform: 'capitalize', justifyContent: 'center' }}
                    onClick={() => setState({ ...state, moduleStyle: style })}
                  >
                    {style.replace('-', ' ')}
                  </button>
                ))}
              </div>
            </div>
          )}

          {activeTab === 'colors' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '18px' }}>
              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '12px' }}>
                <div>
                  <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                    Background
                  </label>
                  <input
                    type="color"
                    style={{ width: '100%', height: '40px', background: 'none', border: 'none', cursor: 'pointer' }}
                    value={state.backgroundColor}
                    onChange={(e) => setState({ ...state, backgroundColor: e.target.value })}
                  />
                </div>
                <div>
                  <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                    Gradient Type
                  </label>
                  <select
                    className="input-field"
                    value={state.gradient.type}
                    onChange={(e) => setState({ ...state, gradient: { ...state.gradient, type: e.target.value as any } })}
                  >
                    <option value="none">Solid Color</option>
                    <option value="linear">Linear Gradient</option>
                  </select>
                </div>
              </div>

              {state.gradient.type !== 'none' ? (
                <>
                  <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '12px' }}>
                    <div>
                      <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                        Start Color
                      </label>
                      <input
                        type="color"
                        style={{ width: '100%', height: '40px', background: 'none', border: 'none', cursor: 'pointer' }}
                        value={state.gradient.colorStart}
                        onChange={(e) => setState({ ...state, gradient: { ...state.gradient, colorStart: e.target.value } })}
                      />
                    </div>
                    <div>
                      <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                        End Color
                      </label>
                      <input
                        type="color"
                        style={{ width: '100%', height: '40px', background: 'none', border: 'none', cursor: 'pointer' }}
                        value={state.gradient.colorEnd}
                        onChange={(e) => setState({ ...state, gradient: { ...state.gradient, colorEnd: e.target.value } })}
                      />
                    </div>
                  </div>
                  <div>
                    <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                      Rotation: {state.gradient.rotationDegrees}°
                    </label>
                    <input
                      type="range"
                      min="0"
                      max="360"
                      className="slider-custom"
                      value={state.gradient.rotationDegrees}
                      onChange={(e) => setState({ ...state, gradient: { ...state.gradient, rotationDegrees: Number(e.target.value) } })}
                    />
                  </div>
                </>
              ) : (
                <div>
                  <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                    Module Color
                  </label>
                  <input
                    type="color"
                    style={{ width: '100%', height: '40px', background: 'none', border: 'none', cursor: 'pointer' }}
                    value={state.moduleColor}
                    onChange={(e) => setState({ ...state, moduleColor: e.target.value })}
                  />
                </div>
              )}
            </div>
          )}

          {activeTab === 'eyes' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '18px' }}>
              <div>
                <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                  Outer Corner Radius: {state.eyeTopLeft.outerRadii.topLeft}
                </label>
                <input
                  type="range"
                  min="0"
                  max="3.5"
                  step="0.5"
                  className="slider-custom"
                  value={state.eyeTopLeft.outerRadii.topLeft}
                  onChange={(e) => {
                    const r = Number(e.target.value);
                    const eyeConfig = {
                      ...state.eyeTopLeft,
                      outerRadii: { topLeft: r, topRight: r, bottomRight: r, bottomLeft: r },
                    };
                    setState({
                      ...state,
                      eyeTopLeft: eyeConfig,
                      eyeTopRight: eyeConfig,
                      eyeBottomLeft: eyeConfig,
                    });
                  }}
                />
              </div>
              <div>
                <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                  Inner Nucleus Radius: {state.eyeTopLeft.innerRadii.topLeft}
                </label>
                <input
                  type="range"
                  min="0"
                  max="1.5"
                  step="0.5"
                  className="slider-custom"
                  value={state.eyeTopLeft.innerRadii.topLeft}
                  onChange={(e) => {
                    const r = Number(e.target.value);
                    const eyeConfig = {
                      ...state.eyeTopLeft,
                      innerRadii: { topLeft: r, topRight: r, bottomRight: r, bottomLeft: r },
                    };
                    setState({
                      ...state,
                      eyeTopLeft: eyeConfig,
                      eyeTopRight: eyeConfig,
                      eyeBottomLeft: eyeConfig,
                    });
                  }}
                />
              </div>
            </div>
          )}

          {activeTab === 'logo' && (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
              <div>
                <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                  Upload Center Logo
                </label>
                <input
                  type="file"
                  ref={fileInputRef}
                  accept="image/*"
                  style={{ display: 'none' }}
                  onChange={handleLogoUpload}
                />
                <button className="btn-secondary" style={{ width: '100%', justifyContent: 'center' }} onClick={() => fileInputRef.current?.click()}>
                  <Upload size={16} /> Choose Image File
                </button>
              </div>

              {state.logo.dataUri && (
                <>
                  <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '12px' }}>
                    <div>
                      <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                        Mask Shape
                      </label>
                      <select
                        className="input-field"
                        value={state.logo.shape}
                        onChange={(e) => setState({ ...state, logo: { ...state.logo, shape: e.target.value as any } })}
                      >
                        <option value="none">No Padding Mask</option>
                        <option value="circle">Circular Mask</option>
                        <option value="square">Square Mask</option>
                      </select>
                    </div>
                    <div>
                      <label style={{ display: 'block', fontSize: '13px', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '6px' }}>
                        Logo Scale: {(state.logo.scale * 100).toFixed(0)}%
                      </label>
                      <input
                        type="range"
                        min="0.1"
                        max="0.35"
                        step="0.02"
                        className="slider-custom"
                        value={state.logo.scale}
                        onChange={(e) => setState({ ...state, logo: { ...state.logo, scale: Number(e.target.value) } })}
                      />
                    </div>
                  </div>
                  <button className="btn-secondary" style={{ color: '#EF4444' }} onClick={() => setState({ ...state, logo: { ...state.logo, dataUri: '', shape: 'none' } })}>
                    Remove Logo
                  </button>
                </>
              )}
            </div>
          )}
        </div>

        {/* Live Preview Column */}
        <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '20px' }}>
          <div
            className="glass-panel"
            style={{
              padding: '24px',
              display: 'flex',
              justifyContent: 'center',
              alignItems: 'center',
              boxShadow: '0 20px 50px rgba(0,0,0,0.5)',
            }}
          >
            <div
              style={{ width: '360px', height: '360px', display: 'flex', justifyContent: 'center', alignItems: 'center' }}
              dangerouslySetInnerHTML={{ __html: svgOutput }}
            />
          </div>

          <div style={{ display: 'flex', gap: '12px', flexWrap: 'wrap', justifyContent: 'center' }}>
            <button className="btn-primary" onClick={downloadSvg}>
              <Download size={16} /> Export SVG
            </button>
            <button className="btn-secondary" onClick={() => downloadPng(2)}>
              <Download size={16} /> Export PNG (2x)
            </button>
            <button className="btn-secondary" onClick={copySvg}>
              {copied ? <Check size={16} color="#10B981" /> : <Copy size={16} />} {copied ? 'Copied!' : 'Copy SVG'}
            </button>
          </div>
        </div>

      </div>
    </div>
  );
}
