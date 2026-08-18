# 应用层目录（application/）

阶段1 应用层：模拟 CPU 处理，接收输入并生成 DrawCall（见 GPUPipelineSimulation.md 3.2.1）：

- SceneGen.h         场景输入（顶点/图元，教学注入）
- SimDrawCall.h      一次绘制调用的完整描述
- DrawCallBuilder.h  场景 -> DrawCall 列表（模拟 CPU 处理）
- ISimRHI.h          RHI 抽象（模拟 D3D12 / Vulkan / GL 分发与翻译）
- SimCommandStream.h 命令流（DrawCall 队列，供下游消费）

对应：教材"应用程序阶段"；UE GameThread + RenderThread（MeshDrawCommand）+ RHI 线程。
