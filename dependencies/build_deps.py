import subprocess

# Configure CMake (equivalent to `cmake -S . -B build`)
subprocess.run(["cmake", "-S", ".", "-B", "build"], check=True)

# Build Project (equivalent to `cmake --build build --config Release`)
subprocess.run(["cmake", "--build", "build", "--target", "install"], check=True)
