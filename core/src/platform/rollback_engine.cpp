#include "platform/rollback_engine.hpp"
#include <iostream>
#include <algorithm>

namespace platform
{

    RollbackEngine::RollbackEngine(std::unique_ptr<IGame> game, int maxPlayers)
        : game_(std::move(game)), maxPlayers_(maxPlayers), playerInputs_(maxPlayers)
    {

        // 初始化游戏
        GameState initState;
        game_->init(initState);

        // 保存初始快照
        saveSnapshot();
    }

    void RollbackEngine::setLocalInput(const Input &input)
    {
        localInput_ = input;
    }

    void RollbackEngine::receiveRemoteInput(int playerId, uint32_t frame, const Input &input)
    {
        // 安全检查
        if (playerId < 0 || playerId >= maxPlayers_)
            return;

        auto &queue = playerInputs_[playerId];

        // 检查是否已经存在这个输入
        if (queue.confirmed.find(frame) != queue.confirmed.end())
        {
            return; // 已经收到过了
        }

        // 存储已确认的输入
        queue.confirmed[frame] = input;

        // 检查是否需要回滚
        if (frame < currentFrame_)
        {
            // 收到了一个比当前帧更早的输入！说明我们之前猜错了
            std::cout << "Rollback triggered! Received input for frame " << frame
                      << " but current frame is " << currentFrame_ << std::endl;
            rollbackCount_++;
            rollbackToFrame(frame);
        }
    }

    void RollbackEngine::update()
    {
        // 1. 保存当前帧快照（用于未来可能的回滚）
        saveSnapshot();

        // 2. 收集当前帧所有玩家的输入
        auto inputs = getFrameInputs(currentFrame_);

        // 3. 推进游戏一帧
        game_->advanceFrame(inputs);

        // 4. 帧号前进
        currentFrame_++;
    }

    GameState RollbackEngine::getCurrentState() const
    {
        return game_->getCurrentState();
    }

    std::vector<uint8_t> RollbackEngine::getRenderData() const
    {
        return game_->getRenderData();
    }

    void RollbackEngine::debugPrint() const
    {
        std::cout << "=== RollbackEngine Debug ===" << std::endl;
        std::cout << "Current Frame: " << currentFrame_ << std::endl;
        std::cout << "History Size: " << snapshotHistory_.size() << std::endl;
        std::cout << "Rollback Count: " << rollbackCount_ << std::endl;

        // 打印每个玩家的输入队列状态
        for (int i = 0; i < maxPlayers_; i++)
        {
            std::cout << "Player " << i << " confirmed inputs: "
                      << playerInputs_[i].confirmed.size() << std::endl;
        }

        game_->debugPrint();
    }

    void RollbackEngine::saveSnapshot()
    {
        Snapshot snapshot;
        snapshot.frame = currentFrame_;
        snapshot.state = game_->getCurrentState();
        // 注意：inputs 暂时不保存，因为恢复时可以从输入队列重新获取
        // 如果需要更精确的回滚，可以在这里保存 inputs

        snapshotHistory_.push_back(snapshot);

        // 清理过旧的快照
        cleanupOldSnapshots();
    }

    void RollbackEngine::rollbackToFrame(uint32_t targetFrame)
    {
        // 1. 找到目标帧的快照
        Snapshot targetSnapshot = findSnapshot(targetFrame);

        // 2. 恢复游戏状态
        game_->restoreSnapshot(targetSnapshot);

        // 3. 重置当前帧号
        currentFrame_ = targetFrame;

        // 4. 从目标帧开始，用正确的输入重新模拟到原本的帧
        resimulateFromFrame(targetFrame);
    }

    void RollbackEngine::resimulateFromFrame(uint32_t startFrame)
    {
        // 从 startFrame 开始，一直模拟到 currentFrame_（模拟过程中 currentFrame_ 会增加）
        // 注意：这里需要小心处理，因为模拟过程中可能会再次触发回滚
        // 简化处理：只模拟一次

        uint32_t targetFrame = currentFrame_; // 目标帧号（回滚前的帧号）

        for (uint32_t frame = startFrame; frame < targetFrame; frame++)
        {
            // 获取这一帧的输入（现在都是确认的）
            auto inputs = getFrameInputs(frame);

            // 推进游戏
            game_->advanceFrame(inputs);

            // 更新当前帧号
            currentFrame_ = frame + 1;
        }
    }

    std::vector<Input> RollbackEngine::getFrameInputs(uint32_t frame)
    {
        std::vector<Input> inputs(maxPlayers_);

        for (int playerId = 0; playerId < maxPlayers_; playerId++)
        {
            auto &queue = playerInputs_[playerId];

            // 查找是否有确认的输入
            auto it = queue.confirmed.find(frame);
            if (it != queue.confirmed.end())
            {
                inputs[playerId] = it->second;
                queue.lastPredicted = it->second; // 更新预测基准
            }
            else
            {
                // 没有确认的输入，使用预测
                inputs[playerId] = predictInput(playerId, frame);
            }
        }

        return inputs;
    }

    Input RollbackEngine::predictInput(int playerId, uint32_t frame)
    {
        // 简化预测策略：重复上一次的输入
        // 更复杂的策略可以考虑保持输入不变或使用移动平均
        return playerInputs_[playerId].lastPredicted;
    }

    Snapshot RollbackEngine::findSnapshot(uint32_t frame) const
    {
        // 在历史中查找
        for (const auto &snapshot : snapshotHistory_)
        {
            if (snapshot.frame == frame)
            {
                return snapshot;
            }
        }

        // 如果没找到，返回第一个快照（最旧的）
        if (!snapshotHistory_.empty())
        {
            std::cerr << "Warning: Snapshot for frame " << frame << " not found. Using oldest." << std::endl;
            return snapshotHistory_.front();
        }

        // 空历史，返回空快照（不应该发生）
        std::cerr << "Error: No snapshots available!" << std::endl;
        return Snapshot{};
    }

    void RollbackEngine::cleanupOldSnapshots()
    {
        // 保留最近 MAX_HISTORY 个快照
        while (snapshotHistory_.size() > MAX_HISTORY)
        {
            snapshotHistory_.pop_front();
        }
    }

} // namespace platform