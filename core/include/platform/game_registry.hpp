#pragma once
#include "igame.hpp"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>

namespace platform
{

    // 游戏工厂函数类型
    // 这是一个函数指针类型：不接受参数，返回一个 IGame 的智能指针
    // 每个游戏都要提供一个这样的函数，用来创建自己的实例
    using GameFactory = std::function<std::unique_ptr<IGame>()>;

    // 游戏注册表类
    // 负责管理所有可用的游戏，提供注册和创建功能
    class GameRegistry
    {
    public:
        // 注册一个游戏
        // 参数 gameId: 游戏的唯一标识符（如 "fighting_game"）
        // 参数 factory: 创建该游戏的工厂函数
        static void registerGame(const std::string &gameId, GameFactory factory);

        // 创建一个游戏实例
        // 参数 gameId: 要创建的游戏标识符
        // 返回值: 游戏的智能指针，如果 gameId 不存在则返回 nullptr
        static std::unique_ptr<IGame> createGame(const std::string &gameId);

        // 获取所有已注册的游戏名称列表
        // 返回值: 游戏名称的 vector，用于在 UI 上显示游戏列表
        static std::vector<std::string> getAvailableGames();

        // 检查某个游戏是否已注册
        // 参数 gameId: 游戏标识符
        // 返回值: true 表示已注册，false 表示未注册
        static bool isGameRegistered(const std::string &gameId);

    private:
        // 私有构造函数，禁止实例化（这是一个静态类）
        GameRegistry() = delete;

        // 获取注册表的内部存储
        // 使用静态局部变量，确保在第一次使用时初始化，且线程安全
        static std::unordered_map<std::string, GameFactory> &getRegistry();
    };

} // namespace platform