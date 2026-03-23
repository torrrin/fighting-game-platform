#pragma once
#include "types.hpp"
#include <memory>
#include <vector>
#include <string>

namespace platform
{

    // 游戏接口
    // 所有要接入平台的游戏都必须实现这个接口
    class IGame
    {
    public:
        // 虚析构函数，确保派生类正确释放资源
        virtual ~IGame() = default;

        // ----- 生命周期管理 -----

        // 初始化游戏，从给定的初始状态开始
        // 参数 initialState: 游戏的初始状态（比如新对局的初始数据）
        virtual void init(const GameState &initialState) = 0;

        // 关闭游戏，释放资源
        virtual void shutdown() = 0;

        // ----- 核心模拟 -----

        // 推进一帧游戏逻辑
        // 参数 playerInputs: 所有玩家在这一帧的输入
        // 例如：playerInputs[0] 是玩家1的输入，playerInputs[1] 是玩家2的输入
        virtual void advanceFrame(const std::vector<Input> &playerInputs) = 0;

        // 获取当前游戏状态
        // 返回值: 当前的 GameState，包含帧号、校验和、状态数据
        virtual GameState getCurrentState() const = 0;

        // 设置游戏状态（用于回滚后恢复）
        // 参数 state: 要恢复到的状态
        virtual void setState(const GameState &state) = 0;

        // ----- 快照支持（用于回滚）-----

        // 保存当前状态的快照
        // 返回值: 包含当前帧号、状态、所有玩家输入的 Snapshot
        virtual Snapshot saveSnapshot() const = 0;

        // 从快照恢复状态
        // 参数 snapshot: 之前保存的快照
        virtual void restoreSnapshot(const Snapshot &snapshot) = 0;

        // ----- 渲染 -----

        // 获取当前帧的渲染数据
        // 返回值: 字节数组，具体格式由游戏自己定义
        // 前端拿到这个数据后，根据游戏类型来解析和绘制
        virtual std::vector<uint8_t> getRenderData() const = 0;

        // ----- 游戏属性 -----

        // 获取游戏支持的最大玩家数
        // 返回值: 玩家数量，例如格斗游戏通常是 2
        virtual int getMaxPlayers() const = 0;

        // 获取游戏的逻辑帧率（tickrate）
        // 返回值: 每秒帧数，例如 60 表示 60 帧/秒
        virtual int getTickRate() const = 0;

        // 获取游戏名称
        // 返回值: 游戏名称字符串
        virtual std::string getGameName() const = 0;

        // ----- 验证 -----

        // 验证游戏状态是否有效
        // 参数 state: 要验证的状态
        // 返回值: true 表示状态有效，false 表示状态损坏或作弊
        virtual bool validateState(const GameState &state) const = 0;
    };

} // namespace platform