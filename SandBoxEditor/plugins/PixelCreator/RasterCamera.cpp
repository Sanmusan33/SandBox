#include "RasterCamera.h"   // 引入相机类声明（成员已在头文件中就地初始化）

RasterCamera::RasterCamera() = default;   // 默认构造函数实现：无额外初始化逻辑

void RasterCamera::orbit(float dx, float dy)  // 实现拖拽增量旋转视角
{
    const float sensitivity = 0.005f;          // 灵敏度常数：像素位移到弧度增量的换算系数
    m_yaw   += dx * sensitivity;               // 水平拖动更新偏航角（右拖动 => 视角左转）
    m_pitch -= dy * sensitivity;               // 垂直拖动更新俯仰角（下拖动 => 视角下转）

    constexpr float kPitchLimit = 1.553f;      // 俯仰角限制（约 89 度），防止视线与 up 轴共线导致退化
    if (m_pitch > kPitchLimit)  m_pitch = kPitchLimit;   // 俯仰角超过上限则钳制到上限
    if (m_pitch < -kPitchLimit) m_pitch = -kPitchLimit;  // 俯仰角低于下限则钳制到下限
}

void RasterCamera::moveForward(float amount) { m_cameraPos += forward() * amount; }      // 位置沿视线方向平移（正=前进、负=后退）
void RasterCamera::moveRight(float amount)   { m_cameraPos += right() * amount; }        // 位置沿右向平移（正=右移、负=左移）
void RasterCamera::moveUp(float amount)      { m_cameraPos += RasterMath::Vec3{0.f, 1.f, 0.f} * amount; }  // 位置沿世界 Y 轴升降

void RasterCamera::setPosition(const RasterMath::Vec3& pos) { m_cameraPos = pos; }       // 直接覆写相机位置

RasterMath::Vec3 RasterCamera::forward() const   // 由 yaw/pitch 计算前向单位向量
{
    const float cp = std::cos(m_pitch), sp = std::sin(m_pitch);  // 计算俯仰角的余弦与正弦
    const float cy = std::cos(m_yaw),   sy = std::sin(m_yaw);    // 计算偏航角的余弦与正弦
    // 右手系，yaw=0/pitch=0 时朝向 -Z（OpenGL 相机惯例）
    return {cp * sy, sp, -cp * cy};   // 球面坐标转笛卡尔坐标：forward = (cosP·sinY, sinP, -cosP·cosY)
}

RasterMath::Vec3 RasterCamera::right() const     // 计算右向单位向量
{
    return RasterMath::normalize(RasterMath::cross(
        forward(), RasterMath::Vec3{0.f, 1.f, 0.f}));   // 前向 叉乘 世界Y = 右向，再归一化
}

RasterMath::Mat4 RasterCamera::viewMatrix() const   // 生成相机视图矩阵
{
    return RasterMath::Mat4::lookAt(m_cameraPos, m_cameraPos + forward(), RasterMath::Vec3{0.f, 1.f, 0.f});  // 以位置为原点、位置+前向为注视点、世界Y为上方向构造
}
