# Qt Simulation Framework / Personal Portfolio

&#x20;&#x20;

A cross-platform (Windows, Linux) framework for building and recording 2D simulations

- UI developed in Qt
- Uses https://github.com/QUItCoding/qnanopainter (NanoVG wrapper) for high-performance Canvas drawing with OpenGL
- FFmpeg for video encoding

Included are some Physics, Biology and Chemistry simulations experiments

## Features

- **Simulation Framework**
  - **Layout** - Supports multiple Viewports / Scenes
  - **Viewport** - For hassle-free performant 2D drawing (uses Qnanopainter, NanoVG wrapper)
  - **Scene** - A scene be mounted to multiple **Viewports** for shared **Scenes** with multiple **Cameras** (each Viewport has it's own Camera)
  - **Camera**
     - Position / Rotation / Panning
     - Easy conversion between Stage / World coordinates
  - **DrawingContext**
     - Easy switching between World-Transform and Stage-Transform (useful for UI / labeling)
     - Viewport anchor (when zooming or resizing viewport)
  - **AttributeList** - List of inputs (sliders, checboxes, etc) designed for rapid prototyping. e.g.
    ```
    options->starting_slider("Particle Count", &particle_count, 10, 1000);
    options->realtime_checkbox("Optimize Collisions", &optimize_collisions);
    ```
  - **Bitmap** - Uses Offscreen FBO for high-performance Pixel manipulation
- **x264 Video Recording**
  - Custom record resolution / FPS (uses offscreen FBO for rendering)
  - Window Capture (optional)
    
- **Custom Qt Components**
  - Custom Spline Graph input component (coming soon)

## Gallery

## Prerequisites

- **Windows or Linux OS**
- **Qt 6.8.x or later** (Qt Creator)
- **CMake 3.16+**
- **Python 3.12** *(optional but recommended for creating new simulations from templates)*
- **Compiler:** MSVC or GCC

## Installation

1. Clone the repository:

   ```sh
   git clone https://github.com/willmh93/QtSim.git
   cd QtSim
   ```
2. Open project in Qt Creator
    - Select target kit (**MinGW 64-bit**, or **MSVC2022 64-bit** if Visual Studio build)
    - Click **Configure**
    - Wait for CMake to fetch dependencies and create project files

3. If using Visual Studio:
   - Run Generator (QtCreator Menu: **Build** --> **Run Generator**)
   - Open **build/Desktop_Qt_6_8_x_MSVC2022-Debug/qtc_Visual_Studio_17_2022/QtSim.sln**
     
4. Compile and run!
   
## Simulation Tutorial

### Creating a new simulation
  - **Run Python Script**
    - ```python scripts/new_sim.py```
    - Enter Class Name        (e.g. **"Explosion"**)
    - Enter Descriptive Name  (e.g. **"Exploding Particles"**)
  - **This will generate your simulation source files**
    ```
    src/simulations/Explosion.h
    src/simulations/Explosion.cpp
    ```
  - When you run the application, your simulation should appear in the TreeView in the top-left corner
### Creating your Scene and rendering it
- Your **Scene** can override these various methods
    ```
    virtual void instanceAttributes(Options* options)
    virtual void startScene()
    virtual void onMount(Viewport *ctx)
    virtual void stopScene()
    virtual void destroyScene()
    virtual void processScene()
    virtual void processViewport(Viewport* ctx)
    virtual void drawViewport(Viewport* ctx)
    
    virtual void mouseDown() {}
    virtual void mouseUp() {}
    virtual void mouseMove() {}
    virtual void mouseWheel() {}
    ```

## Key Files / Directories

```
QtSim/
├── src/
│   └── simulations/
│       └── # Auto-included Simulations...
├── scripts
│   ├── new_sim.py                  # Python helper for creating simulation
│   └── rename_sim.py               # Python helper for renaming existing simulation
└── CMakeLists.txt
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature-xyz`)
3. Commit your changes (`git commit -m 'Added new simulation template'`)
4. Push to the branch (`git push origin feature-xyz`)
5. Open a pull request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENCE.txt) file for details.

## Contact

- **Author:** William Hemsworth
- **Email:** [wm.hemsworth@gmail.com](mailto\:wm.hemsworth@gmail.com)
- **GitHub:** [willmh93](https://github.com/willmh93)

