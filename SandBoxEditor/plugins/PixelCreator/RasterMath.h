#ifndef RASTERMATH_H                     // 头文件保护宏开始：防止同一头文件被重复包含
#define RASTERMATH_H                     // 定义保护宏：首次包含后即为"已定义"状态

// 轻量自包含数学库（header-only）
// 列主序 float[16]，与 GLM/OpenGL 惯例一致；变换约定 v' = M * v（列向量）
#include <cmath>                         // 引入数学函数库（sqrt/cos/sin/tan 等）

namespace RasterMath                     // 自定义命名空间：避免与全局命名空间符号冲突
{

struct Vec3                              // 三维向量结构体：表示三维空间中的点或方向
{
    float x = 0.f, y = 0.f, z = 0.f;     // 三个分量（x/y/z），默认初始化为 0

    Vec3() = default;                                                    // 默认构造：生成零向量
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}          // 带参构造：按实参初始化三分量

    Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }  // 向量加法：逐分量相加
    Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }  // 向量减法：逐分量相减
    Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }              // 标量乘法：每个分量乘以标量
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }  // 复合加法：原地累加
    Vec3& operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }  // 复合减法：原地累减
};

inline float dot(const Vec3& a, const Vec3& b)   // 点积函数：各分量相乘后求和
{
    return a.x * b.x + a.y * b.y + a.z * b.z;    // 返回点积标量结果
}

inline Vec3 cross(const Vec3& a, const Vec3& b)  // 叉积函数：结果向量垂直于 a、b 所在平面
{
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};  // 按叉积公式计算三个分量
}

inline Vec3 normalize(const Vec3& v)   // 归一化函数：将任意向量缩放为单位长度（模长=1）
{
    float len = std::sqrt(dot(v, v));  // 计算模长：向量与自身点积后开方
    return (len > 1e-8f) ? v * (1.f / len) : Vec3{0.f, 0.f, 0.f};  // 模长非零则除以模长；近零向量返回零向量（防止除零）
}

struct Mat4                              // 4x4 矩阵结构体：用于 3D 图形的各类变换 {x,y,z,w}
{

    /** 
    内存:  m[0] m[1] m[2] m[3] | m[4] m[5] m[6] m[7] | m[8] m[9] m[10] m[11] | m[12] m[13] m[14] m[15]
        |-------- 第0列 -------|------- 第1列 --------|------- 第2列 ---------|------- 第3列 ----------|
    */
    float m[16];                         // 以 16 个 float 存储矩阵，列主序：m[列*4+行]

    static Mat4 identity()               // 构造单位矩阵：对角线为 1、其余为 0
    {
        Mat4 r{};                        // 全零初始化结果矩阵
        for (int i = 0; i < 4; ++i)      // 遍历主对角线（行号==列号）
            r.m[i * 4 + i] = 1.f;        // 对角线元素置 1
        return r;                        // 返回单位矩阵
    }

    // 透视投影（右手系，NDC z in [-1,1]）
    static Mat4 perspective(float fovYRad, float aspect, float znear, float zfar)  // 构造透视投影矩阵：参数=垂直视场角(弧度)/宽高比/近平面/远平面
    {
        Mat4 r{};                                              // 全零初始化结果矩阵
        float f = 1.f / std::tan(fovYRad * 0.5f);              // 焦距因子 f = 1/tan(fov/2)，控制视场缩放
        r.m[0] = f / aspect;                                   // X 方向缩放系数（考虑宽高比，防止拉伸变形）
        r.m[5] = f;                                            // Y 方向缩放系数
        r.m[10] = (zfar + znear) / (znear - zfar);             // Z 映射系数：把 [near,far] 映射到 NDC [-1,1]
        r.m[11] = -1.f;                                        // W 分量系数为 -z：为透视除法提供齐次分母
        r.m[14] = (2.f * zfar * znear) / (znear - zfar);       // Z 平移项：修正深度映射范围
        return r;                                              // 返回透视投影矩阵
    }

    // 视图矩阵：由 eye/center/up 构造相机坐标系
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up)  // 构造相机视图矩阵：参数=相机位置/注视点/上方向
    {
        Vec3 f = normalize(center - eye);  // 前向基向量：相机"看向"的单位方向
        Vec3 s = normalize(cross(f, up));  // 右向基向量：前向与上方向叉积（与两者均正交）
        Vec3 u = cross(s, f);              // 上向基向量：右向与前向叉积（重正交化，保证正交坐标系）

        Mat4 r = identity();               // 先取单位矩阵，再填充旋转与平移分量
        r.m[0] = s.x;  r.m[4] = s.y;  r.m[8]  = s.z;   // 旋转部分第 1 行 = 右向量
        r.m[1] = u.x;  r.m[5] = u.y;  r.m[9]  = u.z;   // 旋转部分第 2 行 = 上向量
        r.m[2] = -f.x; r.m[6] = -f.y; r.m[10] = -f.z;  // 旋转部分第 3 行 = 反向前向量（相机默认看向 -Z）
        r.m[12] = -dot(s, eye);            // 平移项 X：负的右向量·相机位置
        r.m[13] = -dot(u, eye);            // 平移项 Y：负的上向量·相机位置
        r.m[14] = dot(f, eye);             // 平移项 Z：前向量·相机位置
        return r;                          // 返回视图矩阵
    }

    // 矩阵乘法（列主序）：(A*B)[c][r] = sum_k A[k][r] * B[c][k]
    Mat4 operator*(const Mat4& o) const    // 重载矩阵乘法：本矩阵 × 传入矩阵
    {
        Mat4 r{};                          // 全零初始化结果矩阵
        // 第0列
        r.m[0] = m[0] * o.m[0] + m[4] * o.m[1] + m[8] * o.m[2] + m[12] * o.m[3];
        r.m[1] = m[1] * o.m[0] + m[5] * o.m[1] + m[9] * o.m[2] + m[13] * o.m[3];
        r.m[2] = m[2] * o.m[0] + m[6] * o.m[1] + m[10] * o.m[2] + m[14] * o.m[3];
        r.m[3] = m[3] * o.m[0] + m[7] * o.m[1] + m[11] * o.m[2] + m[15] * o.m[3];

        // 第1列
        r.m[4] = m[0] * o.m[4] + m[4] * o.m[5] + m[8] * o.m[6] + m[12] * o.m[7];
        r.m[5] = m[1] * o.m[4] + m[5] * o.m[5] + m[9] * o.m[6] + m[13] * o.m[7];
        r.m[6] = m[2] * o.m[4] + m[6] * o.m[5] + m[10] * o.m[6] + m[14] * o.m[7];
        r.m[7] = m[3] * o.m[4] + m[7] * o.m[5] + m[11] * o.m[6] + m[15] * o.m[7];

        // 第2列
        r.m[8]  = m[0] * o.m[8]  + m[4] * o.m[9]  + m[8] * o.m[10]  + m[12] * o.m[11];
        r.m[9]  = m[1] * o.m[8]  + m[5] * o.m[9]  + m[9] * o.m[10]  + m[13] * o.m[11];
        r.m[10] = m[2] * o.m[8]  + m[6] * o.m[9]  + m[10] * o.m[10] + m[14] * o.m[11];
        r.m[11] = m[3] * o.m[8]  + m[7] * o.m[9]  + m[11] * o.m[10] + m[15] * o.m[11];

        // 第3列
        r.m[12] = m[0] * o.m[12] + m[4] * o.m[13] + m[8] * o.m[14] + m[12] * o.m[15];
        r.m[13] = m[1] * o.m[12] + m[5] * o.m[13] + m[9] * o.m[14] + m[13] * o.m[15];
        r.m[14] = m[2] * o.m[12] + m[6] * o.m[13] + m[10] * o.m[14] + m[14] * o.m[15];
        r.m[15] = m[3] * o.m[12] + m[7] * o.m[13] + m[11] * o.m[14] + m[15] * o.m[15];

        return r;                          // 返回乘积矩阵
    }

    // 变换点（w=1）：返回 (clipX, clipY, clipZ, clipW)
    void transform(const Vec3& v, float& cx, float& cy, float& cz, float& cw) const  // 执行齐次变换：输入三维点，输出裁剪空间四分量
    {
        cx = m[0] * v.x + m[4] * v.y + m[8]  * v.z + m[12];   // 变换后 X = 矩阵第 0 行点积向量 + 平移项
        cy = m[1] * v.x + m[5] * v.y + m[9]  * v.z + m[13];   // 变换后 Y = 矩阵第 1 行点积向量 + 平移项
        cz = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];   // 变换后 Z = 矩阵第 2 行点积向量 + 平移项
        cw = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15];   // 变换后 W = 矩阵第 3 行点积向量 + 平移项（透视时含 -z）
    }
};

} // namespace RasterMath                // 命名空间结束

#endif // RASTERMATH_H                   // 头文件保护宏结束
