// web/src/renderer.js

class Renderer {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.width = canvas.width;
        this.height = canvas.height;
        
        // 角色绘制参数
        this.characterSize = { width: 40, height: 60 };
        
        console.log('Renderer initialized');
    }
    
    // 绘制游戏画面
    draw(renderData) {
        // 解析渲染数据（与 C++ 中 FightingGame::getRenderData 的格式对应）
        // 数据格式：
        // [0-1] 玩家1 X 坐标
        // [2-3] 玩家1 Y 坐标
        // [4-5] 玩家1 血量 (0-100)
        // [6]   玩家1 状态
        // [7-8] 玩家2 X 坐标
        // [9-10] 玩家2 Y 坐标
        // [11-12] 玩家2 血量
        // [13]   玩家2 状态
        
        if (!renderData || renderData.length < 14) {
            // 没有有效数据，绘制等待画面
            this.drawWaiting();
            return;
        }
        
        // 解析玩家1数据
        const p1x = renderData[0] | (renderData[1] << 8);
        const p1y = renderData[2] | (renderData[3] << 8);
        const p1hp = renderData[4] | (renderData[5] << 8);
        const p1state = renderData[6];
        
        // 解析玩家2数据
        const p2x = renderData[7] | (renderData[8] << 8);
        const p2y = renderData[9] | (renderData[10] << 8);
        const p2hp = renderData[11] | (renderData[12] << 8);
        const p2state = renderData[13];
        
        // 清空画布
        this.ctx.clearRect(0, 0, this.width, this.height);
        
        // 绘制背景
        this.drawBackground();
        
        // 绘制地面
        this.drawGround();
        
        // 绘制阴影
        this.drawShadow(p1x + this.width/2, p1state);
        this.drawShadow(p2x + this.width/2, p2state);
        
        // 绘制玩家
        this.drawCharacter(p1x + this.width/2, this.height - 100 - p1y, p1state, '#4ecdc4', 'P1');
        this.drawCharacter(p2x + this.width/2, this.height - 100 - p2y, p2state, '#ff6b6b', 'P2');
        
        // 更新血条
        this.updateHealthBar('health1-fill', p1hp);
        this.updateHealthBar('health2-fill', p2hp);
        
        // 绘制状态文字
        this.drawStateText(p1state, 80, 60, 'P1');
        this.drawStateText(p2state, this.width - 120, 60, 'P2');
        
        // 绘制攻击特效
        this.drawEffects(p1state, p2state, p1x + this.width/2, p2x + this.width/2);
    }
    
    drawBackground() {
        // 渐变天空
        const gradient = this.ctx.createLinearGradient(0, 0, 0, this.height);
        gradient.addColorStop(0, '#1a1a2e');
        gradient.addColorStop(1, '#16213e');
        this.ctx.fillStyle = gradient;
        this.ctx.fillRect(0, 0, this.width, this.height);
        
        // 装饰云
        this.ctx.fillStyle = 'rgba(255,255,255,0.1)';
        this.ctx.beginPath();
        this.ctx.ellipse(100, 80, 60, 40, 0, 0, Math.PI * 2);
        this.ctx.ellipse(700, 120, 80, 50, 0, 0, Math.PI * 2);
        this.ctx.fill();
    }
    
    drawGround() {
        // 地面阴影
        this.ctx.fillStyle = '#2c3e50';
        this.ctx.fillRect(0, this.height - 50, this.width, 50);
        
        // 地面高光
        this.ctx.fillStyle = '#3a5a6e';
        this.ctx.fillRect(0, this.height - 52, this.width, 2);
        
        // 装饰线条
        this.ctx.strokeStyle = '#4ecdc4';
        this.ctx.lineWidth = 2;
        for (let i = 0; i < 10; i++) {
            const x = i * 80 + (Date.now() * 0.002) % 80;
            this.ctx.beginPath();
            this.ctx.moveTo(x, this.height - 50);
            this.ctx.lineTo(x + 40, this.height - 45);
            this.ctx.stroke();
        }
    }
    
    drawShadow(x, state) {
        const shadowSize = state === 1 ? 50 : 40;
        this.ctx.fillStyle = 'rgba(0,0,0,0.3)';
        this.ctx.beginPath();
        this.ctx.ellipse(x, this.height - 45, shadowSize, 15, 0, 0, Math.PI * 2);
        this.ctx.fill();
    }
    
    drawCharacter(x, y, state, color, label) {
        const width = this.characterSize.width;
        const height = this.characterSize.height;
        
        // 根据状态改变颜色和效果
        let bodyColor = color;
        let outlineColor = '#fff';
        
        switch(state) {
            case 1: // 攻击状态
                bodyColor = '#ffd700';
                outlineColor = '#ffaa00';
                break;
            case 2: // 受伤状态
                bodyColor = '#ff8888';
                outlineColor = '#ff4444';
                break;
            case 3: // 防御状态
                bodyColor = '#88aaff';
                outlineColor = '#5588ff';
                break;
        }
        
        // 身体
        this.ctx.fillStyle = bodyColor;
        this.ctx.shadowBlur = 10;
        this.ctx.shadowColor = 'rgba(0,0,0,0.5)';
        this.ctx.fillRect(x - width/2, y - height, width, height);
        
        // 轮廓
        this.ctx.strokeStyle = outlineColor;
        this.ctx.lineWidth = 2;
        this.ctx.strokeRect(x - width/2, y - height, width, height);
        
        // 头部
        this.ctx.fillStyle = '#ffccaa';
        this.ctx.fillRect(x - 15, y - height - 15, 30, 30);
        
        // 眼睛
        this.ctx.fillStyle = '#fff';
        this.ctx.fillRect(x - 10, y - height - 8, 6, 8);
        this.ctx.fillRect(x + 4, y - height - 8, 6, 8);
        this.ctx.fillStyle = '#000';
        this.ctx.fillRect(x - 9, y - height - 7, 4, 6);
        this.ctx.fillRect(x + 5, y - height - 7, 4, 6);
        
        // 攻击特效
        if (state === 1) {
            this.ctx.fillStyle = '#ffaa44';
            this.ctx.beginPath();
            this.ctx.moveTo(x + 25, y - 30);
            this.ctx.lineTo(x + 45, y - 40);
            this.ctx.lineTo(x + 35, y - 20);
            this.ctx.fill();
        }
        
        // 防御特效
        if (state === 3) {
            this.ctx.strokeStyle = '#88aaff';
            this.ctx.lineWidth = 3;
            this.ctx.beginPath();
            this.ctx.arc(x, y - 30, 25, 0, Math.PI * 2);
            this.ctx.stroke();
        }
        
        // 标签
        this.ctx.font = 'bold 12px monospace';
        this.ctx.fillStyle = '#fff';
        this.ctx.shadowBlur = 0;
        this.ctx.fillText(label, x - 10, y - height - 25);
        
        this.ctx.shadowBlur = 0;
    }
    
    drawStateText(state, x, y, playerName) {
        let stateText = '';
        let stateColor = '#aaa';
        
        switch(state) {
            case 0: stateText = 'STAND'; stateColor = '#4ecdc4'; break;
            case 1: stateText = 'ATTACK'; stateColor = '#ffd700'; break;
            case 2: stateText = 'HIT'; stateColor = '#ff6b6b'; break;
            case 3: stateText = 'BLOCK'; stateColor = '#88aaff'; break;
            default: stateText = '?';
        }
        
        this.ctx.font = 'bold 14px monospace';
        this.ctx.fillStyle = stateColor;
        this.ctx.shadowBlur = 0;
        this.ctx.fillText(playerName + ': ' + stateText, x, y);
    }
    
    drawEffects(p1state, p2state, p1x, p2x) {
        // 碰撞特效（当双方靠近且一方攻击时）
        const distance = Math.abs(p1x - p2x);
        if (distance < 80 && (p1state === 1 || p2state === 1)) {
            const effectX = (p1x + p2x) / 2;
            const effectY = this.height - 120;
            
            this.ctx.fillStyle = '#ffaa44';
            for (let i = 0; i < 5; i++) {
                const angle = (Date.now() * 0.02 + i) * Math.PI * 2 / 5;
                const radius = 15 + Math.sin(Date.now() * 0.02) * 5;
                const x = effectX + Math.cos(angle) * radius;
                const y = effectY + Math.sin(angle) * radius;
                this.ctx.beginPath();
                this.ctx.arc(x, y, 3, 0, Math.PI * 2);
                this.ctx.fill();
            }
        }
    }
    
    drawWaiting() {
        this.ctx.clearRect(0, 0, this.width, this.height);
        this.drawBackground();
        this.drawGround();
        
        this.ctx.font = '24px monospace';
        this.ctx.fillStyle = '#4ecdc4';
        this.ctx.textAlign = 'center';
        this.ctx.fillText('等待游戏启动...', this.width/2, this.height/2);
        this.ctx.textAlign = 'left';
    }
    
    updateHealthBar(elementId, hp) {
        const element = document.getElementById(elementId);
        if (element) {
            const percent = Math.max(0, Math.min(100, hp));
            element.style.width = percent + '%';
            
            // 根据血量改变颜色
            if (percent < 30) {
                element.style.background = 'linear-gradient(90deg, #ff6b6b, #ff4757)';
            } else if (percent < 60) {
                element.style.background = 'linear-gradient(90deg, #ffaa44, #ff8800)';
            } else {
                element.style.background = 'linear-gradient(90deg, #4ecdc4, #2ecc71)';
            }
        }
    }
}