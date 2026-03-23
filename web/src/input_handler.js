// web/src/input_handler.js

// 按钮位掩码定义（与 C++ 中的 FightingButtons 对应）
const FightingButtons = {
    PUNCH: 1 << 0,   // 1
    KICK: 1 << 1,    // 2
    BLOCK: 1 << 2,   // 4
};

class InputHandler {
    constructor() {
        this.buttons = 0;
        this.axisX = 0;
        this.axisY = 0;
        this.onInput = null;
        
        // 键盘状态（用于处理连续按键）
        this.keyState = new Map();
        
        this.setupKeyboard();
        this.startPolling();
    }
    
    setupKeyboard() {
        // 键盘按下
        window.addEventListener('keydown', (e) => {
            // 防止页面滚动
            if (e.key === 'ArrowLeft' || e.key === 'ArrowRight' ||
                e.key === 'ArrowUp' || e.key === 'ArrowDown') {
                e.preventDefault();
            }
            
            // 标记按键按下
            if (!this.keyState.get(e.code)) {
                this.keyState.set(e.code, true);
                this.updateInputFromKeys();
            }
        });
        
        // 键盘释放
        window.addEventListener('keyup', (e) => {
            this.keyState.set(e.code, false);
            this.updateInputFromKeys();
        });
        
        // 窗口失去焦点时重置所有按键
        window.addEventListener('blur', () => {
            this.keyState.clear();
            this.buttons = 0;
            this.axisX = 0;
            this.axisY = 0;
            this.sendInput();
        });
    }
    
    updateInputFromKeys() {
        let changed = false;
        
        // 重置当前输入
        let newButtons = 0;
        let newAxisX = 0;
        let newAxisY = 0;
        
        // 玩家1 控制 (A, S, ←, →)
        if (this.keyState.get('KeyA')) {
            newButtons |= FightingButtons.PUNCH;
        }
        if (this.keyState.get('KeyS')) {
            newButtons |= FightingButtons.BLOCK;
        }
        
        // 玩家2 控制 (K, L)
        if (this.keyState.get('KeyK')) {
            newButtons |= FightingButtons.PUNCH;
        }
        if (this.keyState.get('KeyL')) {
            newButtons |= FightingButtons.BLOCK;
        }
        
        // 移动控制（双方共享，简化处理）
        if (this.keyState.get('ArrowLeft')) {
            newAxisX = -128;
        } else if (this.keyState.get('ArrowRight')) {
            newAxisX = 127;
        } else {
            newAxisX = 0;
        }
        
        // 检查是否有变化
        if (newButtons !== this.buttons || newAxisX !== this.axisX) {
            this.buttons = newButtons;
            this.axisX = newAxisX;
            changed = true;
        }
        
        if (changed && this.onInput) {
            this.sendInput();
        }
    }
    
    // 定期发送输入（确保连续移动）
    startPolling() {
        setInterval(() => {
            if (this.onInput) {
                this.sendInput();
            }
        }, 16); // 约 60fps
    }
    
    sendInput() {
        this.onInput(this.buttons, this.axisX, this.axisY);
    }
}