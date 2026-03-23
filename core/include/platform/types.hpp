#pragma once
#include <cstdint>
#include <vector>

namespace platform
{

    // 定点数类型（用整数模拟小数，避免浮点数精度问题）
    // 1000 表示 1.000，可以精确到小数点后3位
    using Fixed = int32_t;
    constexpr Fixed FIXED_SCALE = 1000;

    // 玩家输入
    struct Input
    {
        uint16_t buttons = 0; // 按钮状态（位掩码）
        int16_t axis_x = 0;   // 水平轴 (-32768 到 32767)
        int16_t axis_y = 0;   // 垂直轴

        // 检查某个按钮是否被按下
        bool isPressed(uint16_t buttonMask) const
        {
            return (buttons & buttonMask) != 0;
        }

        // 判断两个输入是否相等
        bool operator==(const Input &other) const
        {
            return buttons == other.buttons &&
                   axis_x == other.axis_x &&
                   axis_y == other.axis_y;
        }

        // 判断两个输入是否不相等
        bool operator!=(const Input &other) const
        {
            return !(*this == other);
        }
    };

    // 游戏状态
    struct GameState
    {
        uint32_t frame = 0;        // 当前帧号
        uint32_t checksum = 0;     // 校验和，用于验证同步
        std::vector<uint8_t> data; // 实际的状态数据

        // 判断两个状态是否相等
        bool operator==(const GameState &other) const
        {
            return frame == other.frame &&
                   checksum == other.checksum &&
                   data == other.data;
        }
    };

    // 快照（用于回滚）
    struct Snapshot
    {
        uint32_t frame = 0;        // 这一帧的编号
        GameState state;           // 游戏状态
        std::vector<Input> inputs; // 所有玩家的输入
    };

    // 常用按钮的位掩码定义
    namespace Button
    {
        constexpr uint16_t UP = 1 << 0;     // 上
        constexpr uint16_t DOWN = 1 << 1;   // 下
        constexpr uint16_t LEFT = 1 << 2;   // 左
        constexpr uint16_t RIGHT = 1 << 3;  // 右
        constexpr uint16_t ATTACK = 1 << 4; // 攻击
        constexpr uint16_t JUMP = 1 << 5;   // 跳跃
        constexpr uint16_t BLOCK = 1 << 6;  // 防御
    }

} // namespace platform