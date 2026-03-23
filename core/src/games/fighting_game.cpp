#include "games/fighting_game.hpp"
#include <cstring>
#include <iostream>

namespace platform
{

    // 构造函数
    FightingGame::FightingGame()
    {
        // 初始化角色位置
        characters_[0].x = -150; // 玩家1在左边
        characters_[1].x = 150;  // 玩家2在右边
        characters_[0].y = GROUND_Y;
        characters_[1].y = GROUND_Y;
        characters_[0].hp = 1000;
        characters_[1].hp = 1000;
    }

    // 初始化游戏
    void FightingGame::init(const GameState &initialState)
    {
        if (!initialState.data.empty())
        {
            // 从状态恢复
            deserializeState(initialState.data);
            currentFrame_ = initialState.frame;
        }
        else
        {
            // 重置到初始状态
            characters_[0].x = -150;
            characters_[1].x = 150;
            characters_[0].hp = 1000;
            characters_[1].hp = 1000;
            characters_[0].state = 0;
            characters_[1].state = 0;
            characters_[0].hitTimer = 0;
            characters_[1].hitTimer = 0;
            characters_[0].attackTimer = 0;
            characters_[1].attackTimer = 0;
            currentFrame_ = 0;
        }
        lastChecksum_ = calculateChecksum();
    }

    // 关闭游戏
    void FightingGame::shutdown()
    {
        // 清理资源（本例中无需特殊清理）
    }

    // 推进一帧
    void FightingGame::advanceFrame(const std::vector<Input> &playerInputs)
    {
        // 确保有足够的输入
        if (playerInputs.size() < 2)
        {
            return;
        }

        const auto &input1 = playerInputs[0]; // 玩家1输入
        const auto &input2 = playerInputs[1]; // 玩家2输入

        // 1. 应用玩家输入（只有不在硬直状态才能行动）
        if (characters_[0].canAct())
        {
            applyMove(characters_[0], input1, false);
        }
        else
        {
            // 减少硬直计时器
            if (characters_[0].hitTimer > 0)
                characters_[0].hitTimer--;
            if (characters_[0].attackTimer > 0)
                characters_[0].attackTimer--;
            if (characters_[0].blockTimer > 0)
                characters_[0].blockTimer--;

            // 攻击动画结束后回到站立状态
            if (characters_[0].attackTimer == 0 && characters_[0].state == 1)
            {
                characters_[0].state = 0;
            }
        }

        if (characters_[1].canAct())
        {
            applyMove(characters_[1], input2, true);
        }
        else
        {
            if (characters_[1].hitTimer > 0)
                characters_[1].hitTimer--;
            if (characters_[1].attackTimer > 0)
                characters_[1].attackTimer--;
            if (characters_[1].blockTimer > 0)
                characters_[1].blockTimer--;

            if (characters_[1].attackTimer == 0 && characters_[1].state == 1)
            {
                characters_[1].state = 0;
            }
        }

        // 2. 更新战斗逻辑（碰撞和伤害）
        updateCombat();

        // 3. 更新帧号
        currentFrame_++;

        // 4. 更新校验和（用于调试）
        lastChecksum_ = calculateChecksum();
    }

    // 应用移动/攻击指令
    void FightingGame::applyMove(Character &character, const Input &input, bool isPlayer2)
    {
        // 移动（使用摇杆或方向键）
        int moveDirection = 0;
        if (input.axis_x < -100)
            moveDirection = -1; // 向左
        if (input.axis_x > 100)
            moveDirection = 1; // 向右

        if (moveDirection != 0)
        {
            // 玩家2在右边，移动方向需要反向？不，直接用输入方向
            // 但为了简单，双方都按输入方向移动
            character.x += moveDirection * MOVE_SPEED;

            // 边界限制
            if (character.x > MAX_X)
                character.x = MAX_X;
            if (character.x < MIN_X)
                character.x = MIN_X;
        }

        // 攻击
        if (input.isPressed(FightingButtons::PUNCH) && character.attackTimer == 0)
        {
            character.state = 1; // 攻击状态
            character.attackTimer = ATTACK_DURATION;
        }

        // 防御
        if (input.isPressed(FightingButtons::BLOCK))
        {
            character.state = 3; // 防御状态
            character.blockTimer = BLOCK_DURATION;
        }
        else if (character.state == 3 && character.blockTimer == 0)
        {
            // 防御结束，回到站立
            character.state = 0;
        }
    }

    // 更新战斗逻辑（碰撞检测和伤害计算）
    void FightingGame::updateCombat()
    {
        // 检查两个角色是否靠近（距离小于 50）
        Fixed distance = characters_[0].x - characters_[1].x;
        if (distance < 0)
            distance = -distance;

        bool inRange = distance < 50;

        // 玩家1攻击玩家2
        if (inRange && characters_[0].attackTimer > 0 && characters_[1].hitTimer == 0)
        {
            // 检查防御
            if (characters_[1].state == 3)
            {
                // 防御成功，减少少量伤害
                Fixed damage = ATTACK_DAMAGE / 2;
                characters_[1].hp -= damage;
                if (characters_[1].hp < 0)
                    characters_[1].hp = 0;
                characters_[1].blockTimer = BLOCK_DURATION;
            }
            else
            {
                // 正常受伤
                Fixed damage = ATTACK_DAMAGE;
                characters_[1].hp -= damage;
                if (characters_[1].hp < 0)
                    characters_[1].hp = 0;
                characters_[1].state = 2; // 受伤状态
                characters_[1].hitTimer = HIT_DURATION;
            }
        }

        // 玩家2攻击玩家1
        if (inRange && characters_[1].attackTimer > 0 && characters_[0].hitTimer == 0)
        {
            if (characters_[0].state == 3)
            {
                Fixed damage = ATTACK_DAMAGE / 2;
                characters_[0].hp -= damage;
                if (characters_[0].hp < 0)
                    characters_[0].hp = 0;
                characters_[0].blockTimer = BLOCK_DURATION;
            }
            else
            {
                Fixed damage = ATTACK_DAMAGE;
                characters_[0].hp -= damage;
                if (characters_[0].hp < 0)
                    characters_[0].hp = 0;
                characters_[0].state = 2;
                characters_[0].hitTimer = HIT_DURATION;
            }
        }
    }

    // 计算伤害（简化版）
    Fixed FightingGame::calculateDamage(const Character &attacker, const Character &defender)
    {
        // 基础伤害
        Fixed baseDamage = ATTACK_DAMAGE;

        // 如果防御，伤害减半
        if (defender.state == 3)
        {
            baseDamage /= 2;
        }

        return baseDamage;
    }

    // 获取当前游戏状态
    GameState FightingGame::getCurrentState() const
    {
        GameState state;
        state.frame = currentFrame_;
        state.checksum = lastChecksum_;
        serializeState(state.data);
        return state;
    }

    // 设置游戏状态（用于回滚）
    void FightingGame::setState(const GameState &state)
    {
        if (!state.data.empty())
        {
            deserializeState(state.data);
            currentFrame_ = state.frame;
            lastChecksum_ = calculateChecksum();
        }
    }

    // 保存快照
    Snapshot FightingGame::saveSnapshot() const
    {
        Snapshot snapshot;
        snapshot.frame = currentFrame_;
        snapshot.state = getCurrentState();
        // 注意：inputs 需要由 RollbackEngine 填充，这里先留空
        return snapshot;
    }

    // 从快照恢复
    void FightingGame::restoreSnapshot(const Snapshot &snapshot)
    {
        setState(snapshot.state);
        // inputs 由 RollbackEngine 处理
    }

    // 获取渲染数据
    std::vector<uint8_t> FightingGame::getRenderData() const
    {
        std::vector<uint8_t> data;
        data.reserve(20); // 预分配空间

        // 玩家1数据
        int16_t p1x = static_cast<int16_t>(characters_[0].x / 10); // 转换为整数坐标
        int16_t p1y = static_cast<int16_t>(characters_[0].y / 10);
        uint16_t p1hp = static_cast<uint16_t>(characters_[0].hp / 10); // 0-100
        uint8_t p1state = characters_[0].state;

        // 玩家2数据
        int16_t p2x = static_cast<int16_t>(characters_[1].x / 10);
        int16_t p2y = static_cast<int16_t>(characters_[1].y / 10);
        uint16_t p2hp = static_cast<uint16_t>(characters_[1].hp / 10);
        uint8_t p2state = characters_[1].state;

        // 序列化到字节数组
        data.push_back(p1x & 0xFF);
        data.push_back((p1x >> 8) & 0xFF);
        data.push_back(p1y & 0xFF);
        data.push_back((p1y >> 8) & 0xFF);
        data.push_back(p1hp & 0xFF);
        data.push_back((p1hp >> 8) & 0xFF);
        data.push_back(p1state);

        data.push_back(p2x & 0xFF);
        data.push_back((p2x >> 8) & 0xFF);
        data.push_back(p2y & 0xFF);
        data.push_back((p2y >> 8) & 0xFF);
        data.push_back(p2hp & 0xFF);
        data.push_back((p2hp >> 8) & 0xFF);
        data.push_back(p2state);

        return data;
    }

    // 验证状态
    bool FightingGame::validateState(const GameState &state) const
    {
        // 简单验证：检查血量是否在合理范围
        if (state.data.size() < 20)
            return false;

        // 可以添加更多验证逻辑
        return true;
    }

    // 调试输出
    void FightingGame::debugPrint() const
    {
        std::cout << "=== Fighting Game Debug ===" << std::endl;
        std::cout << "Frame: " << currentFrame_ << std::endl;
        std::cout << "Player1: x=" << characters_[0].x
                  << " hp=" << characters_[0].hp
                  << " state=" << (int)characters_[0].state << std::endl;
        std::cout << "Player2: x=" << characters_[1].x
                  << " hp=" << characters_[1].hp
                  << " state=" << (int)characters_[1].state << std::endl;
        std::cout << "Checksum: " << lastChecksum_ << std::endl;
    }

    // 计算校验和（简化版）
    uint32_t FightingGame::calculateChecksum() const
    {
        uint32_t sum = currentFrame_;
        for (int i = 0; i < 2; i++)
        {
            sum += static_cast<uint32_t>(characters_[i].x);
            sum += static_cast<uint32_t>(characters_[i].y);
            sum += static_cast<uint32_t>(characters_[i].hp);
            sum += characters_[i].state;
            sum += characters_[i].hitTimer;
            sum += characters_[i].attackTimer;
        }
        return sum;
    }

    // 序列化状态到字节数组
    void FightingGame::serializeState(std::vector<uint8_t> &buffer) const
    {
        buffer.clear();
        buffer.reserve(40);

        for (int i = 0; i < 2; i++)
        {
            // 序列化每个角色的数据
            int32_t x = characters_[i].x;
            int32_t y = characters_[i].y;
            int32_t hp = characters_[i].hp;

            buffer.push_back(x & 0xFF);
            buffer.push_back((x >> 8) & 0xFF);
            buffer.push_back((x >> 16) & 0xFF);
            buffer.push_back((x >> 24) & 0xFF);

            buffer.push_back(y & 0xFF);
            buffer.push_back((y >> 8) & 0xFF);
            buffer.push_back((y >> 16) & 0xFF);
            buffer.push_back((y >> 24) & 0xFF);

            buffer.push_back(hp & 0xFF);
            buffer.push_back((hp >> 8) & 0xFF);
            buffer.push_back((hp >> 16) & 0xFF);
            buffer.push_back((hp >> 24) & 0xFF);

            buffer.push_back(characters_[i].state);
            buffer.push_back(characters_[i].hitTimer);
            buffer.push_back(characters_[i].attackTimer);
            buffer.push_back(characters_[i].blockTimer);
        }
    }

    // 从字节数组反序列化状态
    void FightingGame::deserializeState(const std::vector<uint8_t> &buffer)
    {
        if (buffer.size() < 40)
            return;

        size_t pos = 0;
        for (int i = 0; i < 2; i++)
        {
            int32_t x = buffer[pos++] | (buffer[pos++] << 8) | (buffer[pos++] << 16) | (buffer[pos++] << 24);
            int32_t y = buffer[pos++] | (buffer[pos++] << 8) | (buffer[pos++] << 16) | (buffer[pos++] << 24);
            int32_t hp = buffer[pos++] | (buffer[pos++] << 8) | (buffer[pos++] << 16) | (buffer[pos++] << 24);

            characters_[i].x = x;
            characters_[i].y = y;
            characters_[i].hp = hp;
            characters_[i].state = buffer[pos++];
            characters_[i].hitTimer = buffer[pos++];
            characters_[i].attackTimer = buffer[pos++];
            characters_[i].blockTimer = buffer[pos++];
        }
    }

} // namespace platform