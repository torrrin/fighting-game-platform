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
    // 注意：buttons 的每一位代表什么按钮，由具体游戏自己定义
    // 平台层只负责传递这些位掩码，不解释其含义
    struct Input
    {
        uint32_t buttons = 0; // 按钮状态（位掩码），最多支持 32 个按钮
        int16_t axis_x = 0;   // 左摇杆/方向键 X 轴 (-32768 到 32767)
        int16_t axis_y = 0;   // 左摇杆/方向键 Y 轴
        int16_t axis_rx = 0;  // 右摇杆 X 轴（可选，用于需要右摇杆的游戏）
        int16_t axis_ry = 0;  // 右摇杆 Y 轴

        // 检查某个按钮是否被按下
        // 参数 buttonMask: 游戏自己定义的按钮位掩码
        bool isPressed(uint32_t buttonMask) const
        {
            return (buttons & buttonMask) != 0;
        }

        // 按下按钮
        void press(uint32_t buttonMask)
        {
            buttons |= buttonMask;
        }

        // 释放按钮
        void release(uint32_t buttonMask)
        {
            buttons &= ~buttonMask;
        }

        // 判断两个输入是否相等
        bool operator==(const Input &other) const
        {
            return buttons == other.buttons &&
                   axis_x == other.axis_x &&
                   axis_y == other.axis_y &&
                   axis_rx == other.axis_rx &&
                   axis_ry == other.axis_ry;
        }

        // 判断两个输入是否不相等
        bool operator!=(const Input &other) const
        {
            return !(*this == other);
        }

        // 序列化（用于网络传输）
        // 将 Input 结构打包成字节数组
        void serialize(uint8_t *buffer) const
        {
            buffer[0] = buttons & 0xFF;
            buffer[1] = (buttons >> 8) & 0xFF;
            buffer[2] = (buttons >> 16) & 0xFF;
            buffer[3] = (buttons >> 24) & 0xFF;
            buffer[4] = axis_x & 0xFF;
            buffer[5] = (axis_x >> 8) & 0xFF;
            buffer[6] = axis_y & 0xFF;
            buffer[7] = (axis_y >> 8) & 0xFF;
            buffer[8] = axis_rx & 0xFF;
            buffer[9] = (axis_rx >> 8) & 0xFF;
            buffer[10] = axis_ry & 0xFF;
            buffer[11] = (axis_ry >> 8) & 0xFF;
        }

        // 反序列化
        void deserialize(const uint8_t *buffer)
        {
            buttons = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
            axis_x = buffer[4] | (buffer[5] << 8);
            axis_y = buffer[6] | (buffer[7] << 8);
            axis_rx = buffer[8] | (buffer[9] << 8);
            axis_ry = buffer[10] | (buffer[11] << 8);
        }
    };

    // 游戏状态
    // 存储游戏的完整状态，具体内容由游戏自己定义
    struct GameState
    {
        uint32_t frame = 0;        // 当前帧号
        uint32_t checksum = 0;     // 校验和，用于验证同步
        std::vector<uint8_t> data; // 游戏特定的状态数据

        // 计算校验和（CRC32 简化版，后续实现）
        uint32_t computeChecksum() const;

        // 判断两个状态是否相等
        bool operator==(const GameState &other) const
        {
            return frame == other.frame &&
                   checksum == other.checksum &&
                   data == other.data;
        }
    };

    // 快照（用于回滚）
    // 保存某一帧的完整状态和该帧的输入
    struct Snapshot
    {
        uint32_t frame = 0;        // 这一帧的编号
        GameState state;           // 游戏状态
        std::vector<Input> inputs; // 所有玩家在这一帧的输入
    };

} // namespace platform