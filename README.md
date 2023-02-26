# Tiny Renderer G2

A naive renderer with some physics.

*TinyRendererG2* is implemented in **C++17(MSVC)**, **OpenGL3.3+** and **CUDA11.6**, organized by **CMake3.10+** and **Visual Studio 2022**.

## Dependencies

All dependencies are provided under `thirdparty`.

- Eigen3: `thirdparty\eigen`
- GLAD: `thirdparty\GLAD`
- GLFW: `thirdparty\GLFW`
- GLM: `thirdparty\glm`
- IMGUI: `thirdparty\imgui`, Backend = OpenGL+GLFW
- json11: `thirdparty\json11`
- stb: `thirdparty\stb`

## Build

```powershell
mkdir build
cd build
cmake ..
start tinyRendererG2.sln
```

## File

- app: main
  
- render: rendering the related code, very confusing, I will rewrite it of late.
  
- simulation: PD, PBD, and more in the near future. I think it's somewhat elegantly implemented...
  
- UI: ImGUI layout.
  
- utility: literally.
  
- pyscripts: of no use.

## 关于怎么用：

我好像不小心写出了一坨shi山。

目前的实现基于很多回调，回调的所有绑定均由lambda捕获。

### 渲染：

理想上是在 `asset\scene`里面写场景的json文件，然后直接加载渲染。再由GUI控制实体的物理属性。但是现在渲染逻辑有些混乱，所以目前写死了直接加载 `asset\scene\scene.json`。

可以加载任意模型，线框绘制边，顶点绘制Gizmo；模型需要预处理一下，不是标准的OBJ格式。

`TinyRenderer`渲染器对象接受3个`void()`回调函数，分别:
- `_UI_layout_update`: ImGUI的layout绘制。
- `_UI_event_handler`: GUI事件处理。包括重载物理场景。
- `_physics_tick`: loop中的物理帧。



### 物理：
`TinyPhyxSole`的两个方法：
- `get_reset_foo`: 返回重置函数的回调。
- `get_physics_tick_foo`: 返回物理帧的回调。

`TinyPhyxSole`拥有`PD_solver` 或 `PBD_solver`，实际的求解器。
`TinyPhyxSole`拥有`PhyMesh`，保存了所有物理属性。
 

物理的初始化在`UI/src/UI_event.cpp: reload_physics_scene`中实现。