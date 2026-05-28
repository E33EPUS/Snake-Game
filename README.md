# Snake Game / 贪吃蛇

> **This project is 100% AI-generated** using [Claude Code](https://claude.ai/code).
> All code, documentation, and design were produced through human-AI collaboration.
>
> **本项目 100% 由 AI 生成**，使用 [Claude Code](https://claude.ai/code)。所有代码、文档和设计均由人机协作完成。

---

## Overview / 概述

A classic snake game written in C for the Windows console, featuring emoji visuals, two game modes, difficulty settings, and multiple power-ups.

用 C 语言编写的 Windows 控制台贪吃蛇游戏，支持 emoji 视觉、双模式、难度选择和多种道具。

---

## Game Modes / 游戏模式

| Mode / 模式 | Bombs / 炸弹 | Scissors / 剪刀 | Description / 描述 |
|-------------|:------------:|:---------------:|---------------------|
| **Classic / 经典** | No / 无 | No / 无 | Traditional snake, eat apples to grow / 经典贪吃蛇，吃苹果长长 |
| **Bomb Frenzy / 炸弹狂欢** | Yes / 有 | Yes / 有 | Bombs never disappear, they keep piling up / 炸弹不消失，越积越多 |

## Difficulty / 难度

| Difficulty / 难度 | Speed / 速度 | Classic + Hard / 经典困难 | Frenzy + Hard / 狂欢困难 |
|-------------------|:-----------:|---------------------------|--------------------------|
| **Easy / 简单** | 150ms | No bombs / 无炸弹 | Bomb interval 5→1 / 炸弹每5→1苹果 |
| **Hard / 困难** | 80ms | 1 bomb every 5 apples / 每5苹果1炸弹 | Interval 3→1, double spawn, more scissors / 3→1递减, 一次两颗, 剪刀更频繁 |

## Items / 道具

| Item / 道具 | Icon / 图标 | Description / 描述 |
|-------------|:-----------:|---------------------|
| Apple / 苹果 | 🍎 | +1 length, respawns / 长度+1，刷新位置 |
| Bomb / 炸弹 | 💣 | Instant death / 吃到即死 |
| Scissors / 剪刀 | ✂️ | Clear half the bombs (Frenzy-only) / 清除一半炸弹（狂欢专属） |
| Snake body / 蛇身 | 🟩 | Green square / 绿色方块 |
| Wall / 墙 | 🟫 | Border collision = death / 撞墙即死 |

## Controls / 操作

| Key / 按键 | Action / 功能 |
|------------|---------------|
| `W` `A` `S` `D` | Move / 移动 |
| `Arrow Keys / 方向键` | Move / 移动 |
| `Q` | Pause / Resume / 暂停继续 |
| `R` | Restart / 重新开始 |
| `Esc` | Back to menu / 返回上级菜单 |

## Technical Highlights / 技术亮点

| Feature / 特性 | Implementation / 实现 |
|----------------|----------------------|
| Body storage / 蛇身存储 | Circular buffer, zero `malloc`/`free` / 循环缓冲区，零动态分配 |
| Collision / 碰撞检测 | `occupied[][]` boolean grid, O(1) / 布尔网格 |
| Frame timing / 帧率 | `GetTickCount()`, unaffected by snake length / 稳定 tick |
| Screen refresh / 刷屏 | `FillConsoleOutput` API, flicker-free / 无闪烁 |
| Apple spawn / 苹果生成 | Center-weighted random, 85% less likely near border / 距中心加权 |
| Emoji / 表情 | UTF-8 encoded / UTF-8 编码 |

## Build / 编译

```powershell
gcc -O2 -Wall snake.c -o snake.exe -lm
```

## License / 许可证

MIT License
