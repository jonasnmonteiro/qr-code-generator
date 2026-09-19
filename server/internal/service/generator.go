package service

import (
	"encoding/base64"
	"errors"

	"github.com/jonasnmonteiro/qr-code-generator/server/internal/cgo"
	"github.com/jonasnmonteiro/qr-code-generator/server/internal/model"
)

type GeneratorService struct {
	engine *cgo.Engine
}

func NewGeneratorService(engine *cgo.Engine) *GeneratorService {
	return &GeneratorService{
		engine: engine,
	}
}

func (s *GeneratorService) Generate(req *model.QRRequest, format string) (*model.QRResult, error) {
	if req.Text == "" {
		return nil, errors.New("qr text content cannot be empty")
	}

	result := &model.QRResult{
		ID:     req.ID,
		Format: format,
	}

	if format == "svg" {
		svg, err := s.engine.GenerateSVG(req)
		if err != nil {
			result.Error = err.Error()
			return result, err
		}
		result.Payload = svg
		return result, nil
	}

	pngBytes, err := s.engine.GeneratePNG(req)
	if err != nil {
		result.Error = err.Error()
		return result, err
	}
	result.Payload = base64.StdEncoding.EncodeToString(pngBytes)
	return result, nil
}
