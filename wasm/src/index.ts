import { QrOptions, ErrorCorrectionLevel, ModuleStyle, LogoShape, GradientType } from './types';

export * from './types';

const ECC_MAP: Record<ErrorCorrectionLevel, number> = {
    L: 0,
    M: 1,
    Q: 2,
    H: 3
};

const STYLE_MAP: Record<ModuleStyle, number> = {
    'square': 0,
    'dots': 1,
    'rounded': 2,
    'extra-rounded': 3,
    'classy': 4,
    'classy-rounded': 5,
    'fluid': 6
};

const LOGO_SHAPE_MAP: Record<LogoShape, number> = {
    'none': 0,
    'square': 1,
    'rounded-rect': 2,
    'circle': 3
};

const GRADIENT_MAP: Record<GradientType, number> = {
    'none': 0,
    'linear': 1,
    'radial': 2
};

export interface WasmModuleInstance {
    WasmQrEngine: {
        generateSvg(
            text: string,
            ecl: number,
            size: number,
            margin: number,
            bgColor: string,
            moduleColor: string,
            moduleStyle: number,
            moduleScale: number,
            logoShape: number,
            logoScale: number,
            logoPadding: number,
            logoDataUri: string,
            gradType: number,
            gradRotation: number,
            gradColorStart: string,
            gradColorEnd: string,
            eyeRadiusOuter: number,
            eyeRadiusInner: number,
            eyeColorOuter: string,
            eyeColorInner: string
        ): string;
        generatePng(
            text: string,
            ecl: number,
            size: number,
            margin: number,
            bgColor: string,
            moduleColor: string,
            moduleStyle: number,
            eyeRadiusOuter: number,
            eyeRadiusInner: number,
            eyeColorOuter: string,
            eyeColorInner: string
        ): Uint8Array;
        calculateOptimalModuleColor(bgHex: string): string;
    };
}

let wasmInstancePromise: Promise<WasmModuleInstance> | null = null;

export async function initQrEngine(wasmFactory?: () => Promise<WasmModuleInstance>): Promise<WasmModuleInstance> {
    if (!wasmInstancePromise) {
        if (wasmFactory) {
            wasmInstancePromise = wasmFactory();
        } else if (typeof window !== 'undefined' && (window as any).createQrEngine) {
            wasmInstancePromise = (window as any).createQrEngine();
        } else {
            throw new Error('WebAssembly initialization factory not provided.');
        }
    }
    return wasmInstancePromise;
}

export async function generateQrSvg(options: QrOptions, wasmModule?: WasmModuleInstance): Promise<string> {
    const wasm = wasmModule || (await initQrEngine());

    const ecl = ECC_MAP[options.ecLevel || 'M'];
    const style = STYLE_MAP[options.moduleStyle || 'square'];
    const logoShape = LOGO_SHAPE_MAP[options.logo?.shape || 'none'];
    const gradType = GRADIENT_MAP[options.gradient?.type || 'none'];

    return wasm.WasmQrEngine.generateSvg(
        options.text,
        ecl,
        options.size || 400,
        options.margin ?? 20,
        options.backgroundColor || '#FFFFFF',
        options.moduleColor || '#000000',
        style,
        options.moduleScale ?? 1.0,
        logoShape,
        options.logo?.scale ?? 0.22,
        options.logo?.padding ?? 0.04,
        options.logo?.dataUri || '',
        gradType,
        options.gradient?.rotationDegrees ?? 0,
        options.gradient?.colorStart || '',
        options.gradient?.colorEnd || '',
        options.eyeTopLeft?.outerRadius ?? 0.0,
        options.eyeTopLeft?.innerRadius ?? 0.0,
        options.eyeTopLeft?.outerColor || options.moduleColor || '#000000',
        options.eyeTopLeft?.innerColor || options.moduleColor || '#000000'
    );
}

export async function generateQrPng(options: QrOptions, wasmModule?: WasmModuleInstance): Promise<Uint8Array> {
    const wasm = wasmModule || (await initQrEngine());

    const ecl = ECC_MAP[options.ecLevel || 'M'];
    const style = STYLE_MAP[options.moduleStyle || 'square'];

    return wasm.WasmQrEngine.generatePng(
        options.text,
        ecl,
        options.size || 400,
        options.margin ?? 20,
        options.backgroundColor || '#FFFFFF',
        options.moduleColor || '#000000',
        style,
        options.eyeTopLeft?.outerRadius ?? 0.0,
        options.eyeTopLeft?.innerRadius ?? 0.0,
        options.eyeTopLeft?.outerColor || options.moduleColor || '#000000',
        options.eyeTopLeft?.innerColor || options.moduleColor || '#000000'
    );
}
