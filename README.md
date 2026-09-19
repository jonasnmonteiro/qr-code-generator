# QR Code Generator Suite

A high-performance, modular, and modern QR Code generator built on a unified C++ core engine, targeting WebAssembly (in-browser 60 FPS live preview) and Go via CGO (high-throughput server-side batch orchestration), wrapped in an Astro + React interactive studio.

---

## Key Features

- **7 Module Geometries**: `square`, `dots`, `rounded`, `extra-rounded`, `classy`, `classy-rounded`, and `fluid` (continuous 4-way organic connectivity).
- **Asymmetrical Corner Eyes**: Custom corner radii per vertex (Top-Left, Top-Right, Bottom-Right, Bottom-Left) for both outer frame and inner nucleus across all three positioning patterns.
- **Independent Color System**: Dedicated color control for data modules, background, eye frames, eye nuclei, and linear/radial multi-stop gradients with rotation support.
- **Smart Logo Placement**: Automatic cell clearance behind central logos with configurable square (rounded) or circular masks and custom padding.
- **OpenCV Visual Processing**: Background image blending, alpha compositing, luminance-based `autoColor` contrast optimization, and Lanczos antialiased rendering.
- **Zero-Allocation Vector SVG**: High-speed native SVG path generator for vector export without external graphic library overhead.
- **WebAssembly Engine**: Client-side execution in WebAssembly (Wasm) providing latency-free, 60 FPS live customization in the browser.
- **High-Concurrency Go Backend**: Thread-safe CGO integration with worker pools and Goroutines capable of orchestrating 100,000+ QR generation requests per minute.
- **Astro + Islands Frontend**: Lightweight, SEO-optimized web application with interactive studio island for real-time creation and multi-format export (`SVG`, `PNG`, `WebP`, `PDF`).

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Astro Web Application                    │
│   ┌─────────────────────────────────────────────────────┐   │
│   │   Interactive Studio Island (React / Svelte)        │   │
│   │   - Live parameter customization                    │   │
│   │   - Real-time client rendering via Wasm             │   │
│   │   - Instant multi-format export (SVG, PNG, WebP)    │   │
│   └──────────────────────────┬──────────────────────────┘   │
└──────────────────────────────┼──────────────────────────────┘
                               │
            ┌──────────────────┴──────────────────┐
            ▼                                     ▼
┌───────────────────────────────┐   ┌───────────────────────────────┐
│     Client-Side Execution     │   │     Server-Side Execution     │
│   - WebAssembly (Emscripten)  │   │   - Go REST / gRPC API        │
│   - 60 FPS in-browser preview │   │   - Goroutine Worker Pool     │
│   - Zero server cost          │   │   - CGO Bridge                │
└───────────────┬───────────────┘   └───────────────┬───────────────┘
                │                                   │
                └─────────────────┬─────────────────┘
                                  ▼
┌───────────────────────────────────────────────────────────────────┐
│                     Unified C++ Core Engine                       │
│  - Nayuki QR Code (Bit matrix & Reed-Solomon error correction)    │
│  - Topology & Geometry Kernels (4-way neighbor bitmasks)          │
│  - Vector SVG Path Builder (Linear/Radial gradients)              │
│  - OpenCV Pipeline (Blending, AutoColor contrast, Logo masking)   │
└───────────────────────────────────────────────────────────────────┘
```

---

## Repository Structure

```
qr-code-generator/
├── core/
│   ├── include/
│   │   ├── BitBuffer.hpp
│   │   ├── QrCode.hpp
│   │   ├── QrSegment.hpp
│   │   ├── QrTopology.hpp
│   │   ├── QrShapeRenderer.hpp
│   │   ├── QrEyeRenderer.hpp
│   │   ├── QrSvgBuilder.hpp
│   │   ├── QrImageProcessor.hpp
│   │   └── qr_engine_c_api.h
│   └── src/
│       ├── BitBuffer.cpp
│       ├── QrCode.cpp
│       ├── QrSegment.cpp
│       ├── QrTopology.cpp
│       ├── QrShapeRenderer.cpp
│       ├── QrEyeRenderer.cpp
│       ├── QrSvgBuilder.cpp
│       ├── QrImageProcessor.cpp
│       └── qr_engine_c_api.cpp
├── wasm/
│   ├── bindings.cpp
│   ├── build.sh
│   └── dist/
├── server/
│   ├── cmd/server/main.go
│   ├── internal/
│   │   ├── cgo/bridge.go
│   │   ├── service/worker_pool.go
│   │   └── handlers/qr_handler.go
│   ├── go.mod
│   └── go.sum
└── web/
    ├── public/
    │   └── wasm/
    ├── src/
    │   ├── components/QRStudio.tsx
    │   ├── layouts/BaseLayout.astro
    │   └── pages/index.astro
    ├── astro.config.mjs
    └── package.json
```

---

## Technology Stack

- **Core**: C++20, Project Nayuki (QR Matrix), OpenCV 4.x
- **WebAssembly**: Emscripten SDK (`emcc`)
- **Backend**: Go 1.22+, CGO, Goroutines
- **Frontend**: Astro 4.x, React 18, TypeScript, Tailwind CSS / Vanilla CSS

---

## License

This project is licensed under the [MIT License](LICENSE).
