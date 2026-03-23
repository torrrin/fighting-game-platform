/*
// web/public/src/main.js

// 全局变量
let Module = null;
let enginePtr = null;
let animationId = null;
let lastTimestamp = 0;
let frameCount = 0;
let fps = 0;
let renderer = null;
let inputHandler = null;
let isRunning = false;

// 状态显示
const statusElement = document.getElementById('status');

function setStatus(message, type = 'loading') {
    if (statusElement) {
        statusElement.textContent = message;
        statusElement.className = `status ${type}`;
    }
    console.log(`[Status] ${message}`);
}

// 初始化游戏
function initGame() {
    setStatus('正在创建游戏引擎...', 'loading');
    
    // 检查 Module 和导出函数
    if (!Module || !Module._game_create) {
        console.error('Module exports:', Module ? Object.keys(Module).filter(k => k.startsWith('_')) : 'Module is null');
        return false;
    }
    
    // 创建引擎实例
    enginePtr = Module._game_create();
    if (!enginePtr) {
        console.error('Failed to create game engine');
        setStatus('游戏引擎创建失败', 'error');
        return false;
    }
    
    console.log('Engine created, ptr:', enginePtr);
    
    // 初始化渲染器
    const canvas = document.getElementById('game-canvas');
    if (!canvas) {
        console.error('Canvas not found');
        return false;
    }
    renderer = new Renderer(canvas);
    
    // 初始化输入处理器
    inputHandler = new InputHandler();
    inputHandler.onInput = (buttons, axisX, axisY) => {
        if (enginePtr && Module && Module._game_set_local_input) {
            Module._game_set_local_input(buttons, axisX, axisY);
        }
    };
    
    setStatus('游戏引擎已就绪', 'running');
    return true;
}

// 获取渲染数据
function getRenderData() {
    if (!Module || !Module._game_get_render_data) return null;
    
    // 分配缓冲区
    const bufferSize = 1024;
    const bufferPtr = Module._malloc(bufferSize);
    
    if (!bufferPtr) {
        console.error('Failed to allocate buffer');
        return null;
    }
    
    const dataSize = Module._game_get_render_data(bufferPtr, bufferSize);
    
    let result = null;
    if (dataSize > 0 && dataSize <= bufferSize) {
        // 复制数据到 JS 数组
        const data = new Uint8Array(Module.HEAPU8.buffer, bufferPtr, dataSize);
        result = new Uint8Array(data);
    } else if (dataSize === -1) {
        console.warn('Buffer too small');
    }
    
    Module._free(bufferPtr);
    return result;
}

// 游戏循环
function gameLoop(now) {
    if (!isRunning) return;
    
    // 计算 FPS
    const delta = now - lastTimestamp;
    if (delta >= 1000) {
        fps = frameCount;
        frameCount = 0;
        lastTimestamp = now;
        
        const fpsElement = document.getElementById('fps');
        if (fpsElement) fpsElement.textContent = fps;
    }
    
    // 推进游戏逻辑
    if (enginePtr && Module && Module._game_update) {
        Module._game_update();
        frameCount++;
        
        // 更新 UI
        const frame = Module._game_get_current_frame();
        const rollbacks = Module._game_get_rollback_count();
        
        const frameElement = document.getElementById('frame-count');
        const rollbackElement = document.getElementById('rollback-count');
        
        if (frameElement) frameElement.textContent = frame;
        if (rollbackElement) rollbackElement.textContent = rollbacks;
        
        // 获取渲染数据并绘制
        const renderData = getRenderData();
        if (renderData && renderer) {
            renderer.draw(renderData);
        }
    }
    
    // 继续循环
    animationId = requestAnimationFrame(gameLoop);
}

// 启动游戏
async function start() {
    console.log('Starting application...');
    setStatus('正在加载 WASM 模块...', 'loading');
    
    try {
        // 动态加载 WASM 模块（模块化方式）
        const script = document.createElement('script');
        script.src = 'game_wasm.js';
        
        // 等待脚本加载完成
        await new Promise((resolve, reject) => {
            script.onload = resolve;
            script.onerror = reject;
            document.head.appendChild(script);
        });
        
        console.log('WASM script loaded');
        
        // 调用导出的函数创建模块
        // 注意：由于 MODULARIZE=1，全局会有一个 createGameModule 函数
        if (typeof createGameModule === 'undefined') {
            throw new Error('createGameModule not found');
        }
        
        setStatus('正在初始化 WASM...', 'loading');
        
        // 调用 createGameModule 获取 Module 实例
        Module = await createGameModule();
        
        console.log('WASM module created:', Module);
        
        // 设置打印回调
        if (Module.print) {
            Module.print = (text) => console.log('[WASM]', text);
            Module.printErr = (text) => console.error('[WASM]', text);
        }
        
        setStatus('WASM 加载成功', 'loading');
        
        // 初始化游戏
        if (!initGame()) {
            throw new Error('Game initialization failed');
        }
        
        console.log('Game initialized, starting loop');
        
        // 启动游戏循环
        isRunning = true;
        lastTimestamp = performance.now();
        animationId = requestAnimationFrame(gameLoop);
        
        setStatus('游戏运行中', 'running');
        
    } catch (error) {
        console.error('Failed to start:', error);
        setStatus('启动失败: ' + error.message, 'error');
    }
}

// 页面关闭时清理
window.addEventListener('beforeunload', () => {
    isRunning = false;
    if (animationId) {
        cancelAnimationFrame(animationId);
        animationId = null;
    }
    if (enginePtr && Module && Module._game_destroy) {
        Module._game_destroy();
        enginePtr = null;
    }
});

// 启动
start();
*/

// web/public/src/main.js - 简化测试版
// web/public/src/main.js

console.log('=== 游戏平台启动 ===');

// 等待页面加载完成
window.addEventListener('load', async () => {
    console.log('页面加载完成');
    
    const statusElement = document.getElementById('status');
    const canvas = document.getElementById('game-canvas');
    const ctx = canvas.getContext('2d');
    
    statusElement.textContent = '正在加载 WASM...';
    
    try {
        // 检查 createGameModule 是否存在
        if (typeof createGameModule === 'undefined') {
            throw new Error('createGameModule 未定义，请确保 game_wasm.js 已加载');
        }
        
        console.log('createGameModule 存在，正在创建模块...');
        statusElement.textContent = '正在初始化 WASM...';
        
        // 创建 WASM 模块
        const Module = await createGameModule();
        console.log('WASM 模块创建成功', Module);
        
        // 设置打印回调
        Module.print = (text) => console.log('[WASM]', text);
        Module.printErr = (text) => console.error('[WASM]', text);
        
        statusElement.textContent = '正在创建游戏引擎...';
        
        // 创建游戏引擎
        const enginePtr = Module._game_create();
        console.log('引擎指针:', enginePtr);
        
        if (!enginePtr) {
            throw new Error('游戏引擎创建失败');
        }
        
        statusElement.textContent = '游戏运行中！按 A/S/K/L 和方向键测试';
        
        // 输入状态
        let player1Buttons = 0;
        let player2Buttons = 0;
        let axisX = 0;
        
        // 键盘事件处理
        function handleKeyDown(e) {
            let changed = false;
            
            switch(e.code) {
                // 玩家1
                case 'KeyA':
                    player1Buttons |= 1; // PUNCH
                    changed = true;
                    break;
                case 'KeyS':
                    player1Buttons |= 4; // BLOCK
                    changed = true;
                    break;
                // 玩家2
                case 'KeyK':
                    player2Buttons |= 1; // PUNCH
                    changed = true;
                    break;
                case 'KeyL':
                    player2Buttons |= 4; // BLOCK
                    changed = true;
                    break;
                // 移动（双方共享，简化）
                case 'ArrowLeft':
                    axisX = -128;
                    changed = true;
                    break;
                case 'ArrowRight':
                    axisX = 127;
                    changed = true;
                    break;
                default:
                    return;
            }
            
            if (changed) {
                e.preventDefault();
                // 注意：这里我们只发送玩家1的输入，玩家2的输入由远程输入模拟
                // 简化版：玩家2的输入也通过同一个函数发送
                const combinedButtons = player1Buttons | player2Buttons;
                Module._game_set_local_input(combinedButtons, axisX, 0);
            }
        }
        
        function handleKeyUp(e) {
            let changed = false;
            
            switch(e.code) {
                case 'KeyA':
                    player1Buttons &= ~1;
                    changed = true;
                    break;
                case 'KeyS':
                    player1Buttons &= ~4;
                    changed = true;
                    break;
                case 'KeyK':
                    player2Buttons &= ~1;
                    changed = true;
                    break;
                case 'KeyL':
                    player2Buttons &= ~4;
                    changed = true;
                    break;
                case 'ArrowLeft':
                    if (axisX === -128) {
                        axisX = 0;
                        changed = true;
                    }
                    break;
                case 'ArrowRight':
                    if (axisX === 127) {
                        axisX = 0;
                        changed = true;
                    }
                    break;
                default:
                    return;
            }
            
            if (changed) {
                e.preventDefault();
                const combinedButtons = player1Buttons | player2Buttons;
                Module._game_set_local_input(combinedButtons, axisX, 0);
            }
        }
        
        window.addEventListener('keydown', handleKeyDown);
        window.addEventListener('keyup', handleKeyUp);
        
        // 游戏循环
        let frameCount = 0;
        let lastTime = performance.now();
        
        function gameLoop() {
            // 推进游戏
            Module._game_update();
            frameCount++;
            
            // 计算 FPS
            const now = performance.now();
            if (now - lastTime >= 1000) {
                const fps = frameCount;
                frameCount = 0;
                lastTime = now;
                const fpsElement = document.getElementById('fps');
                if (fpsElement) fpsElement.textContent = fps;
            }
            
            // 获取渲染数据
            const bufferSize = 1024;
            const bufferPtr = Module._malloc(bufferSize);
            const dataSize = Module._game_get_render_data(bufferPtr, bufferSize);
            
            if (dataSize > 0 && dataSize <= bufferSize) {
                const data = new Uint8Array(Module.HEAPU8.buffer, bufferPtr, dataSize);
                
                // 解析数据
                if (data.length >= 14) {
                    // 玩家1数据
                    const p1x = data[0] | (data[1] << 8);
                    const p1y = data[2] | (data[3] << 8);
                    const p1hp = data[4] | (data[5] << 8);
                    const p1state = data[6];
                    
                    // 玩家2数据
                    const p2x = data[7] | (data[8] << 8);
                    const p2y = data[9] | (data[10] << 8);
                    const p2hp = data[11] | (data[12] << 8);
                    const p2state = data[13];
                    
                    // 清空画布
                    ctx.fillStyle = '#1a1a2e';
                    ctx.fillRect(0, 0, 800, 400);
                    
                    // 绘制地面
                    ctx.fillStyle = '#2c3e50';
                    ctx.fillRect(0, 350, 800, 50);
                    
                    // 地面装饰
                    ctx.fillStyle = '#4ecdc4';
                    for (let i = 0; i < 10; i++) {
                        const x = i * 80 + (Date.now() * 0.002) % 80;
                        ctx.fillRect(x, 345, 40, 5);
                    }
                    
                    // 绘制玩家1
                    ctx.fillStyle = p1state === 1 ? '#ffd700' : (p1state === 2 ? '#ff8888' : '#4ecdc4');
                    ctx.fillRect(p1x + 400 - 20, 300 - p1y, 40, 60);
                    
                    // 绘制玩家2
                    ctx.fillStyle = p2state === 1 ? '#ffd700' : (p2state === 2 ? '#ff8888' : '#ff6b6b');
                    ctx.fillRect(p2x + 400 - 20, 300 - p2y, 40, 60);
                    
                    // 绘制血条背景
                    ctx.fillStyle = '#333';
                    ctx.fillRect(50, 20, 200, 20);
                    ctx.fillRect(550, 20, 200, 20);
                    
                    // 绘制血条
                    const p1Percent = Math.max(0, Math.min(100, p1hp));
                    const p2Percent = Math.max(0, Math.min(100, p2hp));
                    ctx.fillStyle = '#ff4757';
                    ctx.fillRect(50, 20, p1Percent * 2, 20);
                    ctx.fillRect(550, 20, p2Percent * 2, 20);
                    
                    // 绘制状态文字
                    const stateNames = ['STAND', 'ATTACK', 'HIT', 'BLOCK'];
                    ctx.font = '12px monospace';
                    ctx.fillStyle = '#fff';
                    ctx.fillText('P1: ' + stateNames[p1state] || '?', 50, 55);
                    ctx.fillText('P2: ' + stateNames[p2state] || '?', 700, 55);
                    
                    // 更新 UI 数值
                    document.getElementById('frame-count').textContent = Module._game_get_current_frame();
                    document.getElementById('rollback-count').textContent = Module._game_get_rollback_count();
                    document.getElementById('health1-fill').style.width = p1Percent + '%';
                    document.getElementById('health2-fill').style.width = p2Percent + '%';
                }
            }
            
            Module._free(bufferPtr);
            
            // 继续循环
            requestAnimationFrame(gameLoop);
        }
        
        // 启动游戏循环
        gameLoop();
        
        console.log('游戏循环已启动');
        
    } catch (error) {
        console.error('启动失败:', error);
        statusElement.textContent = '启动失败: ' + error.message;
        statusElement.className = 'status error';
    }
});