package model

type ErrorCorrectionLevel string

const (
	EccLow      ErrorCorrectionLevel = "L"
	EccMedium   ErrorCorrectionLevel = "M"
	EccQuartile ErrorCorrectionLevel = "Q"
	EccHigh     ErrorCorrectionLevel = "H"
)

type ModuleStyle string

const (
	StyleSquare        ModuleStyle = "square"
	StyleDots          ModuleStyle = "dots"
	StyleRounded       ModuleStyle = "rounded"
	StyleExtraRounded  ModuleStyle = "extra-rounded"
	StyleClassy        ModuleStyle = "classy"
	StyleClassyRounded ModuleStyle = "classy-rounded"
	StyleFluid         ModuleStyle = "fluid"
)

type LogoShape string

const (
	LogoShapeNone        LogoShape = "none"
	LogoShapeSquare      LogoShape = "square"
	LogoShapeRoundedRect LogoShape = "rounded-rect"
	LogoShapeCircle      LogoShape = "circle"
)

type GradientType string

const (
	GradientNone   GradientType = "none"
	GradientLinear GradientType = "linear"
	GradientRadial GradientType = "radial"
)

type EyeOptions struct {
	OuterRadius float64 `json:"outer_radius"`
	InnerRadius float64 `json:"inner_radius"`
	OuterColor  string  `json:"outer_color"`
	InnerColor  string  `json:"inner_color"`
}

type LogoOptions struct {
	Shape        LogoShape `json:"shape"`
	Scale        float64   `json:"scale"`
	PaddingRatio float64   `json:"padding_ratio"`
	DataURI      string    `json:"data_uri"`
}

type GradientOptions struct {
	Type            GradientType `json:"type"`
	RotationDegrees float64      `json:"rotation_degrees"`
	ColorStart      string       `json:"color_start"`
	ColorEnd        string       `json:"color_end"`
}

type QRRequest struct {
	ID              string               `json:"id,omitempty"`
	Text            string               `json:"text"`
	ECLevel         ErrorCorrectionLevel `json:"ec_level"`
	CanvasSize      float64              `json:"canvas_size"`
	Margin          float64              `json:"margin"`
	BackgroundColor string               `json:"background_color"`
	ModuleColor     string               `json:"module_color"`
	ModuleStyle     ModuleStyle          `json:"module_style"`
	ModuleScale     float64              `json:"module_scale"`
	EyeOptions      *EyeOptions          `json:"eye_options,omitempty"`
	Logo            *LogoOptions         `json:"logo,omitempty"`
	Gradient        *GradientOptions     `json:"gradient,omitempty"`
}

type QRResult struct {
	ID      string `json:"id,omitempty"`
	Format  string `json:"format"`
	Payload string `json:"payload,omitempty"`
	Error   string `json:"error,omitempty"`
}

type BatchRequest struct {
	Requests    []QRRequest `json:"requests"`
	Format      string      `json:"format"`
	Concurrency int         `json:"concurrency,omitempty"`
}

type BatchResponse struct {
	TotalProcessed int        `json:"total_processed"`
	DurationMs     int64      `json:"duration_ms"`
	Results        []QRResult `json:"results"`
}
