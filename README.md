# Snake Game — Dual-Mode Emoji Terminal Edition

> **This project is 100% AI-generated** using [Claude Code](https://claude.ai/code).
> All code, documentation, and design were produced through human-AI collaboration.

> C Language + Win32 Console | Circular Buffer | Zero Dynamic Allocation | O(1) All Operations

## Layout

```
+--------------------------------------+
|           S N A K E                  |
+--------------------------------------+
  [Brown border enclosing a 20x20 grid]
  Items: Apple, Bombs, Scissors
  Snake: Green squares

  Length: 3      [Easy]     Bombs: 1
  WASD to move | Q to pause | R to reset
```

> For best visual results, run in a terminal. Emoji widths may vary in web browsers.

## Menu Flow

```
Main Menu → Game Mode → Difficulty → Play
```

## Two Game Modes

| Mode | Bombs | Scissors | Description |
|------|:-----:|:--------:|-------------|
| **Classic** | No | No | Traditional snake, eat apples to grow |
| **Bomb Frenzy** | Yes | Yes | Bombs never disappear, they keep piling up! |

## Difficulty System

| Difficulty | Speed | Classic + Hard | Frenzy + Hard |
|------------|:----:|----------------|---------------|
| **Easy** | 150ms | No bombs | Bomb interval 5→1 (decreasing) |
| **Hard** | 80ms | 1 bomb every 5 apples | Bomb interval 3→1, double spawn! Scissors more frequent |

## Items

| Item | Icon | Description |
|------|:----:|-------------|
| Apple | 🍎 | +1 length, respawns elsewhere |
| Bomb | 💣 | Instant death! In Frenzy mode, bombs **never disappear** |
| Scissors | ✂️ | Clears **half** the bombs on screen (Frenzy-only) |
| Snake | 🟩 | Green square body, starts at length 1 |
| Wall | 🟫 | Brown border, collision = death |

## Controls

| Key | Action |
|-----|--------|
| `W` `A` `S` `D` | Move direction |
| `Arrow Keys` | Move direction |
| `Q` | Pause / Resume |
| `R` | Restart |
| `Esc` | Return to previous menu |

## Death / Victory

- 💥 **Hit the wall** — touched the border
- 💥 **Hit yourself** — head collided with body
- 💥 **Bomb** — ate a bomb
- 🎉 **Victory** — snake filled the entire 20x20 = 400 cells!

## Technical Highlights

| Feature | Implementation |
|---------|---------------|
| Body storage | Circular buffer array, zero `malloc`/`free` |
| Collision detection | `occupied[][]` boolean grid, O(1) |
| Frame timing | `GetTickCount()` stable tick, unaffected by snake length |
| Screen refresh | `FillConsoleOutput` API, flicker-free |
| Apple spawning | Center-weighted random distribution (border cells 85% less likely) |
| Emoji rendering | UTF-8 encoded, full emoji visuals |

## Build

```powershell
# Requires GCC (MinGW-w64)
gcc -O2 -Wall snake.c -o snake.exe -lm
```

## License

MIT License
