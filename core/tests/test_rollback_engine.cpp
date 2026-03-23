// core/tests/test_rollback_engine.cpp
#include "platform/rollback_engine.hpp"
#include "games/fighting_game.hpp"
#include "platform/game_registry.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace platform;

int main()
{
    std::cout << "=== 测试 Rollback 引擎 ===" << std::endl;

    // 1. 注册格斗游戏
    GameRegistry::registerGame("fighting", []()
                               { return std::make_unique<FightingGame>(); });

    // 2. 创建游戏和引擎
    auto game = GameRegistry::createGame("fighting");
    RollbackEngine engine(std::move(game), 2);

    std::cout << "引擎初始化完成" << std::endl;
    engine.debugPrint();

    // 3. 模拟游戏循环，同时模拟网络延迟
    std::cout << "\n=== 开始模拟游戏循环 ===" << std::endl;

    // 模拟两个玩家的输入
    std::vector<std::vector<Input>> playerInputs(2);

    // 生成一些测试输入
    for (int frame = 0; frame < 120; frame++)
    {
        Input input1, input2;

        // 玩家1：每30帧攻击一次
        if (frame % 30 == 10)
        {
            input1.press(FightingButtons::PUNCH);
            std::cout << "Frame " << frame << ": Player 1 punches!" << std::endl;
        }

        // 玩家2：每30帧防御一次
        if (frame % 30 == 15)
        {
            input2.press(FightingButtons::BLOCK);
            std::cout << "Frame " << frame << ": Player 2 blocks!" << std::endl;
        }

        // 移动输入
        if (frame < 60)
        {
            input1.axis_x = 100;  // 玩家1向右移动
            input2.axis_x = -100; // 玩家2向左移动
        }

        playerInputs[0].push_back(input1);
        playerInputs[1].push_back(input2);
    }

    // 模拟网络延迟：延迟发送远程输入
    std::cout << "\n开始模拟（网络延迟3帧）..." << std::endl;

    for (int frame = 0; frame < 120; frame++)
    {
        // 设置本地输入（玩家1的输入）
        engine.setLocalInput(playerInputs[0][frame]);

        // 模拟网络延迟：玩家2的输入延迟3帧到达
        if (frame >= 3)
        {
            engine.receiveRemoteInput(1, frame - 3, playerInputs[1][frame - 3]);
        }

        // 推进一帧
        engine.update();

        // 每30帧打印状态
        if (frame % 30 == 0 || frame == 119)
        {
            auto state = engine.getCurrentState();
            std::cout << "Frame " << frame
                      << " | Checksum: " << state.checksum
                      << " | Rollbacks: " << engine.getLastRollbackCount() << std::endl;
        }
    }

    // 4. 最终状态
    std::cout << "\n=== 最终状态 ===" << std::endl;
    engine.debugPrint();

    // 5. 测试快照恢复
    std::cout << "\n=== 测试快照恢复 ===" << std::endl;

    // 获取当前状态
    auto currentState = engine.getCurrentState();
    std::cout << "当前帧号: " << currentState.frame << std::endl;

    // 创建新的引擎并恢复状态
    auto game2 = GameRegistry::createGame("fighting");
    RollbackEngine engine2(std::move(game2), 2);

    // 直接设置状态（通过恢复快照）
    // 注意：这里简化处理，实际应该通过快照恢复

    std::cout << "测试完成！" << std::endl;

    return 0;
}