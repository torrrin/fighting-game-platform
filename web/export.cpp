// web/export.cpp
#include "platform/rollback_engine.hpp"
#include "platform/game_registry.hpp"
#include "games/fighting_game.hpp"
#include <emscripten.h>
#include <memory>
#include <iostream>

using namespace platform;

// 全局引擎实例
static std::unique_ptr<RollbackEngine> g_engine;

// 注册游戏（在 WASM 加载时自动执行）
static void registerGames()
{
    GameRegistry::registerGame("fighting", []()
                               { return std::make_unique<FightingGame>(); });
    std::cout << "Games registered" << std::endl;
}

// 使用构造函数确保在 WASM 加载时执行
struct AutoRegister
{
    AutoRegister()
    {
        registerGames();
    }
};
static AutoRegister auto_register;

extern "C"
{

    // 创建引擎实例
    EMSCRIPTEN_KEEPALIVE
    void *game_create()
    {
        std::cout << "game_create called" << std::endl;
        auto game = GameRegistry::createGame("fighting");
        if (!game)
        {
            std::cout << "Failed to create game" << std::endl;
            return nullptr;
        }

        g_engine = std::make_unique<RollbackEngine>(std::move(game), 2);
        std::cout << "Engine created" << std::endl;
        return g_engine.get();
    }

    // 销毁引擎
    EMSCRIPTEN_KEEPALIVE
    void game_destroy()
    {
        std::cout << "game_destroy called" << std::endl;
        g_engine.reset();
    }

    // 推进一帧
    EMSCRIPTEN_KEEPALIVE
    void game_update()
    {
        if (g_engine)
        {
            g_engine->update();
        }
    }

    // 设置本地输入
    EMSCRIPTEN_KEEPALIVE
    void game_set_local_input(uint32_t buttons, int16_t axis_x, int16_t axis_y)
    {
        if (g_engine)
        {
            Input input;
            input.buttons = buttons;
            input.axis_x = axis_x;
            input.axis_y = axis_y;
            g_engine->setLocalInput(input);
        }
    }

    // 接收远程输入（单机测试时不需要）
    EMSCRIPTEN_KEEPALIVE
    void game_receive_input(int player_id, uint32_t frame,
                            uint32_t buttons, int16_t axis_x, int16_t axis_y)
    {
        if (g_engine)
        {
            Input input;
            input.buttons = buttons;
            input.axis_x = axis_x;
            input.axis_y = axis_y;
            g_engine->receiveRemoteInput(player_id, frame, input);
        }
    }

    // 获取渲染数据
    EMSCRIPTEN_KEEPALIVE
    int game_get_render_data(uint8_t *buffer, int buffer_size)
    {
        if (!g_engine)
            return 0;

        auto data = g_engine->getRenderData();
        if (data.size() > static_cast<size_t>(buffer_size))
        {
            return -1; // 缓冲区太小
        }

        memcpy(buffer, data.data(), data.size());
        return static_cast<int>(data.size());
    }

    // 获取当前帧号
    EMSCRIPTEN_KEEPALIVE
    uint32_t game_get_current_frame()
    {
        if (!g_engine)
            return 0;
        auto state = g_engine->getCurrentState();
        return state.frame;
    }

    // 获取回滚次数
    EMSCRIPTEN_KEEPALIVE
    int game_get_rollback_count()
    {
        if (!g_engine)
            return 0;
        return g_engine->getLastRollbackCount();
    }

} // extern "C"