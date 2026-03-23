#pragma once
#include "igame.hpp"
#include <deque>
#include <unordered_map>
#include <vector>
#include <memory>

namespace platform
{

    // 每个玩家的输入队列
    struct PlayerInputQueue
    {
        // 已确认的输入（从网络收到的）
        std::unordered_map<uint32_t, Input> confirmed;
        // 上一次预测的输入（用于当没有确认输入时）
        Input lastPredicted;
    };

    // Rollback 引擎
    // 负责管理输入队列、快照历史，执行回滚和重模拟
    class RollbackEngine
    {
    public:
        // 构造函数
        // game: 游戏实例（已实现 IGame 接口）
        // maxPlayers: 最大玩家数
        RollbackEngine(std::unique_ptr<IGame> game, int maxPlayers);

        // ----- 输入处理 -----
        // 设置本地玩家的输入（每帧调用）
        void setLocalInput(const Input &input);

        // 收到远程玩家的输入（从网络接收）
        // playerId: 玩家编号（0 或 1）
        // frame: 输入对应的帧号
        // input: 输入数据
        void receiveRemoteInput(int playerId, uint32_t frame, const Input &input);

        // ----- 游戏循环 -----
        // 推进一帧（由游戏循环每帧调用）
        void update();

        // ----- 状态访问 -----
        // 获取当前游戏状态
        GameState getCurrentState() const;

        // 获取当前渲染数据
        std::vector<uint8_t> getRenderData() const;

        // ----- 调试 -----
        int getLastRollbackCount() const { return rollbackCount_; }
        void debugPrint() const;

    private:
        // 游戏实例
        std::unique_ptr<IGame> game_;

        // 玩家数量
        int maxPlayers_;

        // 当前帧号
        uint32_t currentFrame_{0};

        // 每个玩家的输入队列
        std::vector<PlayerInputQueue> playerInputs_;

        // 当前帧的本地输入（待发送）
        Input localInput_;

        // 快照历史（环形队列）
        std::deque<Snapshot> snapshotHistory_;
        static constexpr int MAX_HISTORY = 120; // 保留 2 秒（60帧/秒）

        // 统计
        int rollbackCount_{0};

        // ----- 私有方法 -----
        // 保存当前帧快照
        void saveSnapshot();

        // 回滚到指定帧
        void rollbackToFrame(uint32_t targetFrame);

        // 从指定帧开始重新模拟到当前帧
        void resimulateFromFrame(uint32_t startFrame);

        // 获取某一帧所有玩家的输入
        // 优先使用已确认的输入，否则使用预测
        std::vector<Input> getFrameInputs(uint32_t frame);

        // 预测某个玩家的输入（当没有确认输入时）
        Input predictInput(int playerId, uint32_t frame);

        // 在历史中查找指定帧的快照
        Snapshot findSnapshot(uint32_t frame) const;

        // 清理过旧的快照
        void cleanupOldSnapshots();
    };

} // namespace platform