# 测试与混合目录（output/）

阶段5 测试与混合：深度/Alpha 测试后写入帧缓冲（见 GPUPipelineSimulation.md 3.2.1）：

- OutputStage.h  深度测试 + Alpha 混合

对应：教材"测试与混合"；UE EZ / LZ / ROP。
帧缓冲载体（颜色 + 深度 + 轨迹）在 core/SimFrameBuffer.h。
