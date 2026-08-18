#ifndef RASTERCAMERA_H      // 头文件保护宏开始：防止同一头文件被重复包含
#define RASTERCAMERA_H      // 定义保护宏：首次包含后即为"已定义"状态

#include "RasterMath.h"     // 引入数学库（Vec3/Mat4），提供向量与矩阵运算

// 自由视角相机
//  - 鼠标拖拽：旋转视角（yaw/pitch）
//  - WASD：相对视线方向平移（W/S 前后，A/D 左右）
//  - Q/E：世界坐标系垂直升降
// 后续将作为光栅化管线 View 矩阵的来源
class RasterCamera         // 相机类：维护相机的位置与朝向（yaw/pitch 角度）
{
public:
    RasterCamera();        // 构造函数：成员变量已就地初始化，故为空实现

    // 拖拽增量旋转（dx/dy 为屏幕像素增量）
    void orbit(float dx, float dy);   // 根据鼠标位移增量旋转视角（水平增量改 yaw，垂直增量改 pitch）

    // 相对视线方向平移（amount 为世界单位）
    void moveForward(float amount);   // 沿当前视线方向平移（正值为前进）
    void moveRight(float amount);     // 沿当前视线右向平移（正值为右移）

    // 世界坐标系垂直升降
    void moveUp(float amount);        // 沿世界 Y 轴垂直升降（正值为上升）

    void setPosition(const RasterMath::Vec3& pos);  // 直接设置相机在世界坐标系中的位置
    RasterMath::Vec3 position() const { return m_cameraPos; }  // 内联获取相机当前位置

    RasterMath::Vec3 forward() const;   // 由 yaw/pitch 计算前向单位向量（相机视线方向）
    RasterMath::Vec3 right() const;     // 由前向向量计算右向单位向量

    RasterMath::Mat4 viewMatrix() const;  // 生成相机视图矩阵（View）：供 投影×视图 变换使用

private:
    RasterMath::Vec3 m_cameraPos{0.f, 1.f, 1.f};  // 相机位置：初始在原点上方 1.6 单位、前方 5 单位
    float m_yaw = 0.f;    // 偏航角（弧度），绕世界 Y 轴旋转
    float m_pitch = 0.f;  // 俯仰角（弧度），绕相机右轴旋转
};

#endif // RASTERCAMERA_H   // 头文件保护宏结束
