export type ErrorCorrectionLevel = 'L' | 'M' | 'Q' | 'H';

export type ModuleStyle =
    | 'square'
    | 'dots'
    | 'rounded'
    | 'extra-rounded'
    | 'classy'
    | 'classy-rounded'
    | 'fluid';

export type LogoShape = 'none' | 'square' | 'rounded-rect' | 'circle';

export type GradientType = 'none' | 'linear' | 'radial';

export interface ColorStop {
    offset: number;
    color: string;
    opacity?: number;
}

export interface GradientOptions {
    type: GradientType;
    rotationDegrees?: number;
    colorStart?: string;
    colorEnd?: string;
}

export interface EyeOptions {
    outerRadius?: number;
    innerRadius?: number;
    outerColor?: string;
    innerColor?: string;
}

export interface LogoOptions {
    shape?: LogoShape;
    scale?: number;
    padding?: number;
    dataUri?: string;
}

export interface QrOptions {
    text: string;
    ecLevel?: ErrorCorrectionLevel;
    size?: number;
    margin?: number;
    backgroundColor?: string;
    moduleColor?: string;
    moduleStyle?: ModuleStyle;
    moduleScale?: number;
    gradient?: GradientOptions;
    eyeTopLeft?: EyeOptions;
    eyeTopRight?: EyeOptions;
    eyeBottomLeft?: EyeOptions;
    logo?: LogoOptions;
}
