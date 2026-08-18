#include "core/PixelPipeline.h"

// =====================================================================
// PixelPipeline 实现（骨架，代码待用户填充）
// 填充指引见 doc/WorkPlan.md 3.3 节
// =====================================================================

PixelPipeline::PixelPipeline()
{
    // TODO(用户): 初始化内部状态（图元/迭代器游标/z-buffer）
}

PixelPipeline::~PixelPipeline()
{
    // TODO(用户): 如有动态资源在此释放
}

void PixelPipeline::begin(const QVector<PixelVertex>& verts,
                          const QVector<PixelPrimitive>& prims)
{
    // TODO(用户):
    //   1) 深拷贝顶点池与图元列表
    //   2) Stage0 顶点变换（本地 -> 裁剪空间，需要 PixelMath）
    //   3) 重置迭代器游标（指向第 0 个图元）
}

void PixelPipeline::reset()
{
    // TODO(用户): 游标归零、z-buffer 清空，但保留顶点/图元数据
}

bool PixelPipeline::nextFragment(PixelFragment& out)
{
    // TODO(用户): 可暂停光栅化迭代器（核心）
    //   1) 若已耗尽返回 false
    //   2) 对当前图元逐像素产出：
    //        - Point : 直接产出 1 像素
    //        - Line  : Bresenham 逐步产出（维护 err/dx/dy 状态）
    //        - Triangle : 包围盒内逐像素，边界函数判定 + 重心坐标插值
    //   3) S5 着色（Flat/Gouraud），S6 与 CPU z-buffer 比较
    //   4) 填充 out（x/y/color/depth/primitiveId/order），返回 true
    Q_UNUSED(out);
    return false;
}

int PixelPipeline::pendingCount() const
{
    // TODO(用户): 估算剩余像素数（可粗略，供教学进度显示）
    return 0;
}

bool PixelPipeline::isFinished() const
{
    // TODO(用户): 返回是否已处理完所有图元的所有像素
    return true;
}
