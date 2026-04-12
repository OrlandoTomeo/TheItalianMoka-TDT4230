# The Italian Moka Ritual ☕

**Course:** TDT4230 Graphics and Visualization  
**Institution:** Norwegian University of Science and Technology (NTNU)  
**Author:** Orlando Tomeo  
**Academic Year:** Spring 2026

---

## Project Overview
"The Italian Moka Ritual" is an advanced real-time 3D graphics application built with **C++** and **OpenGL 4.3 Core Profile**. The project recreates the atmospheric and physical process of brewing coffee, focusing on programmatic generation and physically accurate light interactions.

Rather than relying on high-poly external models, this assignment emphasizes **procedural geometry**, custom **PBR (Physically Based Rendering)** shaders, and dynamic physics systems.

![Final Scene Render](Images/imgcompleta.png)

## Technical Implementation

### 1. Advanced Shading (PBR)
The scene utilizes a custom implementation of the **Cook-Torrance BRDF**.
* **Microfacet Theory:** Uses Trowbridge-Reitz GGX for normal distribution and Smith’s Schlick-GGX for geometry shadowing.
* **Multi-Material Logic:** The Moka pot mesh dynamically switches between polished aluminum and matte Bakelite (plastic) based on object-space coordinates.
* **Procedural Textures:** Wall tiles and cast-iron stove surfaces are generated mathematically using Perlin noise and grid masks directly in the fragment shader.

### 2. Lighting & Shadow Mapping
A robust **Two-Pass Shadow Mapping** system anchors the objects in the room.
* **Depth Pass:** Renders the scene from the light's perspective into a high-resolution Framebuffer Object (FBO).
* **PCF (Percentage-Closer Filtering):** Shadows are softened using a 3x3 sampling kernel to eliminate aliasing.
* **Dynamic Attenuation:** The gas fire acts as a point light with a specific quadratic falloff: $E = 1 / (K_c + K_l d + K_q d^2)$.

### 3. Procedural Geometry
To demonstrate low-level vertex management, most environmental assets are generated via a custom `buildCylinder` function using trigonometric distributions for positions and normals.

### 4. Particle Physics & Cycle Logic
A modular `ParticleSystem` manages three distinct physical simulations:
* **Gas Fire:** Additive blending and flickering intensity.
* **Steam:** Rising vapor with pseudo-collision detection against the Moka lid.
* **Coffee:** Dark, viscous particles with downward gravity.
* **Ritual Timer:** All systems are synchronized via an `fmod` timer to simulate the 10-second boiling cycle.

### 5. Immersive Audio
A dedicated **OpenAL** module handles spatialized 3D sound.
* **SFX:** Looping boiling sounds synchronized with the visual steam.
* **Music:** Background atmosphere with dedicated volume control.

## Project Structure
* `/src`: Main application logic and geometry generators.
* `/shaders`: GLSL source files (moka, env, particle, shadow).
* `/models`: Faceted Moka pot mesh.
* `/audio`: WAV files for the boiling effect and music.
* `/external`: Dependency management (GLFW, GLAD, GLM, Assimp).

## Requirements & Build
* **Compiler:** C++17 compliant.
* **Dependencies:** GLFW, GLAD, GLM, Assimp, OpenAL.
* **Build System:** CMake.

---
*Developed as a final project for the TDT4230 course at NTNU.*
