#include "platform/game_registry.hpp"
#include <iostream>

namespace platform
{

    // 获取注册表的内部存储
    // 使用静态局部变量，在第一次调用时初始化
    std::unordered_map<std::string, GameFactory> &GameRegistry::getRegistry()
    {
        static std::unordered_map<std::string, GameFactory> registry;
        return registry;
    }

    // 注册游戏
    void GameRegistry::registerGame(const std::string &gameId, GameFactory factory)
    {
        auto &registry = getRegistry();

        // 检查是否已经注册过
        if (registry.find(gameId) != registry.end())
        {
            // 如果已存在，可以选择覆盖或报错
            // 这里选择覆盖，并打印警告
            std::cerr << "Warning: Game '" << gameId << "' is already registered. Overwriting." << std::endl;
        }

        registry[gameId] = factory;
        std::cout << "Registered game: " << gameId << std::endl;
    }

    // 创建游戏实例
    std::unique_ptr<IGame> GameRegistry::createGame(const std::string &gameId)
    {
        auto &registry = getRegistry();
        auto it = registry.find(gameId);

        if (it == registry.end())
        {
            // 游戏未注册
            std::cerr << "Error: Game '" << gameId << "' not found in registry." << std::endl;
            return nullptr;
        }

        // 调用工厂函数创建游戏实例
        return it->second();
    }

    // 获取所有已注册的游戏名称
    std::vector<std::string> GameRegistry::getAvailableGames()
    {
        auto &registry = getRegistry();
        std::vector<std::string> games;
        games.reserve(registry.size());

        for (const auto &pair : registry)
        {
            games.push_back(pair.first);
        }

        return games;
    }

    // 检查游戏是否已注册
    bool GameRegistry::isGameRegistered(const std::string &gameId)
    {
        auto &registry = getRegistry();
        return registry.find(gameId) != registry.end();
    }

} // namespace platform