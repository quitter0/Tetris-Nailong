#include "stdafx.h"
#include "Tetris.h"

// 音效支持
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// 贴图相关全局变量
HBITMAP g_hBrickBitmaps[15] = { NULL };   // 每种方块的贴图（最多15种）
bool g_useBitmap = false;                 // 是否启用贴图模式（否则用纯色）

// 音效资源ID（需在资源文件中对应定义）
#define IDR_WAV_BGM           201
#define IDR_WAV_CLEAR         202

//End

void TDrawScreen(HDC hdc, HWND hWnd)
{
    
    // TODO(作业20): 完整绘制游戏界面。
    // 1. 绘制背景

    // 按固定设计尺寸(700x945)渲染到离屏缓冲，最终由调用方整体等比缩放贴到窗口，
    // 这样不受窗口实际大小影响，小屏也能完整显示
    const int designW = nWidth * GRID;
    const int designH = nHeight * GRID;
    UNREFERENCED_PARAMETER(hWnd);

    // 2. 创建内存DC和兼容位图 (双缓冲开始)
    HDC dcMem = CreateCompatibleDC(hdc);
    HBITMAP bmpMem = CreateCompatibleBitmap(hdc, designW, designH);
    HBITMAP* pOldBitmap = (HBITMAP*)SelectObject(dcMem, bmpMem);

    // --- 绘制逻辑开始 ---

    // 【背景绘制】
    // 检查全局变量 g_hBackgroundBitmap 是否有效
    if (g_hBackgroundBitmap != NULL)
    {
        // 如果有背景图，使用它（按位图实际尺寸拉伸铺满客户区）
        HDC hdcBg = CreateCompatibleDC(dcMem);
        HBITMAP hOldBg = (HBITMAP)SelectObject(hdcBg, g_hBackgroundBitmap);
        BITMAP bgBm;
        GetObject(g_hBackgroundBitmap, sizeof(bgBm), &bgBm);
        // 大幅缩小时用 HALFTONE 采样，避免默认 COLORONCOLOR 丢像素导致的锯齿/摩尔纹
        int oldStretchMode = SetStretchBltMode(dcMem, HALFTONE);
        SetBrushOrgEx(dcMem, 0, 0, NULL);
        StretchBlt(dcMem, 0, 0, designW, designH,
            hdcBg, 0, 0, bgBm.bmWidth, bgBm.bmHeight, SRCCOPY);
        SetStretchBltMode(dcMem, oldStretchMode);
        SelectObject(hdcBg, hOldBg);
        DeleteDC(hdcBg);
    }
    else
    {
        // 如果没有背景图（加载失败），填充深灰色背景
        HBRUSH hBrush = CreateSolidBrush(RGB(30, 30, 30));
        SelectObject(dcMem, hBrush);
        Rectangle(dcMem, 0, 0, designW, designH);
        DeleteObject(hBrush);
    }


    // 2. 网格和方块绘制


        // 定义网格参数
    const int blockSize = BLOCK_SIZE;  // 每个方块像素大小
    const int startX = 50;             // 工作区左偏移
    const int startY = 50;             // 工作区上偏移
    const int nextStartX = startX + 10 * blockSize + 150;  // 下一个方块显示区X
    const int nextStartY = startY;


    // 绘制工作区网格及已固定的方块
    for (int i = 0; i < 25; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            int value = workRegion[i][j];
            if (value != 0)
            {
                // 绘制已固定的方块（value 对应方块类型0~14）
                int x = startX + j * blockSize;
                int y = startY + i * blockSize;
                if (g_useBitmap && value >= 0 && value < 15 && g_hBrickBitmaps[value] != NULL)
                    DrawBrickByBitmap(dcMem, value - 1, x, y, blockSize);
                else
                {
                    HBRUSH brush = CreateSolidBrush(BrickColor[value - 1]);
                    RECT rect = { x, y, x + blockSize, y + blockSize };
                    FillRect(dcMem, &rect, brush);
                    DeleteObject(brush);
                }
            }
            // 画网格线（浅灰色）
            HPEN pen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
            HPEN oldPen = (HPEN)SelectObject(dcMem, pen);
            MoveToEx(dcMem, startX + j * blockSize, startY + i * blockSize, NULL);
            LineTo(dcMem, startX + (j + 1) * blockSize, startY + i * blockSize);
            LineTo(dcMem, startX + (j + 1) * blockSize, startY + (i + 1) * blockSize);
            LineTo(dcMem, startX + j * blockSize, startY + (i + 1) * blockSize);
            LineTo(dcMem, startX + j * blockSize, startY + i * blockSize);
            SelectObject(dcMem, oldPen);
            DeleteObject(pen);
        }
    }



    // 【方块绘制】

    int shapeType = type;   // 当前方块形状索引（0~14）
    int rotState = state;    // 旋转状态

    // 2. 绘制瞄准器（半透明灰色提示块）
    if (targetFlag && target.y >= 0)
    {
        HBRUSH brush = CreateSolidBrush(RGB(200, 200, 200));
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (bricks[shapeType][rotState][i][j] != 0)
                {
                    int x = startX + (target.x + j) * blockSize;
                    int y = startY + (target.y + i) * blockSize;
                    if (x < 0 || y < 0) continue;
                    RECT rect = { x, y, x + blockSize, y + blockSize };
                    FillRect(dcMem, &rect, brush);
                }
            }
        }
        DeleteObject(brush);
    }

    // 3. 绘制当前下落中的方块
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (bricks[shapeType][rotState][i][j] != 0)
            {
                int x = startX + (point.x + j) * blockSize;
                int y = startY + (point.y + i) * blockSize;
                if (g_useBitmap && shapeType >= 0 && shapeType < 15 && g_hBrickBitmaps[shapeType] != NULL)
                    DrawBrickByBitmap(dcMem, shapeType, x, y, blockSize);
                else
                {
                    HBRUSH brush = CreateSolidBrush(BrickColor[shapeType]);
                    RECT rect = { x, y, x + blockSize, y + blockSize };
                    FillRect(dcMem, &rect, brush);
                    DeleteObject(brush);
                }
            }
        }
    }



    // 4. 绘制“下一个方块”预览区（使用 shapeType 的索引，但通常 type 存储的是下一个方块类型）
    int nextType = nType;   // 下一个方块类型（0~14）
    int previewStartX = nextStartX;
    int previewStartY = nextStartY;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (bricks[nextType][0][i][j] != 0)   // 默认使用旋转状态0
            {
                int x = previewStartX + 20 + j * blockSize;
                int y = previewStartY + 50 + i * blockSize;
                if (g_useBitmap && nextType >= 0 && nextType < 15 && g_hBrickBitmaps[nextType] != NULL)
                    DrawBrickByBitmap(dcMem, nextType, x, y, blockSize);
                else
                {
                    HBRUSH brush = CreateSolidBrush(BrickColor[nextType]);
                    RECT rect = { x, y, x + blockSize, y + blockSize };
                    FillRect(dcMem, &rect, brush);
                    DeleteObject(brush);
                }
            }
        }
    }

    HFONT g = CreateFont(28, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"微软雅黑");
    HFONT h = (HFONT)SelectObject(dcMem, g);
    SetBkMode(dcMem, TRANSPARENT); // 改为 dcMem
    SetTextColor(dcMem, RGB(0, 0, 0)); // 改为 dcMem
    TextOut(dcMem, previewStartX + 10, previewStartY - 20, L"Next:", 5); // 改为 dcMem    SelectObject(dcMem, h);
    DeleteObject(g);

 



    // --- 绘制逻辑结束 ---

        // 5. 显示得分
    HFONT f = CreateFont(28, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"微软雅黑");
    HFONT old = (HFONT)SelectObject(dcMem, f);

    wchar_t scoreText[64];
    wsprintf(scoreText, L"Score: %d", tScore);
    SetTextColor(dcMem, RGB(255, 255, 255));
    TextOut(dcMem, startX, startY - 30, scoreText, wcslen(scoreText));

    // 显示历史最高分（当前分超过纪录时实时显示当前分）
    int bestScore = (tScore > highScore) ? tScore : highScore;
    wchar_t bestText[64];
    wsprintf(bestText, L"Best: %d", bestScore);
    SetTextColor(dcMem, RGB(255, 255, 0));
    TextOut(dcMem, startX + 180, startY - 30, bestText, wcslen(bestText));

    SelectObject(dcMem, old);
    DeleteObject(f);
    // 6. 显示连击数
    DrawCombo(dcMem, comboCount);

    // 7. 显示道具状态
    DrawItemStatus(dcMem, currentItem, itemDuration);

    // 8. 游戏结束标志 + 暂停标志
   // 第一个数字 = 字体高度（改这个调大小）
    HFONT hBigFont = CreateFont(
        80,    // 【字体大小】高度，数字越大字越大
        0,     // 宽度自适应
        0,
        0,
        FW_BOLD, // 加粗
        FALSE,
        FALSE,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        L"微软雅黑"
    );

    // 应用字体
    HFONT hOldFont = (HFONT)SelectObject(dcMem, hBigFont);


    // 游戏结束
    if (gameOverFlag && g_hEndBitmap != NULL)
    {
        SetTextColor(dcMem, RGB(0, 0, 255));
        TextOut(dcMem, startX + 0, startY + 100, L"按“R”重新开始", 8);
        PlayEndSound();

        // 1. 创建兼容DC
        HDC hdcMemEnd = CreateCompatibleDC(dcMem); // 以主内存DC为模板
        HBITMAP hOldEnd = (HBITMAP)SelectObject(hdcMemEnd, g_hEndBitmap);

        // 2. 获取图片大小 (假设你的图片是 200x100，或者你可以使用 StretchBlt 自动拉伸)
        BITMAP bm;
        GetObject(g_hEndBitmap, sizeof(bm), &bm);
        int imgWidth = bm.bmWidth;
        int imgHeight = bm.bmHeight;

        // 3. 计算居中位置 (假设游戏区域起始点为 startX, startY)
        // 这里我们让图片水平居中，垂直位置在游戏区域中间
        int imgX = startX + (nGameWidth * blockSize - imgWidth) / 2;
        int imgY = startY + (nGameHeight * blockSize - imgHeight) / 2;

        // 4. 将图片绘制到主内存DC (dcMem) 上
        // 如果你的图片有透明通道需求，这里可能需要 TransparentBlt，但普通贴图用 BitBlt 即可
        // 如果图片太大或太小，可以用 StretchBlt 自动适应
        // BitBlt(dcMem, imgX, imgY, imgWidth, imgHeight, hdcMemEnd, 0, 0, SRCCOPY);

        // 推荐使用 StretchBlt (防止图片过大撑破窗口)
        StretchBlt(dcMem, imgX, imgY, imgWidth, imgHeight, hdcMemEnd, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

        // 5. 恢复并删除临时DC
        SelectObject(hdcMemEnd, hOldEnd);
        DeleteDC(hdcMemEnd);
    }
    // 如果图片加载失败，保留一个文字备用方案（可选）
    else if (gameOverFlag)
    {
        SetTextColor(dcMem, RGB(255, 0, 0));
        SetBkMode(dcMem, TRANSPARENT);
        TextOut(dcMem, startX + 100, startY + 200, L"LOAD IMAGE FAILED", 17);
    }


    // 暂停
    if (suspendFlag)
    {
        SetTextColor(dcMem, RGB(255, 110, 0));
        TextOut(dcMem, startX + 160, startY + 200, L"PAUSE", 5);
    }



        // 释放字体
    SelectObject(dcMem, hOldFont);
    DeleteObject(hBigFont);



    // 9.【关键步骤】将内存画布 (dcMem) 上的最终图像一次性拷贝到屏幕DC (hdc)
    // 这就是双缓冲技术消除闪烁的核心
    BitBlt(hdc, 0, 0, designW, designH, dcMem, 0, 0, SRCCOPY);


    // 10. 清理内存DC的资源 (双缓冲结束)
    SelectObject(dcMem, pOldBitmap);
    DeleteObject(bmpMem);
    DeleteDC(dcMem);
}

// ---------- 贴图相关实现 ----------

// 用 HALFTONE 采样把贴图缩放到指定尺寸，返回新位图（原图被释放）。
// 方块贴图 521x520、炸弹贴图 2000x2095，若绘制时直接用默认的最近邻采样
// 缩到 30x30（缩 17~67 倍）会严重锯齿/丢细节，故在加载阶段一次性高质量缩放，
// 绘制时即 1:1 贴图——与背景优化同一思路。
static HBITMAP ScaleBitmap(HBITMAP src, int width, int height)
{
    if (src == NULL)
        return NULL;

    BITMAP bm;
    GetObject(src, sizeof(bm), &bm);
    if (bm.bmWidth == width && bm.bmHeight == height)
        return src;

    // 两个 GDI 陷阱，任一处写错贴图都会变灰：
    // 1) 目的位图要用真正的屏幕 DC 作模板——CreateCompatibleDC(NULL) 初始选中的是
    //    1x1 单色位图，拿它 CreateCompatibleBitmap 只能得到 1bpp 单色位图；
    // 2) 源位图要选进一个内存 DC，不能直接选进屏幕 DC，否则 StretchBlt 会破坏颜色。
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
    HBITMAP dst = CreateCompatibleBitmap(hdcScreen, width, height);
    HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcSrc, src);
    HBITMAP hOldDst = (HBITMAP)SelectObject(hdcMem, dst);

    SetStretchBltMode(hdcMem, HALFTONE);
    SetBrushOrgEx(hdcMem, 0, 0, NULL);
    StretchBlt(hdcMem, 0, 0, width, height, hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    SelectObject(hdcMem, hOldDst);
    SelectObject(hdcSrc, hOldSrc);
    DeleteDC(hdcSrc);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(src);
    return dst;
}

void LoadBrickBitmaps(HINSTANCE hInstance)
{
    // 定义15个资源ID，请根据实际资源编号修改
    // 若某些贴图不存在，可暂时注释掉对应条目，或设资源ID为0略过
    int resIds[15] = {
        IDB_BRICK1,  IDB_BRICK2,  IDB_BRICK3,  IDB_BRICK4,
        IDB_BRICK5,  IDB_BRICK6,  IDB_BRICK1,  IDB_BRICK2,  
        IDB_BRICK3,  IDB_BRICK4,  IDB_BRICK5,  IDB_BRICK6,
        IDB_BRICK1,  IDB_BRICK2,  IDB_BRICK14
    };
    bool allLoaded = true;
    for (int i = 0; i < 15; i++)
    {
        if (g_hBrickBitmaps[i])
            DeleteObject(g_hBrickBitmaps[i]);
        if (resIds[i] != 0)
        {
            g_hBrickBitmaps[i] = LoadBitmap(hInstance, MAKEINTRESOURCE(resIds[i]));
            if (g_hBrickBitmaps[i] != NULL)
                g_hBrickBitmaps[i] = ScaleBitmap(g_hBrickBitmaps[i], BLOCK_SIZE, BLOCK_SIZE);
        }
        if (g_hBrickBitmaps[i] == NULL)
            allLoaded = false;
    }
    g_useBitmap = allLoaded;   // 只有全部成功才启用贴图模式
}

// ---------- 绘制与音效函数框架 ----------


void DrawBrickByBitmap(HDC hdc, int type, int x, int y, int size)
{
    if (type < 0 || type >= 15 || g_hBrickBitmaps[type] == NULL)
        return;
    HDC memDC = CreateCompatibleDC(hdc);
    SelectObject(memDC, g_hBrickBitmaps[type]);
    BITMAP bm;
    GetObject(g_hBrickBitmaps[type], sizeof(bm), &bm);
    StretchBlt(hdc, x, y, size, size, memDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
    DeleteDC(memDC);
}

void DrawCombo(HDC hdc, int combo)
{
    if (combo >= 0)
    {
        wchar_t comboText[64];
        wsprintf(comboText, L"Combo: x%d", combo);
        SetTextColor(hdc, RGB(255, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, 100, 850, comboText, wcslen(comboText));
    }
}

void DrawItemStatus(HDC hdc, int itemType, int duration)
{
    if (itemType != ITEM_NONE && duration > 0)
    {
        wchar_t itemText[128];
        const wchar_t* itemName = L"";
        switch (itemType)
        {
        case ITEM_CLEAR_BOTTOM: itemName = L"Clear Bottom"; break;
        case ITEM_SLOW:         itemName = L"Slow"; break;
        case ITEM_DOUBLE_SCORE: itemName = L"Double Score"; break;
        }
        wsprintf(itemText, L"Item: %s (%ds)", itemName, duration);
        SetTextColor(hdc, RGB(0, 0, 255));
        TextOut(hdc, 220, 20, itemText, wcslen(itemText));
    }
}



void PlayBGM()
{
    // TODO(音效): 循环播放背景音乐
    PlaySound(MAKEINTRESOURCE(IDR_WAV_BGM), GetModuleHandle(NULL), SND_RESOURCE | SND_LOOP | SND_ASYNC);
}

void StopBGM()
{
    // TODO(音效): 停止背景音乐
    PlaySound(NULL, NULL, 0);
}


void PlayEndSound()
{
    // TODO(音效): 播放结束音效
    PlaySound(MAKEINTRESOURCE(IDR_WAV_CLEAR), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
}
