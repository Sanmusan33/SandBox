# 几何处理目录（geometry/）

阶段2 几何处理：顶点变换 + 光照计算 + 裁剪，输出屏幕空间顶点（见 GPUPipelineSimulation.md 3.2.1）：

- GeoStage.h  S0 顶点变换（MVP）+ VS 光照（Gouraud）+ 裁剪/NDC/视口/装配

对应：教材"几何处理阶段"；UE VS / CLIP / PERSP / VIEW。
