// core/tests/test_fighting_game.cpp
#include "games/fighting_game.hpp"
#include "platform/game_registry.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace platform;

int main()
{
    std::cout << "=== 测试格斗游戏 ===" << std::endl;

    // 1. 注册格斗游戏
    GameRegistry::registerGame("fighting", []()
                               { return std::make_unique<FightingGame>(); });

    // 2. 创建游戏实例
    auto game = GameRegistry::createGame("fighting");
    if (!game)
    {
        std::cout << "创建游戏失败！" << std::endl;
        return 1;
    }

    std::cout << "游戏名称: " << game->getGameName() << std::endl;
    std::cout << "最大玩家: " << game->getMaxPlayers() << std::endl;
    std::cout << "帧率: " << game->getTickRate() << std::endl;

    // 3. 初始化游戏
    GameState initState;
    game->init(initState);

    // 4. 模拟60帧的游戏循环
    std::cout << "\n开始游戏循环..." << std::endl;

    for (int frame = 0; frame < 60; frame++)
    {
        // 创建输入（玩家1按攻击，玩家2按防御）
        std::vector<Input> inputs(2);

        if (frame == 10)
        {
            // 第10帧，玩家1攻击
            inputs[0].press(FightingButtons::PUNCH);
            std::cout << "Frame " << frame << ": Player 1 punches!" << std::endl;
        }

        if (frame == 15)
        {
            // 第15帧，玩家2防御
            inputs[1].press(FightingButtons::BLOCK);
            std::cout << "Frame " << frame << ": Player 2 blocks!" << std::endl;
        }

        // 推进游戏
        game->advanceFrame(inputs);

        // 每10帧打印一次状态
        if (frame % 10 == 0 || frame == 59)
        {
            auto state = game->getCurrentState();
            std::cout << "Frame " << frame
                      << " | Checksum: " << state.checksum << std::endl;
        }
    }

    // 5. 获取最终状态
    auto finalState = game->getCurrentState();
    std::cout << "\n最终帧号: " << finalState.frame << std::endl;
    std::cout << "最终校验和: " << finalState.checksum << std::endl;

    // 6. 测试快照保存和恢复
    std::cout << "\n=== 测试快照功能 ===" << std::endl;

    // 保存快照
    auto snapshot = game->saveSnapshot();
    std::cout << "保存快照，帧号: " << snapshot.frame << std::endl;

    // 重置游戏
    game->init(initState);
    std::cout << "重置游戏" << std::endl;

    // 恢复快照
    game->restoreSnapshot(snapshot);
    auto restoredState = game->getCurrentState();
    std::cout << "恢复快照，帧号: " << restoredState.frame << std::endl;

    if (restoredState.checksum == finalState.checksum)
    {
        std::cout << "✓ 快照恢复成功，状态一致" << std::endl;
    }
    else
    {
        std::cout << "✗ 快照恢复失败，状态不一致" << std::endl;
    }

    // 7. 获取渲染数据
    auto renderData = game->getRenderData();
    std::cout << "\n渲染数据大小: " << renderData.size() << " 字节" << std::endl;

    std::cout << "\n测试完成！" << std::endl;
    return 0;
}