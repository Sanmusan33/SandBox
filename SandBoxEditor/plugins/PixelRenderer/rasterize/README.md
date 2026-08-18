# 光栅化目录（rasterize/）

阶段3 光栅化：把屏幕空间顶点离散化为片元，生成插值后的片元属性（见 GPUPipelineSimulation.md 3.2.1）：

- PixelRasterizeStage.h  点/线/三角形逐像素离散化 + 属性插值（复用 core/PixelPipeline 迭代器）

对应：教材"光栅化阶段"；UE RAST。
