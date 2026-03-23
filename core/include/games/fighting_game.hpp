#pragma once
#include "platform/igame.hpp"
#include <array>
#include <cstdint>

namespace platform
{

    // 格斗游戏自己的按钮定义
    namespace FightingButtons
    {
        constexpr uint32_t PUNCH = 1 << 0; // 出拳
        constexpr uint32_t KICK = 1 << 1;  // 踢腿
        constexpr uint32_t BLOCK = 1 << 2; // 防御
    }

    // 简单的格斗游戏示例
    // 实现 IGame 接口，包含两个角色的对战逻辑
    class FightingGame : public IGame
    {
    public:
        FightingGame();
        ~FightingGame() override = default;

        // ----- IGame 接口实现 -----

        // 生命周期
        void init(const GameState &initialState) override;
        void shutdown() override;

        // 核心模拟
        void advanceFrame(const std::vector<Input> &playerInputs) override;
        GameState getCurrentState() const override;
        void setState(const GameState &state) override;

        // 快照支持
        Snapshot saveSnapshot() const override;
        void restoreSnapshot(const Snapshot &snapshot) override;

        // 渲染
        std::vector<uint8_t> getRenderData() const override;

        // 游戏属性
        int getMaxPlayers() const override { return 2; }
        int getTickRate() const override { return 60; }
        std::string getGameName() const override { return "Fighting Game"; }

        // 验证
        bool validateState(const GameState &state) const override;

        // 调试
        void debugPrint() const override;

    private:
        // 角色状态结构
        struct Character
        {
            Fixed x = 0;             // X 坐标（定点数）
            Fixed y = 0;             // Y 坐标（地面高度为 0）
            Fixed hp = 1000;         // 生命值（1000 = 100%）
            uint8_t state = 0;       // 状态：0=站立, 1=攻击, 2=受伤, 3=防御
            uint8_t hitTimer = 0;    // 受击硬直剩余帧数
            uint8_t attackTimer = 0; // 攻击动画剩余帧数
            uint8_t blockTimer = 0;  // 防御硬直剩余帧数

            // 辅助方法
            bool isAlive() const { return hp > 0; }
            bool canAct() const { return hitTimer == 0 && attackTimer == 0 && blockTimer == 0; }
        };

        // 游戏数据
        std::array<Character, 2> characters_;
        uint32_t currentFrame_ = 0;
        uint32_t lastChecksum_ = 0;

        // 游戏常量
        static constexpr Fixed GROUND_Y = 0;
        static constexpr Fixed MOVE_SPEED = 5;      // 每帧移动速度
        static constexpr Fixed ATTACK_DAMAGE = 100; // 攻击伤害（10%）
        static constexpr int ATTACK_DURATION = 10;  // 攻击动画持续帧数
        static constexpr int HIT_DURATION = 15;     // 受击硬直帧数
        static constexpr int BLOCK_DURATION = 8;    // 防御硬直帧数
        static constexpr Fixed MAX_X = 300;         // 左边界
        static constexpr Fixed MIN_X = -300;        // 右边界

        // 私有方法
        void applyMove(Character &character, const Input &input, bool isPlayer2);
        void updateCombat();
        Fixed calculateDamage(const Character &attacker, const Character &defender);
        uint32_t calculateChecksum() const;
        void serializeState(std::vector<uint8_t> &buffer) const;
        void deserializeState(const std::vector<uint8_t> &buffer);
    };

} // namespace platform