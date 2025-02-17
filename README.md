# <img src="resources/icon.png" alt="Icon" style="height:1em; vertical-align:middle;"> Qt Simulation Framework

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
    ```cpp
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
- **Python 3.12**
- **Compiler:** MSVC or GCC

## Installation

1. **Make sure Qt Creator is installed**
   - https://www.qt.io/download-qt-installer-oss
   - If using **Visual Studio**, add to your Environment PATH: ```C:\Qt\6.8.2\msvc2022_64\bin``` *(replace with your version)*

3. **Clone the repository**

   ```sh
   git clone https://github.com/willmh93/QtSim.git
   cd QtSim
   ```
4. **Open project in Qt Creator**
    - Select target kit
    - Click **Configure**
    - Wait for CMake to fetch dependencies and create project files

5. Create Visual Studio Solution *(Optional - Requires MSVC2022 64-bit kit)*
   - Run Generator (QtCreator: **Build** ─ **Run Generator**)
   - Open **build/Desktop_Qt_6_8_x_MSVC2022-Debug/qtc_Visual_Studio_17_2022/QtSim.sln**
     
6. **Compile and Run!**
   
## Simulation Tutorial

### Creating a new Simulation project
  - **Run Python Script**
    - ```python scripts/new_sim.py```
    - Enter Class Name        (e.g. **"Explosion"**)
    - Enter Descriptive Name  (e.g. **"Exploding Particles"**)
  - **This will generate your simulation source files**
    ```
    src/simulations/Explosion.h
    src/simulations/Explosion.cpp
    ```
  - When you run the application, your simulation **"Explosion"** should appear in the TreeView in the top-left corner
### Creating your Project / Scene and rendering it
- Your generated **Project** class can override these methods
    ```cpp
    void Explosion_Project::projectPrepare();
    void Explosion_Project::projectStart();
    void Explosion_Project::projectStop();
    void Explosion_Project::projectDestroy();
    ```
- Your generated **Scene** class can override these methods
    ```cpp
    void Explosion_Scene::sceneAttributes(Options* options);
    void Explosion_Scene::sceneStart();
    void Explosion_Scene::sceneMounted(Viewport *ctx);
    void Explosion_Scene::sceneStop();
    void Explosion_Scene::sceneDestroy();
    void Explosion_Scene::sceneProcess();

    // May be called multiple times on a single Scene if mounted to multiple Viewports
    void Explosion_Scene::viewportProcess(Viewport* ctx);
    void Explosion_Scene::viewportDraw(Viewport* ctx);

    // Handle Mouse Events (only invoked on Viewport mouse interaction)
    void Explosion_Scene::mouseDown();
    void Explosion_Scene::mouseUp();
    void Explosion_Scene::mouseMove();
    void Explosion_Scene::mouseWheel();
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

