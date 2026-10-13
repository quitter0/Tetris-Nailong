#include "stdafx.h"
#include "Tetris.h"
#include <cstdlib>  // rand()
#include <ctime>    // time()
#include <cstdio>   // fopen_s / fread / fwrite
#include <string.h> // strrchr

// ---------- 全局变量定义 ----------
int nType = 0;           // 下一方块类型
int type = 0;            // 当前方块类型
int state = 0;           // 当前方块状态（旋转状态0-3）
int tScore = 0;          // 当前得分
int highScore = 0;       // 历史最高分
bool gameOverFlag = false;
bool suspendFlag = false;
bool targetFlag = false;
int workRegion[25][15] = { 0 };
Point point = { 0, 0 }; // 当前方块左上角在工作区的位置（行, 列）
Point target = { 0, 0 }; // 瞄准器预测落点
int bricks[15][4][4][4] =
{
    {
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}},
        {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}},
        {{0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}, {0, 0, 1, 0}}
    },//长条
    {
        {{0, 0, 0, 0}, {0, 0, 2, 0}, {0, 2, 2, 2}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 0, 2, 0}, {0, 0, 2, 2}, {0, 0, 2, 0}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 2, 2, 2}, {0, 0, 2, 0}},
        {{0, 0, 0, 0}, {0, 0, 2, 0}, {0, 2, 2, 0}, {0, 0, 2, 0}}
    },//短T形
    {
        {{0, 3, 0, 0}, {0, 3, 0, 0}, {0, 3, 3, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {3, 3, 3, 0}, {3, 0, 0, 0}, {0, 0, 0, 0}},
        {{3, 3, 0, 0}, {0, 3, 0, 0}, {0, 3, 0, 0}, {0, 0, 0, 0}},
        {{0, 0, 3, 0}, {3, 3, 3, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
    },//L形
    {
        {{0, 0, 4, 0}, {0, 0, 4, 0}, {0, 4, 4, 0}, {0, 0, 0, 0}},
        {{0, 4, 0, 0}, {0, 4, 4, 4}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        {{0, 0, 4, 4}, {0, 0, 4, 0}, {0, 0, 4, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 4, 4, 4}, {0, 0, 0, 4}, {0, 0, 0, 0}}
    },//J形
    {
        {{0, 0, 0, 0}, {0, 5, 5, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 5, 5, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 5, 5, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 5, 5, 0}, {0, 5, 5, 0}, {0, 0, 0, 0}}
    },//方块
    {
        {{0, 6, 0, 0}, {0, 6, 6, 0}, {0, 0, 6, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 6, 6, 0}, {6, 6, 0, 0}, {0, 0, 0, 0}},
        {{6, 0, 0, 0}, {6, 6, 0, 0}, {0, 6, 0, 0}, {0, 0, 0, 0}},
        {{0, 6, 6, 0}, {6, 6, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}
    },//小s形
    {
        {{0, 0, 7, 0}, {0, 7, 7, 0}, {0, 7, 0, 0}, {0, 0, 0, 0}},
        {{0, 7, 7, 0}, {0, 0, 7, 7}, {0, 0, 0, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 7}, {0, 0, 7, 7}, {0, 0, 7, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 7, 7, 0}, {0, 0, 7, 7}, {0, 0, 0, 0}}
    },//小z形
    {
        {{0, 0, 0, 0}, {0, 8, 8, 8}, {0, 8, 0, 8}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 0, 8, 8}, {0, 0, 0, 8}, {0, 0, 8, 8}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 8, 0, 8}, {0, 8, 8, 8}},
        {{0, 0, 0, 0}, {0, 8, 8, 0}, {0, 8, 0, 0}, {0, 8, 8, 0}}
    },//U形
    {
        {{0, 0, 0, 0}, {0, 0, 9, 0}, {0, 9, 9, 9}, {0, 0, 9, 0}},
        {{0, 0, 0, 0}, {0, 0, 9, 0}, {0, 9, 9, 9}, {0, 0, 9, 0}},
        {{0, 0, 0, 0}, {0, 0, 9, 0}, {0, 9, 9, 9}, {0, 0, 9, 0}},
        {{0, 0, 0, 0}, {0, 0, 9, 0}, {0, 9, 9, 9}, {0, 0, 9, 0}}
    },//十形
    {
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 10, 10}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 10}, {0, 0, 0, 10}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 10, 10}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 10}, {0, 0, 0, 10}}
    },//1*2
    {
        {{0, 0, 0, 0}, {0, 0, 11, 11}, {0, 0, 11, 0}, {0, 11, 11, 0}},
        {{0, 0, 0, 0}, {0, 11, 0, 0}, {0, 11, 11, 11}, {0, 0, 0, 11}},
        {{0, 0, 0, 0}, {0, 0, 11, 11}, {0, 0, 11, 0}, {0, 11, 11, 0}},
        {{0, 0, 0, 0}, {0, 11, 0, 0}, {0, 11, 11, 11}, {0, 0, 0, 11}}
    },//大S形
    {
        {{0, 0, 0, 0}, {0, 12, 12, 0}, {0, 0, 12, 0}, {0, 0, 12, 12}},
        {{0, 0, 0, 0}, {0, 0, 0, 12}, {0, 12, 12, 12}, {0, 12, 0, 0}},
        {{0, 0, 0, 0}, {0, 12, 12, 0}, {0, 0, 12, 0}, {0, 0, 12, 12}},
        {{0, 0, 0, 0}, {0, 0, 0, 12}, {0, 12, 12, 12}, {0, 12, 0, 0}}
    },//大Z形
    {
        {{0, 0, 0, 0}, {0, 0, 0, 13}, {0, 13, 13, 13}, {0, 0, 0, 13}},
        {{0, 0, 0, 0}, {0, 0, 13, 0}, {0, 0, 13, 0}, {0, 13, 13, 13}},
        {{0, 0, 0, 0}, {0, 13, 0, 0}, {0, 13, 13, 13}, {0, 13, 0, 0}},
        {{0, 0, 0, 0}, {0, 13, 13, 13}, {0, 0, 13, 0}, {0, 0, 13, 0}}
    },//大T形
    {
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 14}, {0, 0, 14, 14}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 14, 0}, {0, 0, 14, 14}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 14, 14}, {0, 0, 14, 0}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 14, 14}, {0, 0, 0, 14}}
    },//小三角
    {
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 15, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 15, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 15, 0}, {0, 0, 0, 0}},
        {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 15, 0}, {0, 0, 0, 0}}
    }//炸弹
};

COLORREF BrickColor[15] =
{
    RGB(255, 0, 0),     // 0: 长条 - 红色
    RGB(0, 255, 0),     // 1: 短T形 - 绿色
    RGB(0, 0, 255),     // 2: L形 - 蓝色
    RGB(0, 255, 255),   // 3: J形 - 青色
    RGB(255, 150, 0),   // 4: 方块 - 橙色
    RGB(255, 255, 0),   // 5: 小s形 - 黄色
    RGB(168, 0, 168),   // 6: 小z形 - 紫色
    RGB(255, 105, 180), // 7: U形 - 亮粉红
    RGB(128, 0, 128),   // 8: 十形 - 深紫
    RGB(255, 165, 0),   // 9: 1*2 - 橙黄
    RGB(0, 128, 128),   // 10: 大S形 - 深青
    RGB(255, 20, 147),  // 11: 大Z形 - 深粉红
    RGB(128, 128, 0),   // 12: 大T形 - 橄榄绿
    RGB(70, 130, 180),  // 13: 小三角 - 钢蓝
    RGB(255, 255, 255)  // 14: 炸弹 - 白色
};

// ---------- 连击系统 ----------
int comboCount = 0;           // 当前连击数
int maxCombo = 0;             // 本局最大连击数
bool comboActive = false;     // 是否处于连击状态

// ---------- 道具系统 ----------
// ITEM_* 与 TIMER_ID_* 宏定义统一在 Tetris.h，此处不再重复

int currentItem = ITEM_NONE;
int itemDuration = 0;

// ---------- 难度与初始行 ----------
int difficultyLevel = 1;      // 0:简单 1:普通 2:困难
int initialRows = 0;          // 初始随机填充行数
int dropInterval = 500;       // 下落间隔（毫秒）

// ---------- 特殊方块类型编号 ----------
#define EASY_BRICK_TYPES 7    // 总共7种简单方块类型
#define TOTAL_BRICK_TYPES 15  // 总共15种方块类型
// BOMB_TYPE 宏定义统一在 Tetris.h，此处不再重复

// ---------- 生成随机方块类型 ----------
int RandomBrickType()
{
    // 困难模式下炸弹出现概率4%，简单模式15%，普通模式10%
    int r = rand() % 100;
    if (difficultyLevel == 2 && r < 4)
        return BOMB_TYPE;
    else if (difficultyLevel == 1 && r < 10)
        return BOMB_TYPE;
    else if (difficultyLevel == 0 && r < 15)
        return BOMB_TYPE;
    else if (difficultyLevel == 0)
        return rand() % (EASY_BRICK_TYPES); // 0-6（不含炸弹）
    else
        return rand() % (BOMB_TYPE);
}

// ---------- 检查方块在指定位置是否合法 ----------
bool IsPositionValid(int brickType, int brickState, int posY, int posX)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (bricks[brickType][brickState][i][j] != 0)
            {
                int newY = posY + i;
                int newX = posX + j;

                // 检查边界
                if (newX < 0 || newX >= nGameWidth || newY >= nGameHeight)
                    return false;

                // 检查是否与已固定的方块重叠（newY < 0 表示上方，允许）
                if (newY >= 0 && workRegion[newY][newX] != 0)
                    return false;
            }
        }
    }
    return true;
}

// ---------- 方块移动判断 ----------
bool CanMoveDown()
{
    return IsPositionValid(type, state, point.y + 1, point.x);
}

bool CanMoveLeft()
{
    return IsPositionValid(type, state, point.y, point.x - 1);
}

bool CanMoveRight()
{
    return IsPositionValid(type, state, point.y, point.x + 1);
}

// ---------- 方块移动 ----------
void MoveDown()
{
    point.y++;
}

void MoveLeft()
{
    point.x--;
}

void MoveRight()
{
    point.x++;
}

// ---------- 旋转 ----------
void Rotate()
{
    int newState = (state + 1) % 4;

    // 尝试旋转，如果不行尝试向左/向右偏移（墙踢）
    if (IsPositionValid(type, newState, point.y, point.x))
    {
        state = newState;
    }
    else if (IsPositionValid(type, newState, point.y, point.x - 1))
    {
        state = newState;
        point.x--;
    }
    else if (IsPositionValid(type, newState, point.y, point.x + 1))
    {
        state = newState;
        point.x++;
    }
    else if (IsPositionValid(type, newState, point.y, point.x - 2))
    {
        state = newState;
        point.x -= 2;
    }
    else if (IsPositionValid(type, newState, point.y, point.x + 2))
    {
        state = newState;
        point.x += 2;
    }
}

// ---------- 一键到底 ----------
void DropDown()
{
    while (CanMoveDown())
    {
        MoveDown();
    }
}

// ---------- 固定方块到工作区 ----------
void Fixing()
{
    // 将当前方块写入工作区
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (bricks[type][state][i][j] != 0)
            {
                int newY = point.y + i;
                int newX = point.x + j;
                if (newY >= 0 && newY < nGameHeight && newX >= 0 && newX < nGameWidth)
                {
                    workRegion[newY][newX] = type + 1;
                }
            }
        }
    }

    // 生成下一个方块
    type = nType;
    nType = RandomBrickType();
    state = 0;

    // 初始位置：上方中间
    point.x = nGameWidth / 2 - 2;
    point.y = -2;  // 从顶部上方出现

    // 如果初始位置不合法，尝试上移
    while (!IsPositionValid(type, state, point.y, point.x) && point.y < 0)
    {
        point.y++;
    }

    // 如果仍然不合法（顶部有方块），游戏结束
    if (!IsPositionValid(type, state, point.y, point.x))
    {
        gameOverFlag = true;
    }

    if (targetFlag)
        ComputeTarget();
}

// ---------- 删除一行 ----------
void DeleteOneLine(int number)
{
    // 从number行开始，将所有上方行向下移动一行
    for (int i = number; i > 0; i--)
    {
        for (int j = 0; j < nGameWidth; j++)
        {
            workRegion[i][j] = workRegion[i - 1][j];
        }
    }
    // 最顶行清零
    for (int j = 0; j < nGameWidth; j++)
    {
        workRegion[0][j] = 0;
    }
}

// ---------- 消行逻辑 ----------
void DeleteLines()
{
    int linesCleared = 0;

    // 从下往上检查每一行
    for (int i = nGameHeight - 1; i >= 0; i--)
    {
        bool fullLine = true;
        for (int j = 0; j < nGameWidth; j++)
        {
            if (workRegion[i][j] == 0)
            {
                fullLine = false;
                break;
            }
        }

        if (fullLine)
        {
            DeleteOneLine(i);
            linesCleared++;
            i++; // 重新检查当前行（因为上面的行移下来了）
        }
    }

    // 计分逻辑
    if (linesCleared > 0)
    {
        // 基础分：1行100, 2行200, 3行400, 4行800
        int baseScore = 0;
        switch (linesCleared)
        {
        case 1: baseScore = 100; break;
        case 2: baseScore = 200; break;
        case 3: baseScore = 400; break;
        case 4: baseScore = 800; break;
        }

        // 道具双倍得分
        if (currentItem == ITEM_DOUBLE_SCORE)
            baseScore *= 2;

        // 连击加成
        baseScore += comboCount * 50;

        tScore += baseScore;
    }
}

// ---------- 游戏结束判断 ----------
bool GameOver()
{
    // 检查顶部一行是否有方块
    for (int j = 0; j < nGameWidth; j++)
    {
        if (workRegion[0][j] != 0)
            return true;
    }
    return gameOverFlag;
}

// ---------- 瞄准器计算 ----------
void ComputeTarget()
{
    target = point;
    // 模拟下落直到无法继续
    Point tempPoint = point;
    while (IsPositionValid(type, state, tempPoint.y + 1, tempPoint.x))
    {
        tempPoint.y++;
    }
    target = tempPoint;
    if (target.y < 0) target.y = 0;   // 保证瞄准器始终可见
}

// ---------- 重置游戏 ----------
void Restart()
{
    // 清空工作区
    for (int i = 0; i < nGameHeight; i++)
    {
        for (int j = 0; j < nGameWidth; j++)
        {
            workRegion[i][j] = 0;
        }
    }

    // 重置状态
    tScore = 0;
    gameOverFlag = false;
    suspendFlag = false;
    comboCount = 0;
    maxCombo = 0;
    comboActive = false;
    currentItem = ITEM_NONE;
    itemDuration = 0;

    // 应用难度
    ApplyDifficulty();

    // 生成初始随机行
    GenerateInitialRows();

    // 初始化随机种子
    srand((unsigned int)time(NULL));

    // 生成方块
    type = rand() % BOMB_TYPE;   // 0 到 13，不包含炸弹
    nType = rand() % BOMB_TYPE;
    state = 0;

    // 初始位置
    point.x = nGameWidth / 2 - 2;
    point.y = 0;

    // 确保初始方块合法
    if (!IsPositionValid(type, state, point.y, point.x))
    {
        gameOverFlag = true;
    }

    if (targetFlag)
        ComputeTarget();
}



// ---------- 连击系统 ----------
void ResetCombo()
{
    comboCount = 0;
    comboActive = false;
}

void IncreaseCombo()
{
    comboCount++;
    comboActive = true;
    if (comboCount > maxCombo)
        maxCombo = comboCount;
}

// ---------- 道具系统 ----------
void ActivateItem(int itemType)
{
    // 如果已有道具，先清除
    if (currentItem != ITEM_NONE)
        DeactivateItem();

    currentItem = itemType;

    switch (itemType)
    {
    case ITEM_CLEAR_BOTTOM:
        // 立即清除底部一行
        for (int j = 0; j < nGameWidth; j++)
        {
            workRegion[nGameHeight - 1][j] = 0;
        }
        // 将上方所有行下移
        for (int i = nGameHeight - 1; i > 0; i--)
        {
            for (int j = 0; j < nGameWidth; j++)
            {
                workRegion[i][j] = workRegion[i - 1][j];
            }
        }
        for (int j = 0; j < nGameWidth; j++)
        {
            workRegion[0][j] = 0;
        }
        tScore += 50; // 使用道具加分
        currentItem = ITEM_NONE; // 一次性道具
        break;

    case ITEM_SLOW:
        itemDuration = 10; // 持续10秒
        // 临时降低下落速度
        dropInterval = dropInterval * 2;

        // 立即更新计时器间隔
        if (!gameOverFlag && !suspendFlag) // 只有游戏运行中才设置
        {
            SetTimer(g_hWndMain, TIMER_ID_DROP, dropInterval, NULL);
        }
        break;

    case ITEM_DOUBLE_SCORE:
        itemDuration = 15; // 持续15秒
        break;
    }
}

void DeactivateItem()
{
    switch (currentItem)
    {
    case ITEM_SLOW:
        // 恢复原速度
        ApplyDifficulty();

        // 立即恢复计时器间隔
        if (!gameOverFlag && !suspendFlag) // 只有游戏运行中才设置
        {
            SetTimer(g_hWndMain, TIMER_ID_DROP, dropInterval, NULL);
        }
        break;
    case ITEM_DOUBLE_SCORE:
        // 无特殊恢复操作
        break;
    }
    currentItem = ITEM_NONE;
    itemDuration = 0;
}

void UpdateItem()
{
    if (itemDuration > 0)
    {
        itemDuration--;
        if (itemDuration <= 0)
        {
            DeactivateItem();
        }
    }
}

// ---------- 炸弹爆炸 ----------
void BombExplode(int row, int col)
{
    // 1. 消除 5x5 区域（边界安全）
    for (int dy = -2; dy <= 2; dy++)
    {
        for (int dx = -2; dx <= 2; dx++)
        {
            int y = row + dy;
            int x = col + dx;
            if (y >= 0 && y < nGameHeight && x >= 0 && x < nGameWidth)
            {
                workRegion[y][x] = 0;
            }
        }
    }

    // 2. 紧凑排列所有行 —— 从下往上，非空行沉底
    int writeRow = nGameHeight - 1;   // 下一个要写入的行（从最底部开始）
    for (int readRow = nGameHeight - 1; readRow >= 0; readRow--)
    {
        // 判断当前扫描行是否有方块
        bool hasBlock = false;
        for (int j = 0; j < nGameWidth; j++)
        {
            if (workRegion[readRow][j] != 0)
            {
                hasBlock = true;
                break;
            }
        }

        if (hasBlock)
        {
            // 如果读行与写行不同，把读行复制到写行
            if (readRow != writeRow)
            {
                for (int j = 0; j < nGameWidth; j++)
                {
                    workRegion[writeRow][j] = workRegion[readRow][j];
                }
            }
            writeRow--;   // 写指针上移
        }
    }

    // 3. 清空上方剩余的行（writeRow 及以上的所有行）
    for (int i = 0; i <= writeRow; i++)
    {
        for (int j = 0; j < nGameWidth; j++)
        {
            workRegion[i][j] = 0;
        }
    }

    tScore += 30;
}

// ---------- 难度应用 ----------
void ApplyDifficulty()
{
    switch (difficultyLevel)
    {
    case 0: // 简单
        dropInterval = 800;
        initialRows = 0;
        break;
    case 1: // 普通
        dropInterval = 500;
        initialRows = 3;
        break;
    case 2: // 困难
        dropInterval = 300;
        initialRows = 6;
        break;
    default:
        dropInterval = 500;
        initialRows = 3;
        break;
    }
}

// ---------- 生成初始随机行 ----------
void GenerateInitialRows()
{
    if (initialRows <= 0)
        return;

    // 从底部开始填充
    for (int row = nGameHeight - 1; row >= nGameHeight - initialRows; row--)
    {
        // 每行随机填充，但留一些空隙
        int fillCount = rand() % 5 + 5; // 5-9个方块
        int attempts = 0;
        while (fillCount > 0 && attempts < 20)
        {
            int col = rand() % nGameWidth;
            if (workRegion[row][col] == 0)
            {
                workRegion[row][col] = rand() % (BOMB_TYPE) + 1; // 1-BOMB_TYPE之间的值
                fillCount--;
            }
            attempts++;
        }
    }
}

// ---------- 最高分持久化 ----------
// 返回 highscore.dat 的完整路径：定位到 exe 所在目录，避免受当前工作目录影响
static void GetHighScorePath(char* out, size_t outSize)
{
    char exePath[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0)
    {
        _snprintf_s(out, outSize, _TRUNCATE, "highscore.dat");   // 兜底：退回相对路径
        return;
    }
    char* slash = strrchr(exePath, '\\');   // 截掉文件名，保留目录
    if (slash != NULL)
        *(slash + 1) = '\0';
    else
        exePath[0] = '\0';
    _snprintf_s(out, outSize, _TRUNCATE, "%shighscore.dat", exePath);
}

void LoadHighScore()
{
    highScore = 0;
    char path[MAX_PATH];
    GetHighScorePath(path, MAX_PATH);
    FILE* fp = nullptr;
    if (fopen_s(&fp, path, "rb") == 0 && fp != nullptr)
    {
        if (fread(&highScore, sizeof(int), 1, fp) != 1)
            highScore = 0;   // 旧文件或损坏，归零
        fclose(fp);
    }
    if (highScore < 0)
        highScore = 0;
}

void SaveHighScore()
{
    if (tScore <= highScore)
        return;

    highScore = tScore;
    char path[MAX_PATH];
    GetHighScorePath(path, MAX_PATH);
    FILE* fp = nullptr;
    if (fopen_s(&fp, path, "wb") == 0 && fp != nullptr)
    {
        fwrite(&highScore, sizeof(int), 1, fp);
        fclose(fp);
    }
}