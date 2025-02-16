# Qt Simulation Framework / Personal Portfolio

&#x20;&#x20;

A cross-platform (Windows, Linux) framework for building and recording 2D simulations.

- UI developed in Qt
- Uses https://github.com/QUItCoding/qnanopainter (NanoVG wrapper) for high-performance Canvas drawing.
- FFmpeg for video encoding

Included are Physics, Biology and Chemistry simulations, as well as other experiments.

## Features

- **Simulation Framework**
- - Layouts supporting multiple viewports / scenes
  - Custom Attribute Input List, for rapid prototyping (for starting conditions, or realtime feedback), e.g.
  - ```options->starting_slider("Particle Count", &particle_count, 10, 1000); # Slider only 'updates particle_count' on simulation start```
  - ```options->realtime_checkbox("Optimize Collisions", &optimize_collisions); # Updates 'optimize_collisions' boolean in realtime```
- **x264 Video Recording**
- - Custom resolution / FPS (uses offscreen FBO for rendering)
  - Window Capture (optional)
- **Custom Components**
- - 2D viewport (uses qnanopainter/NanoVG wrapper)
     - Support for Camera rotation
     - Conversion between Stage/World coordinates
     - Easy switching between World-Transform and Stage-Transform (useful for UI/Labels)
     - Viewport anchor (when zooming or resizing viewport)
  - Custom Spline Graph input component
- **Various Templates** - Physics / Biology / Chemistry templates to build on
- **Personal Projects**

## Screenshots

## Getting Started

### Prerequisites

- **Windows or Linux OS**
- **Qt 6.x or later** (Qt Creator)
- **CMake 3.16+**
- **Compiler:** MSVC or GCC

### Installation

1. Clone the repository:

   ```sh
   git clone https://github.com/willmh93/QtSim.git
   cd QtSim
   ```
2. (Windows) Extract FFmpeg_dlls.7z into: ``external\ffmpeg\windows\bin``
   
3. Build the project:

   ```sh
   cmake -B build
   ```

## Project Structure

```
QtSim/
├── src/
│   └── simulations/    # Auto-included Simulations
├── resources/
├── external/
│   └── ffmpeg/
│       └── windows/
│           ├── bin/
│           │   └── FFmpeg_dlls.7z  # (extract in-place)
│           ├── include/
│           ├── lib/
├── scripts
│   ├── new_sim.py      # Python helper for creating simulation
│   └── rename_sim.py   # Python helper for renaming simulation
├── templates
├── CMakeLists.txt      # Build system
├── README.md           # Project documentation
└── LICENSE             # License information
```

## Contributing

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature-xyz`).
3. Commit your changes (`git commit -m 'Add new simulation'`).
4. Push to the branch (`git push origin feature-xyz`).
5. Open a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENCE.txt) file for details.

## Contact

- **Author:** William Hemsworth
- **Email:** [wm.hemsworth@gmail.com](mailto\:wm.hemsworth@gmail.com)
- **GitHub:** [willmh93](https://github.com/willmh93)

