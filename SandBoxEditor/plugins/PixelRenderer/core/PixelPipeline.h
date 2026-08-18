#ifndef PIXELPIPELINE_H
#define PIXELPIPELINE_H

#include "core/PixelTypes.h"
#include <QVector>

// CPU 模拟光栅化管线（core/）
// 设计来源：PixelRendererDesign.md 2.4 节；GPUPipelineSimulation.md 3.4 节
// 与 PixelCreator 的软件光栅化算法一致，差异仅在产出去向：
// 逐像素产出 PixelFragment 交给呈现层（PixelPainter），而非写入 QImage。
//
// 教学核心是"可暂停迭代器"（令牌驱动）：每次 nextFragment 只产出 1 个像素，
// 支持单步 / 慢放 / 回放。深度测试仅用 CPU z-buffer（已确认决策）。
class PixelPipeline
{
public:
    PixelPipeline();                             // 构造函数
    ~PixelPipeline();                            // 析构函数

    // 装配顶点池与图元列表，并重置管线状态
    void begin(const QVector<PixelVertex>& verts,
               const QVector<PixelPrimitive>& prims);
    void reset();                                // 回到初始状态（不销毁数据）
    bool nextFragment(PixelFragment& out);       // 产出下一个片元；耗尽返回 false
    int pendingCount() const;                    // 剩余待产出像素数（教学显示）
    bool isFinished() const;                     // 是否已全部产出

private:
    // TODO(用户): 内部状态机实现（填充指引见 doc/WorkPlan.md 3.3 节）
    //   1. Stage0 顶点变换：v' = P * V * M * v（需要 PixelMath，拷贝自 PixelCreator 数学库）
    //   2. S1 裁剪/剔除、S2 NDC/视口（y 翻转）
    //   3. S4 可暂停光栅化迭代器：点/线(Bresenham)/三角形(边界函数) 逐像素
    //   4. S5/S6 着色 + CPU z-buffer 深度比较
    //   成员建议：顶点池/图元池副本、当前图元索引、行/列游标、z-buffer 表
};

#endif // PIXELPIPELINE_H
