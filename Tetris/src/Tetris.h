#pragma once

#include "resource.h"
#include "stdafx.h"
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

constexpr int GRID = 35;
constexpr int BLOCK_SIZE = 30;       // 单个方块在屏幕上的像素边长
constexpr int nGameWidth = 15;
constexpr int nGameHeight = 25;
constexpr int nWidth = 20;
constexpr int nHeight = 27;
extern HBITMAP g_hBackgroundBitmap; // 声明背景贴图
extern HBITMAP g_hEndBitmap; // 声明游戏结束贴图

// 定义一个点，用来表示方块的位置
struct Point
{
    int x;
    int y;
};

extern int nType;                    // 下一方块类型
extern int type;                     // 当前方块类型
extern int state;                    // 当前方块状态
extern int tScore;                   // 当前得分
extern int highScore;                // 历史最高分
extern bool gameOverFlag;            // 游戏已经结束
extern bool suspendFlag;             // 暂停游戏
extern bool targetFlag;              // 启用瞄准器
extern int workRegion[25][15];       // 工作区数据，0 表示该位置没有被占用
extern Point point;                  // 当前方块的当前位置
extern Point target;                 // 当前方块目标位置，用于瞄准器
extern int bricks[15][4][4][4];
extern COLORREF BrickColor[15];

// 游戏逻辑相关函数
bool CanMoveDown();
bool CanMoveLeft();
bool CanMoveRight();
void MoveDown();
void MoveLeft();
void MoveRight();
void Rotate();
void DropDown();
void Fixing();
void DeleteOneLine(int number);
int DeleteLines();
bool GameOver();
void ComputeTarget();
void Restart();
void LoadHighScore();
void SaveHighScore();

// 新增函数声明（移到Tetris.h中，移除stdafx.h中的重复声明）
void ResetCombo();
void IncreaseCombo();
void ActivateItem(int itemType);
void DeactivateItem();
void UpdateItem();
void BombExplode(int row, int col);
void ApplyDifficulty();
void GenerateInitialRows();

// 绘图函数
void TDrawScreen(HDC hdc, HWND hWnd);

// 新增绘制函数声明
void LoadBrickBitmaps(HINSTANCE hInstance);
void DrawBrickByBitmap(HDC hdc, int type, int x, int y, int size);
void DrawCombo(HDC hdc, int combo);
void DrawItemStatus(HDC hdc, int itemType, int duration);

// 缓存字体（全程序只创建一次，避免每帧重复 CreateFont）
extern HFONT g_hFont28Bold;   // 28 粗体：标题 / 得分 / Next
extern HFONT g_hFontInstrBody;    // 操作说明正文（22 半粗）
extern HFONT g_hFontInstrTitle;   // 道具说明标题（22 粗体）
extern HFONT g_hFont80Bold;   // 80 粗体：结束 / 暂停大字
void CreateGameFonts();
void DestroyGameFonts();

// 音效函数声明
void PlayBGM();
void StopBGM();
void PlayEndSound();

// 难度选择函数声明
int ShowStartDialog(HINSTANCE hInstance, HWND hParent);
INT_PTR CALLBACK DifficultyDlgProc(HWND, UINT, WPARAM, LPARAM);

// 外部全局变量声明（从stdafx.h移过来）
extern int comboCount;
extern int maxCombo;
extern bool comboActive;
extern int currentItem;
extern int itemDuration;
extern int difficultyLevel;
extern int initialRows;
extern int dropInterval;
extern HBITMAP g_hBrickBitmaps[];
extern bool g_useBitmap;
extern HWND g_hWndMain;
extern HDC g_hBrickDC;       // 方块贴图复用的内存 DC（TetrisRender.cpp 定义，WM_DESTROY 释放）

// 道具类型宏定义（移到头文件，让所有文件都能访问）
#define ITEM_NONE            0
#define ITEM_CLEAR_BOTTOM    1
#define ITEM_SLOW            2
#define ITEM_DOUBLE_SCORE    3

// 计时器ID（主窗口与逻辑模块共用，统一定义在此）
#define TIMER_ID_DROP   1
#define TIMER_ID_ITEM   2

// 特殊方块类型宏定义
#define BOMB_TYPE  14