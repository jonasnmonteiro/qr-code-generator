package handlers

import (
	"encoding/json"
	"net/http"

	"github.com/jonasnmonteiro/qr-code-generator/server/internal/model"
	"github.com/jonasnmonteiro/qr-code-generator/server/internal/service"
)

type QRHandler struct {
	generator  *service.GeneratorService
	workerPool *service.WorkerPool
}

func NewQRHandler(generator *service.GeneratorService, pool *service.WorkerPool) *QRHandler {
	return &QRHandler{
		generator:  generator,
		workerPool: pool,
	}
}

func (h *QRHandler) HandleGenerateSVG(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req model.QRRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON payload: "+err.Error(), http.StatusBadRequest)
		return
	}

	res, err := h.generator.Generate(&req, "svg")
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "image/svg+xml")
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(res.Payload))
}

func (h *QRHandler) HandleGeneratePNG(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var req model.QRRequest
	if err := json.NewDecoder(r.Body).Decode(&req); err != nil {
		http.Error(w, "Invalid JSON payload: "+err.Error(), http.StatusBadRequest)
		return
	}

	pngBytes, err := h.generator.Generate(&req, "png")
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(pngBytes)
}

func (h *QRHandler) HandleBatch(w http.ResponseWriter, r *http.Request) {
	if r.Method != http.MethodPost {
		http.Error(w, "Method not allowed", http.StatusMethodNotAllowed)
		return
	}

	var batch model.BatchRequest
	if err := json.NewDecoder(r.Body).Decode(&batch); err != nil {
		http.Error(w, "Invalid JSON payload: "+err.Error(), http.StatusBadRequest)
		return
	}

	if batch.Format == "" {
		batch.Format = "svg"
	}

	res, err := h.workerPool.ProcessBatch(r.Context(), &batch)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(res)
}

func (h *QRHandler) HandleHealth(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(`{"status":"ok","engine":"cgo_cpp20"}`))
}
