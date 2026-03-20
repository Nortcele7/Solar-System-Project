# Interactive Solar Sytem Simulator | SSS

This project is a real-time 3D solar-system simulator built using modern OpenGL (core profile) in C++.

This README keeps only concepts that are used or partially used in this project.

## 1. Introduction and Application

### 1.2 Overview of Graphic Systems

#### 1.2.1 Video Display Devices
- Raster-scan displays
  - `Status`: Used implicitly.
  - `Where`: Framebuffer rendering through OpenGL window in `src/main.cpp`.
- Flat-panel displays
  - `Status`: Used implicitly by running on modern monitors.
- 3D viewing devices
  - `Status`: Partially used.
  - `Where`: 3D camera and perspective projection in `src/main.cpp`.

#### 1.2.2 Graphics Software and Tools
- Coordinate representations
  - `Status`: Used.
  - `Where`: `glm::vec3`, matrices, transformations in `src/main.cpp`.
- Graphics functions
  - `Status`: Used.
  - `Where`: OpenGL draw calls and shader uniforms in `src/main.cpp`.
- Software standards
  - `Status`: Used.
  - `Where`: OpenGL 3.3 core, GLSL, CMake in `CMakeLists.txt` and `src/main.cpp`.
- OpenGL
  - `Status`: Used (primary API).
  - `Where`: Entire rendering pipeline in `src/main.cpp` and mesh setup in `include/Sphere.h`.

### 1.3 Graphics Pipeline

#### 1.3.1 2D Viewing Pipeline
- `Status`: Partially used.
- `Where`: 2D overlay text drawn in normalized device coordinates in `src/main.cpp`.

#### 1.3.2 3D Viewing Pipeline
- `Status`: Used.
- `Where`:
  - Model transforms per celestial body in `DrawBodies` (`src/main.cpp`).
  - View transform via `glm::lookAt` in `Render` (`src/main.cpp`).
  - Projection transform via `glm::perspective` in `Render` (`src/main.cpp`).
  - Vertex processing and lighting in GLSL shaders in `src/main.cpp`.

### 1.4 Applications in Fields
- `Status`: Partially used.
- `How represented`: Engineering/scientific visualization through interactive simulation.

## 2. Raster Graphics and Algorithms

### 2.1 Rasterizing a Point
- `Status`: Used.
- `Where`: Starfield, asteroid belt, Kuiper belt, and text glyph pixels rendered using `GL_POINTS` in `src/main.cpp`.

### 2.8 Point Clipping
- `Status`: Used implicitly by OpenGL clipping stages.

### 2.9 Line Clipping
- `Status`: Used implicitly by OpenGL clipping stages.

### 2.11 Text Clipping
- `Status`: Partially used implicitly by viewport clipping.
- `Where`: Overlay text points outside NDC are clipped by pipeline.

## 3. 2D and 3D Coordinate Systems and Viewing Transformations

### 3.1 2D Transformation
- `Status`: Partially used.
- `Where`: Overlay text placement uses 2D screen-to-NDC mapping in `src/main.cpp`.

### 3.3 Window-to-Viewport Coordinate Transformation
- `Status`: Used.
- `Where`: Mouse picking conversion to normalized coordinates in `ProcessQueuedPick` (`src/main.cpp`).

### 3.4.2 Perspective Projection
- `Status`: Used.
- `Where`: `glm::perspective` in `Render` and picking path in `ProcessQueuedPick` (`src/main.cpp`).

### 3.5 3D Transformation
- `Status`: Used.
- `Where`: Translation, rotation, scaling for bodies/rings in `DrawBodies` (`src/main.cpp`).

### 3.6 3D Composite Transformation
- `Status`: Used.
- `Where`: Combined model matrices per object in `DrawBodies` (`src/main.cpp`).

### 3.7 Projection and Viewing Transformation
- `Status`: Used.
- `Where`: `glm::lookAt` and `glm::perspective` in `Render` (`src/main.cpp`).

## 4. Curve Modeling and Surface Modeling

### 4.2 Surface Modeling (Polygon Surface, Normals, Orientation)
- `Status`: Used.
- `Where`:
  - Sphere triangle mesh generation in `include/Sphere.h`.
  - Ring mesh generation in `src/main.cpp`.
  - Normal vectors used for lighting in shaders (`src/main.cpp`).

## 5. Visible Surface Determination

### 5.1 Image Space and Object Space Techniques
- `Status`: Used (image-space depth testing).
- `Where`: `glEnable(GL_DEPTH_TEST)` in `Initialize` (`src/main.cpp`).

### 5.2 Z-Buffer
- `Status`: Used.
- `Where`: OpenGL depth test (`GL_DEPTH_TEST`) in `src/main.cpp`.

## 6. Illumination and Surface Rendering Methods

### 6.1 Ambient, Diffuse, Specular Reflection Algorithms
- `Status`: Used.
- `Where`: Fragment shader computes ambient + diffuse + specular terms in `src/main.cpp`.

### 6.2 Phong Shading
- `Status`: Used.
- `Where`: Per-fragment lighting using interpolated normals in the body fragment shader (`src/main.cpp`).

## 7. Computer Animation and Visualization

### 7.1 Computer Animation Functions
- `Status`: Used.
- `Where`: Time-step update of orbit and spin in `Update` (`src/main.cpp`).

### 7.2 Raster Animations
- `Status`: Used.
- `Where`: Continuous frame rendering loop in `Run` (`src/main.cpp`).

### 7.4.1 Direct-motion Specifications
- `Status`: Used.
- `Where`: Procedural orbital and rotational equations in `Update` (`src/main.cpp`).

### 7.4.2 Goal-directed Systems
- `Status`: Partially used.
- `Where`: Camera target interpolation toward selected body in `Update` (`src/main.cpp`).

### 7.4.3 Kinematics and Dynamics
- `Status`: Partially used.
- `Where`: Kinematic motion only (no force-based physics integrator).

## 8. Latest Trends in Computer Graphics

### 8.1 Interactive Visualization
- `Status`: Used.
- `Where`: Interactive controls, picking, overlays in `src/main.cpp`.

### 8.4 Game Development and Real-time Graphics
- `Status`: Used conceptually.
- `Where`: Real-time render loop, GPU shading, and camera interaction in `src/main.cpp`.

## Build and Run

### Requirements
- C++17+
- CMake 3.10+
- OpenGL 3.3+
- GLFW
- GLEW
- GLM

### Ubuntu/Debian
```bash
sudo apt-get install libglfw3-dev libglew-dev libglm-dev cmake build-essential
```

### Build
```bash
cd /home/shreyam/Desktop/graphics-project
mkdir -p build
cd build
cmake ..
make
```

### Run
```bash
./solar_system
```
