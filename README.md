# Photorealistic Scene with OpenGL

## Overview

This project presents a photorealistic scene rendered in OpenGL, featuring a blend of industrial age machinery and futuristic technology set in a barren, arid landscape. With interactive elements, animations, lighting, and shadowing, users can explore the scene using keyboard and mouse controls.

## Features

- **Free Movement:** Navigate through the scene using the W, A, S, D, Z, X keys and mouse.
- **Interactive Elements:** Engage with animated objects and manipulate environmental factors like lighting and fog density.
- **Presentation Mode:** Automatically rotate the camera around the scene with the press of a button.
- **Customizable Settings:** Toggle night mode, view shadow maps, and adjust fog density through keyboard shortcuts.

## Implementation Details

### Project Structure
The SceneProject is structured into several key components, each responsible for different aspects of the 3D rendering engine:

- **Main Application (`main.cpp`)**: The entry point of the application, setting up the rendering window and event loop.
- **Mesh (`Mesh.cpp`, `Mesh.hpp`)**: Handles the loading, setup, and rendering of 3D models.
- **Model 3D (`Model3D.cpp`, `Model3D.hpp`)**: Manages complex 3D models composed of multiple meshes.
- **Camera (`Camera.cpp`, `Camera.hpp`)**: Implements camera movement and perspective management.
- **SkyBox (`SkyBox.cpp`, `SkyBox.hpp`)**: Provides an environmental background using cube mapping.
- **Shader (`Shader.cpp`, `Shader.hpp`)**: Compiles and manages vertex and fragment shaders for rendering.
- **Utilities**:
  - `stb_image.h/cpp`: A single-header library for image loading, used for textures.
  - `tiny_obj_loader.h/cpp`: A tiny OBJ file loader for reading mesh data.

### Graphics Pipeline
The project utilizes OpenGL as its core rendering API, integrating GLSL shaders for real-time graphics rendering. The pipeline stages include:

1. **Model Loading**: OBJ models are loaded using `tiny_obj_loader`, parsed into mesh data.
2. **Texture Mapping**: Textures are applied to meshes using UV coordinates, with `stb_image` for image loading.
3. **Shader Processing**: Vertex and fragment shaders (`shaders` directory) transform and color the 3D geometry based on lighting and camera position.
4. **Rendering Loop**: The main application loop handles event processing, camera updates, and drawing commands.

## User Manual

### Controls

| Key(s)       | Action                                      |
|--------------|---------------------------------------------|
| W, A, S, D   | Move Forward/Left/Backward/Right            |
| Z, X         | Move Up/Down                                |
| MOUSE        | Look around                                 |
| ESC          | Exit the scene                              |
| CONTROL      | Toggle mouse capture/release                |

### Hot Keys for Enhanced Interaction

| Key(s)       | Action                                      |
|--------------|---------------------------------------------|
| M            | View shadow mapping                         |
| O, P         | Activate/deactivate night mode              |
| ARROW KEYS   | Move Forward/Left/Right/Backward Wall-E     |
| Q, E         | Rotate Left/Right Wall-E                    |
| J, L         | Rotate Left/Right light cube                |
| 1, 2, 3      | Change viewing modes (Line/Point/Normal view)|
| F, G         | Increase/Decrease fog density               |
| 9, 0         | Start/Stop scene presentation mode          |
| 5, 6         | Start/Stop point light                      |


These hotkeys allow users to interact with the scene dynamically, providing a rich and immersive experience. Experiment with these controls to discover the full capabilities of the OpenGL scene.

## Conclusions and Further Development

This project demonstrates the capabilities of OpenGL for creating photorealistic scenes. Future enhancements could include third-person character movement and expanded interactive features.
