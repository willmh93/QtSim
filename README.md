# Qt Simulation Portfolio

&#x20;&#x20;

A collection of physics-based simulations and interactive experiments designed in Qt.

## Features

- **Custom Qt Components** - Tailored UI/UX using Qt Widgets & QML.
- **Physics Engine** - Lightweight physics simulation for realistic object interactions.
- **3D Rendering** - OpenGL/QtQuick-based visualizations.
- **Minimal Dependencies** - Only core Qt modules and essential libraries.
- **High Performance** - Optimized for real-time execution.

## Screenshots



## Getting Started

### Prerequisites

- **Qt 6.x or later** (Qt Creator recommended)
- **CMake 3.16+**
- **Compiler:** MSVC, Clang, or GCC

### Installation

1. Clone the repository:

   ```sh
   git clone https://github.com/yourusername/qt-simulation-portfolio.git
   cd qt-simulation-portfolio
   ```

2. Configure and build the project:

   ```sh
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)  # Use 'nmake' or 'cmake --build .' on Windows
   ```

3. Run the application:

   ```sh
   ./qt-simulation-portfolio
   ```

## Project Structure

```
qt-simulation-portfolio/
├── src/              # Source code
│   ├── core/         # Simulation core logic
│   ├── ui/           # Qt-based UI components
│   ├── physics/      # Physics engine
│   ├── renderer/     # OpenGL rendering (if applicable)
│   ├── main.cpp      # Entry point
├── assets/           # Textures, models, and assets
├── CMakeLists.txt    # Build system
├── README.md         # Project documentation
└── LICENSE           # License information
```

## Simulations Included

- **Rigid Body Dynamics** – Collision handling and object interactions.
- **Particle Systems** – Fluid and gas-like behaviors.
- **Spring-Mass Systems** – Realistic elasticity modeling.
- **Electromagnetism** – Visualizing electromagnetic fields.

## Contributing

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature-xyz`).
3. Commit your changes (`git commit -m 'Add new simulation'`).
4. Push to the branch (`git push origin feature-xyz`).
5. Open a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Contact

- **Author:** William Hemsworth
- **Email:** [wm.hemsworth@gmail.com](mailto\:wm.hemsworth@gmail.com)
- **GitHub:** [willmh93](https://github.com/willmh93)

