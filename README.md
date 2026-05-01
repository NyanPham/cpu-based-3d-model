# 3D Software Renderer

A lightweight 3D software renderer written in pure C, implementing a complete graphics pipeline from scratch without any GPU APIs (OpenGL, DirectX, Vulkan).

## 🎯 Features

- **Software Rasterization**: All 3D to 2D conversion done on CPU
- **Perspective Projection**: Proper depth with perspective divide
- **Frustum Clipping**: Sutherland-Hodgman polygon clipping
- **Back-face Culling**: Dot product triangle facing test
- **Flat Shading**: Directional light with normal-based intensity
- **Texture Mapping**: Perspective-correct UV interpolation
- **Z-Buffer**: Depth testing for correct triangle ordering

## 🏗️ Graphics Pipeline

```
Vertices (OBJ) → World Transform → View Transform → Clipping → 
Perspective Divide → Screen Mapping → Rasterization → Display
```

## 📁 Project Structure

```
3drenderer/
├── src/
│   ├── array.c/h      # Dynamic array (stretchy buffer)
│   ├── camera.c/h     # Virtual camera & view matrix
│   ├── clipping.c/h   # Frustum culling & polygon clipping
│   ├── display.c/h   # SDL window, color buffer, Z-buffer
│   ├── light.c/h     # Directional light & flat shading
│   ├── main.c        # Graphics pipeline & game loop
│   ├── matrix.c/h    # 4x4 transformation matrices
│   ├── mesh.c/h      # OBJ file loader
│   ├── triangle.c/h  # Rasterization & barycentric interpolation
│   ├── vector.c/h    # 2D/3D/4D vector math
│   └── texture.c/h   # UV texture coordinates
├── assets/            # .obj models & .png textures
└── Makefile          # Build configuration
```

## ⌨️ Controls

| Key | Action |
|-----|--------|
| `W` / `S` | Look up / down (pitch) |
| `←` / `→` | Look left / right (yaw) |
| `↑` / `↓` | Move forward / backward |
| `1` | Wireframe vertices |
| `2` | Wireframe |
| `3` | Filled triangles |
| `4` | Filled + wireframe |
| `5` | Textured triangles |
| `6` | Textured + wireframe |
| `C` | Enable back-face culling |
| `X` | Disable back-face culling |
| `ESC` | Exit |

## 🛠️ Build & Run

```bash
# On Linux/macOS
make
./3drenderer

# On Windows (with MinGW)
mingw32-make
./3drenderer.exe
```

## 📚 Key Concepts Explained

### Matrix Transformations
- **World Matrix**: Scale → Rotate → Translate
- **View Matrix**: Move world opposite to camera (LookAt)
- **Projection Matrix**: Perspective frustum mapping

### Rasterization
- **Scanline Algorithm**: Split triangle into flat-bottom/top
- **Barycentric Coordinates**: Interpolate depth, UV, color across triangle

### Depth Correction
- **1/w Interpolation**: Depth is non-linear in screen space, but 1/w is linear
- **Perspective-Correct UV**: Store u/w and v/w at vertices, interpolate, then divide

## 📖 Learning Resources

This codebase is heavily commented to serve as a learning tool. Key files to study:

1. **[matrix.c](src/matrix.c)** - Perspective projection & LookAt derivation
2. **[vector.c](src/vector.c)** - Dot & cross product intuition
3. **[triangle.c](src/triangle.c)** - Rasterization & barycentric weights
4. **[clipping.c](src/clipping.c)** - Sutherland-Hodgman algorithm
5. **[main.c](src/main.c)** - Complete graphics pipeline

## 🔧 Dependencies

- SDL2 (`libsdl2-dev`)
- PNG library (`libpng-dev`)

## 📝 License

MIT License - Feel free to use this for learning or building your own software renderer!

---

*Built as a learning project to understand how 3D graphics actually work under the hood.*