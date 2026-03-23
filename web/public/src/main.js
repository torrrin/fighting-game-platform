// web/public/src/main.js

console.log('=== 游戏平台启动 ===');

// 等待页面和 WASM 加载完成
window.addEventListener('load', () => {
    console.log('页面加载完成');
    
    const statusElement = document.getElementById('status');
    const canvas = document.getElementById('game-canvas');
    const ctx = canvas.getContext('2d');
    
    statusElement.textContent = '正在等待 WASM 初始化...';
    
    // 等待 Module 完全初始化
    function waitForModuleReady() {
        return new Promise((resolve, reject) => {
            // 检查 Module 是否存在
            if (!window.Module) {
                reject(new Error('Module 不存在'));
                return;
            }
            
            const Module = window.Module;
            
            // 检查是否已经初始化
            if (Module.ready || (Module._game_create && typeof Module._game_create !== 'function' && Module._game_create.toString().includes('makeInvalidEarlyAccess') === false)) {
                // 尝试调用一个函数测试
                try {
                    if (Module._game_create && Module._game_create !== Module.makeInvalidEarlyAccess) {
                        console.log('Module 已完全初始化');
                        resolve(Module);
                        return;
                    }
                } catch(e) {}
            }
            
            // 等待 onRuntimeInitialized 回调
            if (Module.onRuntimeInitialized) {
                const originalCallback = Module.onRuntimeInitialized;
                Module.onRuntimeInitialized = () => {
                    console.log('onRuntimeInitialized 触发');
                    if (originalCallback) originalCallback();
                    resolve(Module);
                };
            } else {
                // 轮询检查
                let attempts = 0;
                const interval = setInterval(() => {
                    attempts++;
                    try {
                        if (Module._game_create && Module._game_create !== Module.makeInvalidEarlyAccess) {
                            clearInterval(interval);
                            console.log('Module 已就绪（轮询检测）');
                            resolve(Module);
                        } else if (attempts > 200) {
                            clearInterval(interval);
                            reject(new Error('Module 初始化超时'));
                        }
                    } catch(e) {}
                }, 100);
            }
        });
    }
    
    // 启动游戏
    async function startGame() {
        try {
            const Module = await waitForModuleReady();
            console.log('WASM 模块已完全就绪', Module);
            
            // 验证函数是否可用
            console.log('_game_create 类型:', typeof Module._game_create);
            console.log('_game_create 是否是函数:', typeof Module._game_create === 'function');
            
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
                    case 'KeyA':
                        player1Buttons |= 1;
                        changed = true;
                        break;
                    case 'KeyS':
                        player1Buttons |= 4;
                        changed = true;
                        break;
                    case 'KeyK':
                        player2Buttons |= 1;
                        changed = true;
                        break;
                    case 'KeyL':
                        player2Buttons |= 4;
                        changed = true;
                        break;
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
                if (!enginePtr) return;
                
                try {
                    Module._game_update();
                    frameCount++;
                    
                    const now = performance.now();
                    if (now - lastTime >= 1000) {
                        const fps = frameCount;
                        frameCount = 0;
                        lastTime = now;
                        const fpsElement = document.getElementById('fps');
                        if (fpsElement) fpsElement.textContent = fps;
                    }
                    
                    const bufferSize = 1024;
                    const bufferPtr = Module._malloc(bufferSize);
                    const dataSize = Module._game_get_render_data(bufferPtr, bufferSize);
                    
                    if (dataSize > 0 && dataSize <= bufferSize) {
                        const data = new Uint8Array(Module.HEAPU8.buffer, bufferPtr, dataSize);
                        
                        if (data.length >= 14) {
                            // 正确解析有符号 16 位整数
                            const p1x_raw = data[0] | (data[1] << 8);
                            const p1x = p1x_raw > 32767 ? p1x_raw - 65536 : p1x_raw;
                            
                            const p1y_raw = data[2] | (data[3] << 8);
                            const p1y = p1y_raw > 32767 ? p1y_raw - 65536 : p1y_raw;
                            
                            const p1hp = data[4] | (data[5] << 8);
                            const p1state = data[6];
                            
                            const p2x_raw = data[7] | (data[8] << 8);
                            const p2x = p2x_raw > 32767 ? p2x_raw - 65536 : p2x_raw;
                            
                            const p2y_raw = data[9] | (data[10] << 8);
                            const p2y = p2y_raw > 32767 ? p2y_raw - 65536 : p2y_raw;
                            
                            const p2hp = data[11] | (data[12] << 8);
                            const p2state = data[13];
                            
                            // 调试输出（每60帧打印一次）
                            if (frameCount % 60 === 0) {
                                console.log(`玩家1: x=${p1x}, y=${p1y}, hp=${p1hp}, state=${p1state}`);
                                console.log(`玩家2: x=${p2x}, y=${p2y}, hp=${p2hp}, state=${p2state}`);
                            }
                            
                            // 绘制背景
                            ctx.fillStyle = '#1a1a2e';
                            ctx.fillRect(0, 0, 800, 400);
                            
                            // 地面
                            ctx.fillStyle = '#2c3e50';
                            ctx.fillRect(0, 350, 800, 50);
                            
                            // 装饰
                            ctx.fillStyle = '#4ecdc4';
                            for (let i = 0; i < 10; i++) {
                                const x = i * 80 + (Date.now() * 0.002) % 80;
                                ctx.fillRect(x, 345, 40, 5);
                            }
                            
                            // 玩家1（左侧，屏幕中心偏移）
                            const screenX1 = 400 + p1x;
                            const screenY1 = 300 - p1y;
                            ctx.fillStyle = p1state === 1 ? '#ffd700' : (p1state === 2 ? '#ff8888' : '#4ecdc4');
                            ctx.fillRect(screenX1 - 20, screenY1 - 60, 40, 60);
                            
                            // 玩家2（右侧，屏幕中心偏移）
                            const screenX2 = 400 + p2x;
                            const screenY2 = 300 - p2y;
                            ctx.fillStyle = p2state === 1 ? '#ffd700' : (p2state === 2 ? '#ff8888' : '#ff6b6b');
                            ctx.fillRect(screenX2 - 20, screenY2 - 60, 40, 60);
                            
                            // 血条背景
                            ctx.fillStyle = '#333';
                            ctx.fillRect(50, 20, 200, 20);
                            ctx.fillRect(550, 20, 200, 20);
                            
                            // 血条
                            const p1Percent = Math.max(0, Math.min(100, p1hp));
                            const p2Percent = Math.max(0, Math.min(100, p2hp));
                            ctx.fillStyle = '#ff4757';
                            ctx.fillRect(50, 20, p1Percent * 2, 20);
                            ctx.fillRect(550, 20, p2Percent * 2, 20);
                            
                            // 状态文字
                            const stateNames = ['STAND', 'ATTACK', 'HIT', 'BLOCK'];
                            ctx.font = '12px monospace';
                            ctx.fillStyle = '#fff';
                            ctx.fillText('P1: ' + (stateNames[p1state] || '?'), 50, 55);
                            ctx.fillText('P2: ' + (stateNames[p2state] || '?'), 700, 55);
                            
                            // 更新 UI
                            document.getElementById('frame-count').textContent = Module._game_get_current_frame();
                            document.getElementById('rollback-count').textContent = Module._game_get_rollback_count();
                            document.getElementById('health1-fill').style.width = p1Percent + '%';
                            document.getElementById('health2-fill').style.width = p2Percent + '%';
                        }
                    }
                    
                    Module._free(bufferPtr);
                } catch (err) {
                    console.error('Game loop error:', err);
                }
                
                requestAnimationFrame(gameLoop);
            }
            
            gameLoop();
            console.log('游戏循环已启动');
            
        } catch (error) {
            console.error('启动失败:', error);
            statusElement.textContent = '启动失败: ' + error.message;
            statusElement.className = 'status error';
        }
    }
    
    startGame();
});