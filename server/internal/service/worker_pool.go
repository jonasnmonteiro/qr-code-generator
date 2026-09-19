package service

import (
	"context"
	"runtime"
	"sync"
	"time"

	"github.com/jonasnmonteiro/qr-code-generator/server/internal/model"
)

type WorkerPool struct {
	generator   *GeneratorService
	concurrency int
}

func NewWorkerPool(generator *GeneratorService, concurrency int) *WorkerPool {
	if concurrency <= 0 {
		concurrency = runtime.NumCPU() * 2
	}
	return &WorkerPool{
		generator:   generator,
		concurrency: concurrency,
	}
}

type job struct {
	index   int
	request model.QRRequest
	format  string
}

type jobResult struct {
	index  int
	result model.QRResult
}

func (wp *WorkerPool) ProcessBatch(ctx context.Context, batch *model.BatchRequest) (*model.BatchResponse, error) {
	startTime := time.Now()
	total := len(batch.Requests)
	results := make([]model.QRResult, total)

	if total == 0 {
		return &model.BatchResponse{
			TotalProcessed: 0,
			DurationMs:     0,
			Results:        results,
		}, nil
	}

	concurrency := wp.concurrency
	if batch.Concurrency > 0 && batch.Concurrency < concurrency {
		concurrency = batch.Concurrency
	}

	jobs := make(chan job, total)
	out := make(chan jobResult, total)

	var wg sync.WaitGroup
	for w := 0; w < concurrency; w++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for {
				select {
				case <-ctx.Done():
					return
				case j, ok := <-jobs:
					if !ok {
						return
					}
					res, err := wp.generator.Generate(&j.request, j.format)
					if err != nil {
						out <- jobResult{
							index: j.index,
							result: model.QRResult{
								ID:     j.request.ID,
								Format: j.format,
								Error:  err.Error(),
							},
						}
					} else {
						out <- jobResult{
							index:  j.index,
							result: *res,
						}
					}
				}
			}
		}()
	}

	for i, req := range batch.Requests {
		jobs <- job{
			index:   i,
			request: req,
			format:  batch.Format,
		}
	}
	close(jobs)

	go func() {
		wg.Wait()
		close(out)
	}()

	for res := range out {
		results[res.index] = res.result
	}

	duration := time.Since(startTime).Milliseconds()

	return &model.BatchResponse{
		TotalProcessed: total,
		DurationMs:     duration,
		Results:        results,
	}, nil
}
