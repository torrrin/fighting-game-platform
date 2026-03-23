// core/tests/test_registry.cpp
#include "platform/game_registry.hpp"
#include "platform/igame.hpp"
#include <iostream>
#include <memory>

using namespace platform;

// 一个模拟的游戏类，用于测试
class DummyGame : public IGame
{
public:
    void init(const GameState &) override
    {
        std::cout << "DummyGame::init called" << std::endl;
    }
    void shutdown() override
    {
        std::cout << "DummyGame::shutdown called" << std::endl;
    }
    void advanceFrame(const std::vector<Input> &) override
    {
        std::cout << "DummyGame::advanceFrame called" << std::endl;
    }
    GameState getCurrentState() const override
    {
        return GameState{};
    }
    void setState(const GameState &) override {}
    Snapshot saveSnapshot() const override
    {
        return Snapshot{};
    }
    void restoreSnapshot(const Snapshot &) override {}
    std::vector<uint8_t> getRenderData() const override
    {
        return {};
    }
    int getMaxPlayers() const override { return 2; }
    int getTickRate() const override { return 60; }
    std::string getGameName() const override { return "DummyGame"; }
    bool validateState(const GameState &) const override { return true; }
};

int main()
{
    std::cout << "=== 测试游戏注册表 ===" << std::endl;

    // 1. 注册游戏
    GameRegistry::registerGame("dummy", []()
                               { return std::make_unique<DummyGame>(); });

    // 2. 查看已注册的游戏
    auto games = GameRegistry::getAvailableGames();
    std::cout << "已注册的游戏: ";
    for (const auto &name : games)
    {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    // 3. 检查游戏是否存在
    if (GameRegistry::isGameRegistered("dummy"))
    {
        std::cout << "✓ 'dummy' 游戏已注册" << std::endl;
    }

    // 4. 创建游戏实例
    auto game = GameRegistry::createGame("dummy");
    if (game)
    {
        std::cout << "✓ 成功创建游戏实例" << std::endl;
        std::cout << "  游戏名称: " << game->getGameName() << std::endl;
        std::cout << "  最大玩家: " << game->getMaxPlayers() << std::endl;
        std::cout << "  帧率: " << game->getTickRate() << std::endl;

        // 测试调用游戏方法
        game->init(GameState{});
        std::vector<Input> inputs(2);
        game->advanceFrame(inputs);
        game->shutdown();
    }
    else
    {
        std::cout << "✗ 创建游戏失败" << std::endl;
    }

    // 5. 测试不存在的游戏
    auto notExist = GameRegistry::createGame("not_exist");
    if (!notExist)
    {
        std::cout << "✓ 不存在的游戏返回 nullptr" << std::endl;
    }

    return 0;
}