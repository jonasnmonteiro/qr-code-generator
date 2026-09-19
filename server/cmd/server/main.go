package main

import (
	"context"
	"log"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/jonasnmonteiro/qr-code-generator/server/internal/cgo"
	"github.com/jonasnmonteiro/qr-code-generator/server/internal/handlers"
	"github.com/jonasnmonteiro/qr-code-generator/server/internal/service"
)

func main() {
	port := os.Getenv("PORT")
	if port == "" {
		port = "8080"
	}

	engine := cgo.NewEngine()
	generatorSvc := service.NewGeneratorService(engine)
	workerPool := service.NewWorkerPool(generatorSvc, 0)
	qrHandler := handlers.NewQRHandler(generatorSvc, workerPool)

	mux := http.NewServeMux()
	mux.HandleFunc("/health", qrHandler.HandleHealth)
	mux.HandleFunc("/api/v1/generate/svg", qrHandler.HandleGenerateSVG)
	mux.HandleFunc("/api/v1/generate/png", qrHandler.HandleGeneratePNG)
	mux.HandleFunc("/api/v1/batch", qrHandler.HandleBatch)

	server := &http.Server{
		Addr:         ":" + port,
		Handler:      mux,
		ReadTimeout:  15 * time.Second,
		WriteTimeout: 30 * time.Second,
		IdleTimeout:  60 * time.Second,
	}

	go func() {
		log.Printf("QR Code Generator API Server running on port %s", port)
		if err := server.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			log.Fatalf("Server error: %v", err)
		}
	}()

	stop := make(chan os.Signal, 1)
	signal.Notify(stop, os.Interrupt, syscall.SIGTERM)
	<-stop

	log.Println("Shutting down server...")
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	if err := server.Shutdown(ctx); err != nil {
		log.Fatalf("Server forced shutdown: %v", err)
	}

	log.Println("Server gracefully stopped")
}
