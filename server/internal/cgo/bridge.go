package cgo

/*
#cgo CFLAGS: -I${SRCDIR}/../../../core/include
#cgo CXXFLAGS: -std=c++20 -I${SRCDIR}/../../../core/include
#include "qr_engine_c_api.h"
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"unsafe"

	"github.com/jonasnmonteiro/qr-code-generator/server/internal/model"
)

type Engine struct{}

func NewEngine() *Engine {
	return &Engine{}
}

func parseEcc(ecl model.ErrorCorrectionLevel) int {
	switch ecl {
	case model.EccLow:
		return 0
	case model.EccMedium:
		return 1
	case model.EccQuartile:
		return 2
	case model.EccHigh:
		return 3
	default:
		return 1
	}
}

func parseStyle(s model.ModuleStyle) int {
	switch s {
	case model.StyleSquare:
		return 0
	case model.StyleDots:
		return 1
	case model.StyleRounded:
		return 2
	case model.StyleExtraRounded:
		return 3
	case model.StyleClassy:
		return 4
	case model.StyleClassyRounded:
		return 5
	case model.StyleFluid:
		return 6
	default:
		return 0
	}
}

func parseLogoShape(s model.LogoShape) int {
	switch s {
	case model.LogoShapeNone:
		return 0
	case model.LogoShapeSquare:
		return 1
	case model.LogoShapeRoundedRect:
		return 2
	case model.LogoShapeCircle:
		return 3
	default:
		return 0
	}
}

func parseGradientType(g model.GradientType) int {
	switch g {
	case model.GradientNone:
		return 0
	case model.GradientLinear:
		return 1
	case model.GradientRadial:
		return 2
	default:
		return 0
	}
}

func (e *Engine) toCConfig(req *model.QRRequest) (C.C_QrConfig, func()) {
	var c C.C_QrConfig

	cText := C.CString(req.Text)
	cBg := C.CString(req.BackgroundColor)
	cMod := C.CString(req.ModuleColor)

	var cLogoData *C.char
	if req.Logo != nil && req.Logo.DataURI != "" {
		cLogoData = C.CString(req.Logo.DataURI)
	}

	var cGradStart, cGradEnd *C.char
	if req.Gradient != nil {
		if req.Gradient.ColorStart != "" {
			cGradStart = C.CString(req.Gradient.ColorStart)
		}
		if req.Gradient.ColorEnd != "" {
			cGradEnd = C.CString(req.Gradient.ColorEnd)
		}
	}

	var cEyeOuter, cEyeInner *C.char
	if req.EyeOptions != nil {
		if req.EyeOptions.OuterColor != "" {
			cEyeOuter = C.CString(req.EyeOptions.OuterColor)
		}
		if req.EyeOptions.InnerColor != "" {
			cEyeInner = C.CString(req.EyeOptions.InnerColor)
		}
	}

	c.text = cText
	c.error_correction_level = C.int(parseEcc(req.ECLevel))
	c.canvas_size = C.double(req.CanvasSize)
	c.margin = C.double(req.Margin)
	c.background_color = cBg
	c.module_color = cMod
	c.module_style = C.int(parseStyle(req.ModuleStyle))
	c.module_scale = C.double(req.ModuleScale)

	if req.Logo != nil {
		c.logo_shape = C.int(parseLogoShape(req.Logo.Shape))
		c.logo_scale = C.double(req.Logo.Scale)
		c.logo_padding_ratio = C.double(req.Logo.PaddingRatio)
		c.logo_data_uri = cLogoData
	}

	if req.Gradient != nil {
		c.gradient_type = C.int(parseGradientType(req.Gradient.Type))
		c.gradient_rotation = C.double(req.Gradient.RotationDegrees)
		c.gradient_color_start = cGradStart
		c.gradient_color_end = cGradEnd
	}

	if req.EyeOptions != nil {
		c.eye_radius_outer = C.double(req.EyeOptions.OuterRadius)
		c.eye_radius_inner = C.double(req.EyeOptions.InnerRadius)
		c.eye_color_outer = cEyeOuter
		c.eye_color_inner = cEyeInner
	}

	cleanup := func() {
		C.free(unsafe.Pointer(cText))
		C.free(unsafe.Pointer(cBg))
		C.free(unsafe.Pointer(cMod))
		if cLogoData != nil {
			C.free(unsafe.Pointer(cLogoData))
		}
		if cGradStart != nil {
			C.free(unsafe.Pointer(cGradStart))
		}
		if cGradEnd != nil {
			C.free(unsafe.Pointer(cGradEnd))
		}
		if cEyeOuter != nil {
			C.free(unsafe.Pointer(cEyeOuter))
		}
		if cEyeInner != nil {
			C.free(unsafe.Pointer(cEyeInner))
		}
	}

	return c, cleanup
}

func (e *Engine) GenerateSVG(req *model.QRRequest) (string, error) {
	cConfig, cleanup := e.toCConfig(req)
	defer cleanup()

	result := C.qr_generate_svg_c(&cConfig)
	if result.error_code != 0 {
		errMsg := C.GoString(result.error_message)
		return "", errors.New(errMsg)
	}
	defer C.qr_free_string_c(result.svg_string)

	return C.GoString(result.svg_string), nil
}

func (e *Engine) GeneratePNG(req *model.QRRequest) ([]byte, error) {
	cConfig, cleanup := e.toCConfig(req)
	defer cleanup()

	result := C.qr_generate_png_c(&cConfig)
	if result.error_code != 0 {
		errMsg := C.GoString(result.error_message)
		return nil, errors.New(errMsg)
	}
	defer C.qr_free_binary_c(result.data)

	data := C.GoBytes(unsafe.Pointer(result.data), C.int(result.size))
	return data, nil
}
