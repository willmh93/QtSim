# Qt Simulation Framework / Personal Portfolio

&#x20;&#x20;

A cross-platform (Windows, Linux) framework for building and recording 2D simulations.

- UI developed in Qt
- Uses https://github.com/QUItCoding/qnanopainter (NanoVG wrapper) for high-performance Canvas drawing with OpenGL
- FFmpeg for video encoding

Included are Physics, Biology and Chemistry simulations, as well as other experiments.

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
  
- - Custom resolution / FPS (uses offscreen FBO for rendering)
  - Window Capture (optional)
    
- **Custom Qt Components**
     
  - Custom Spline Graph input component (coming soon)
    
- **Various Templates** - Physics / Biology / Chemistry templates to build on
  
- **My Personal Projects**

## Gallery

## Prerequisites

- **Windows or Linux OS**
- **Qt 6.x or later** (Qt Creator)
- **CMake 3.16+**
- **Python 3.12** *(optional but recommended for creating new simulations from templates)*
- **Compiler:** MSVC or GCC

## Installation

1. Clone the repository:

   ```sh
   git clone https://github.com/willmh93/QtSim.git
   cd QtSim
   ```
2. (Windows) Extract FFmpeg_dlls.7z into: ``external\ffmpeg\windows\bin``
   
3. Build the project with CMake (automatically fetches other dependencies):

   ```sh
   cmake -B build
   ```

## Simulation Tutorial
### Creating a new simulation
  - **Run Python Script:**
    - ```python scripts/new_sim.py```
    - Enter Class Name        (e.g. **"Explosion"**)
    - Enter Descriptive Name  (e.g. **"Exploding Particles"**)
  - **Open Project in Qt Creator**
      - Optionally, generate a Visual Studio 2022 Solution (QtCreator Menu: **Build** --> **Run Generator**)
  - **Find your generated simulation source files:**
    ```
    src/simulations/Explosion.h
    src/simulations/Explosion.cpp
    ```
  - If you run the application, your simulation should now appear in the TreeView

### Creating your Scene and rendering it
- Your **Scene** can override these various methods:
    ```
    virtual void instanceAttributes(Options* options)
    virtual void start()
    virtual void mount(Panel *ctx)
    virtual void stop()
    virtual void destroy()
    virtual void processScene()
    virtual void processPanel(Panel* ctx)
    virtual void draw(Panel* ctx)
    
    virtual void mouseDown() {}
    virtual void mouseUp() {}
    virtual void mouseMove() {}
    virtual void mouseWheel() {}
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
3. Commit your changes (`git commit -m 'Added new simulation template'`).
4. Push to the branch (`git push origin feature-xyz`).
5. Open a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENCE.txt) file for details.

## Contact

- **Author:** William Hemsworth
- **Email:** [wm.hemsworth@gmail.com](mailto\:wm.hemsworth@gmail.com)
- **GitHub:** [willmh93](https://github.com/willmh93)

