// Tetris.cpp : 程序入口、窗口初始化与消息处理

#include "stdafx.h"
#include "Tetris.h"
#include <mmsystem.h>

// 对话框资源ID 见 resource.h，计时器ID 见 Tetris.h，此处不再重复定义

// 对话框过程声明
INT_PTR CALLBACK DifficultyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// 难度选择函数
int ShowStartDialog(HINSTANCE hInstance, HWND hParent)
{
    int result = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIFFICULTY_DLG), hParent, DifficultyDlgProc);
    if (result == -1)
        return 1; // 默认普通难度
    return result;
}

// 难度对话框过程
INT_PTR CALLBACK DifficultyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
    {
        // 默认选择"普通"
        CheckRadioButton(hDlg, IDC_RADIO_EASY, IDC_RADIO_HARD, IDC_RADIO_NORMAL);
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);
        switch (id)
        {
        case IDOK:
        {
            // 确定：读取选中的难度
            if (IsDlgButtonChecked(hDlg, IDC_RADIO_EASY) == BST_CHECKED)
                difficultyLevel = 0;
            else if (IsDlgButtonChecked(hDlg, IDC_RADIO_HARD) == BST_CHECKED)
                difficultyLevel = 2;
            else
                difficultyLevel = 1; // 普通

            EndDialog(hDlg, difficultyLevel);
            return (INT_PTR)TRUE;
        }

        case IDCANCEL:
        {
            difficultyLevel = 1; // 取消默认普通
            EndDialog(hDlg, 1);
            return (INT_PTR)TRUE;
        }
        }
        break;
    }
    }
    return (INT_PTR)FALSE;
}


#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
TCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HBITMAP g_hBackgroundBitmap = NULL;             // 定义背景贴图
HBITMAP g_hEndBitmap = NULL;                    // 定义游戏结束贴图

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 全局变量：主窗口句柄（供逻辑模块使用）
HWND g_hWndMain = NULL;

int APIENTRY _tWinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_TETRIS, szWindowClass, MAX_LOADSTRING);

    // 注册窗口类
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    // 主消息循环:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TETRIS));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCE(IDC_TETRIS);  // 如果有菜单资源
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中

    // 显示难度选择对话框
    int diffResult = ShowStartDialog(hInstance, NULL);
    difficultyLevel = diffResult;
    ApplyDifficulty();  // 应用难度设置

    // 计算窗口大小（根据游戏区域），并按屏幕可用区自适应缩放
    const int designClientW = nWidth * GRID;    // 设计客户区 700
    const int designClientH = nHeight * GRID;   // 设计客户区 945

    // 设计尺寸下窗口的非客户区(标题栏/菜单/边框)大小
    RECT designWin = { 0, 0, designClientW, designClientH };
    AdjustWindowRect(&designWin, WS_OVERLAPPEDWINDOW, TRUE);
    int ncW = (designWin.right - designWin.left) - designClientW;
    int ncH = (designWin.bottom - designWin.top) - designClientH;

    // 屏幕可用区（避开任务栏）
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    int workW = workArea.right - workArea.left;
    int workH = workArea.bottom - workArea.top;

    // 计算最大等比缩放系数（上限 1.0，大屏保持原始大小）
    double scale = 1.0;
    double sx = (double)(workW - ncW) / designClientW;
    double sy = (double)(workH - ncH) / designClientH;
    if (sx < scale) scale = sx;
    if (sy < scale) scale = sy;
    if (scale <= 0.0) scale = 1.0;

    int clientW = (int)(designClientW * scale + 0.5);
    int clientH = (int)(designClientH * scale + 0.5);

    RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = clientW;
    rect.bottom = clientH;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
    int winW = rect.right - rect.left;
    int winH = rect.bottom - rect.top;

    // 窗口居中于工作区
    int winX = workArea.left + (workW - winW) / 2;
    int winY = workArea.top + (workH - winH) / 2;
    if (winX < workArea.left) winX = workArea.left;
    if (winY < workArea.top) winY = workArea.top;

    HWND hWnd = CreateWindowW(
        szWindowClass,
        szTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        winX, winY, winW, winH,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd)
    {
        return FALSE;
    }

    g_hWndMain = hWnd;

    // 加载方块贴图（可选）
    LoadBrickBitmaps(hInstance);


    // 加载背景图片（从内嵌资源加载，无需外部文件）
    g_hBackgroundBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BACKGROUND));
    if (!g_hBackgroundBitmap) {
        MessageBox(hWnd, TEXT("背景图片加载失败！"), TEXT("错误"), MB_OK);
    }

    // 加载游戏结束图片
    g_hEndBitmap = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_GAMEOVER));
    if (!g_hEndBitmap) {
        MessageBox(hWnd, TEXT("Game Over 图片加载失败！"), TEXT("错误"), MB_OK);
    }

    // 初始化游戏状态
    Restart();

    // 读取历史最高分
    LoadHighScore();

    // 启动下落计时器
    SetTimer(hWnd, TIMER_ID_DROP, dropInterval, NULL);
    // 启动道具更新计时器
    SetTimer(hWnd, TIMER_ID_ITEM, 1000, NULL);  // 每秒更新道具状态

    // 播放背景音乐
    PlayBGM();

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

// 当前方块落定时的统一处理：固定方块 → 炸弹/消行/连击结算 → 游戏结束判定 → 刷新瞄准器与画面。
// 原先 WM_TIMER（自然下落到底）与 VK_SPACE（空格直落）两处逐字重复，此处置于一处避免逻辑漂移。
static void LockCurrentBrick(HWND hWnd)
{
    int oldType = type;        // 保存固定前的方块类型/状态/位置
    int oldState = state;
    Point oldPoint = point;
    Fixing();                  // 固定并生成下一块（type 此时已变为下一个）

    if (oldType == BOMB_TYPE)
    {
        // 查找炸弹方块的第一个有效格子作为爆炸中心
        bool bombFound = false;
        int bombRow = 0, bombCol = 0;
        for (int i = 0; i < 4 && !bombFound; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if (bricks[oldType][oldState][i][j] != 0)
                {
                    bombRow = oldPoint.y + i;
                    bombCol = oldPoint.x + j;
                    bombFound = true;
                    break;
                }
            }
        }
        if (bombFound)
            BombExplode(bombRow, bombCol);

        DeleteLines();         // 爆炸后处理产生的空行
    }
    else
    {
        // 普通方块的消行统计与连击
        int linesCleared = 0;
        for (int i = 0; i < nGameHeight; i++)
        {
            bool full = true;
            for (int j = 0; j < nGameWidth; j++)
            {
                if (workRegion[i][j] == 0)
                {
                    full = false;
                    break;
                }
            }
            if (full) linesCleared++;
        }
        DeleteLines();

        if (linesCleared > 0)
            IncreaseCombo();
        else
            ResetCombo();
    }

    if (GameOver())
    {
        gameOverFlag = true;
        KillTimer(hWnd, TIMER_ID_DROP);
        StopBGM();
        SaveHighScore();
    }

    if (targetFlag) ComputeTarget();
    InvalidateRect(hWnd, NULL, FALSE);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_TIMER:
    {
        if (wParam == TIMER_ID_DROP && !gameOverFlag && !suspendFlag)
        {
            if (CanMoveDown())
            {
                MoveDown();                   // 仅下移一格
                if (targetFlag) ComputeTarget();
                InvalidateRect(hWnd, NULL, FALSE);
            }
            else
            {
                LockCurrentBrick(hWnd);       // 无法下落，固定方块并结算
            }
        }
        // 道具计时器
        if (wParam == TIMER_ID_ITEM)
        {
            UpdateItem();
        }
        break;
    }
    case WM_KEYDOWN:
    {
        if (gameOverFlag)
        {
            // 游戏结束后按R重新开始
            if (wParam == 'R')
            {
                KillTimer(hWnd, TIMER_ID_DROP);   // 先停止旧定时器
                Restart();
                SetTimer(hWnd, TIMER_ID_DROP, dropInterval, NULL);
                PlayBGM();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            break;
        }

        if (suspendFlag && wParam != 'P')
        {
            // 暂停时只响应P键
            break;
        }

        switch (wParam)
        {
        case VK_LEFT:
            if (CanMoveLeft())
            {
                MoveLeft();
                if (targetFlag) ComputeTarget();
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;

        case VK_RIGHT:
            if (CanMoveRight())
            {
                MoveRight();
                if (targetFlag) ComputeTarget();
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;

        case VK_DOWN:
            if (CanMoveDown())
            {
                MoveDown();
                if (targetFlag) ComputeTarget();
                InvalidateRect(hWnd, NULL, FALSE);
            }
            break;

        case VK_UP:
        {
            Rotate();
            if (targetFlag) ComputeTarget();
            InvalidateRect(hWnd, NULL, FALSE);
            break;
        }

        case VK_SPACE:
            if (!gameOverFlag)
            {
                DropDown();                    // 直接落到底部
                LockCurrentBrick(hWnd);        // 固定方块并结算（与自然下落到底共用逻辑）
            }
            break;
            // 暂停键（P键）
        case 'P':
            suspendFlag = !suspendFlag;
            if (suspendFlag)
            {
                KillTimer(hWnd, TIMER_ID_DROP);
            }
            else
            {
                SetTimer(hWnd, TIMER_ID_DROP, dropInterval, NULL);
            }
            InvalidateRect(hWnd, NULL, FALSE);
            break;

            // 切换瞄准器（T键）
        case 'T':
            targetFlag = !targetFlag;
            if (targetFlag) ComputeTarget();
            InvalidateRect(hWnd, NULL, FALSE);
            break;

            // 使用道具（数字键1-3）
        case '1':
            ActivateItem(ITEM_CLEAR_BOTTOM);
            break;
        case '2':
            ActivateItem(ITEM_SLOW);
            break;
        case '3':
            ActivateItem(ITEM_DOUBLE_SCORE);
            break;
        }
        break;
    }

    case WM_ERASEBKGND:
        // 背景由 WM_PAINT 双缓冲整体绘制，这里拦截系统擦除，避免白色底闪
        return 1;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 创建内存双缓冲，避免闪烁
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, nWidth * GRID, nHeight * GRID);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

        // 填充背景
        HBRUSH hbrBkg = CreateSolidBrush(RGB(30, 30, 30));
        RECT rcClient;
        rcClient.left = 0;
        rcClient.top = 0;
        rcClient.right = nWidth * GRID;
        rcClient.bottom = nHeight * GRID;
        FillRect(hdcMem, &rcClient, hbrBkg);
        DeleteObject(hbrBkg);

        // 调用游戏绘制函数
        TDrawScreen(hdcMem, hWnd);


        // ----- 绘制操作说明提示 -----
// 设置文本属性
        SetBkMode(hdcMem, TRANSPARENT);


        HFONT hFontTitle = CreateFont(
            28, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
        HFONT hOldFontTitle = (HFONT)SelectObject(hdcMem, hFontTitle);
        SetTextColor(hdcMem, RGB(0, 0, 0));  
        TextOut(hdcMem, nWidth* GRID - 170, 300, L"操作说明", 5);
        SelectObject(hdcMem, hOldFontTitle);
        DeleteObject(hFontTitle);

        HFONT hFontText = CreateFont(
            18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
        HFONT hOldFontText = (HFONT)SelectObject(hdcMem, hFontText);

        int textX = nWidth * GRID - 170;
        int textY = 340;
        int lineHeight = 28;

        // ← → 移动
        SetTextColor(hdcMem, RGB(0, 0, 0));  
        TextOut(hdcMem, textX, textY, L"← →", 3);
        SetTextColor(hdcMem, RGB(0, 0, 0));  
        TextOut(hdcMem, textX + 60, textY, L"左右移动方块", 7);

        // ↑ 旋转
        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"↑", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"旋转方块", 5);

        // ↓ 加速
        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"↓", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"加速下落", 5);

        // 空格 直接下落
        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"空格", 2);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"直接落到底部", 7);

        // P 暂停
        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"P", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"暂停/继续", 6);

        // T 瞄准器
        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"T", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"切换瞄准器", 6);

        // R 重新开始
        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"R", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"重新开始", 4);

        // 道具说明分隔线
        textY += lineHeight + 5;
        SetTextColor(hdcMem, RGB(0, 0, 0));  // 金色
        MoveToEx(hdcMem, textX, textY, NULL);
        LineTo(hdcMem, textX + 200, textY);

        // 道具说明标题
        textY += 10;
        HFONT hFontItemTitle = CreateFont(
            18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
        HFONT hOldFontItemTitle = (HFONT)SelectObject(hdcMem, hFontItemTitle);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"道具快捷键", 6);
        SelectObject(hdcMem, hOldFontItemTitle);
        DeleteObject(hFontItemTitle);

        // 道具说明内容
        SelectObject(hdcMem, hFontText);
        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"1", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"消除底层一行", 7);

        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"2", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"减速下落", 5);

        textY += lineHeight;
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX, textY, L"3", 1);
        SetTextColor(hdcMem, RGB(0, 0, 0));
        TextOut(hdcMem, textX + 60, textY, L"双倍得分", 5);

        // 恢复旧字体
        SelectObject(hdcMem, hOldFontText);
        DeleteObject(hFontText);
        // ----- 操作说明绘制结束 -----



        // 将 700x945 的设计画面等比缩放到实际客户区（居中、不变形，必要时留黑边）
        RECT rcWin;
        GetClientRect(hWnd, &rcWin);
        int cw = rcWin.right;
        int ch = rcWin.bottom;
        const int srcW = nWidth * GRID;
        const int srcH = nHeight * GRID;

        double fa = (double)cw / srcW;
        double fb = (double)ch / srcH;
        double fit = (fa < fb) ? fa : fb;
        int dstW = (int)(srcW * fit + 0.5);
        int dstH = (int)(srcH * fit + 0.5);
        int dstX = (cw - dstW) / 2;
        int dstY = (ch - dstH) / 2;

        // 先在客户区大小的离屏缓冲里合成（黑边 + 缩放后的画面），再一次性贴到屏幕，
        // 避免分两步直接绘制到窗口DC导致的闪烁
        HDC hdcScreen = CreateCompatibleDC(hdc);
        HBITMAP hbmScreen = CreateCompatibleBitmap(hdc, cw, ch);
        HBITMAP hbmScreenOld = (HBITMAP)SelectObject(hdcScreen, hbmScreen);

        // 铺底（窗口比例与画面不一致时四周留黑边）
        HBRUSH hbrEdge = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdcScreen, &rcWin, hbrEdge);
        DeleteObject(hbrEdge);

        int oldStretch = SetStretchBltMode(hdcScreen, HALFTONE);
        SetBrushOrgEx(hdcScreen, 0, 0, NULL);
        StretchBlt(hdcScreen, dstX, dstY, dstW, dstH, hdcMem, 0, 0, srcW, srcH, SRCCOPY);
        SetStretchBltMode(hdcScreen, oldStretch);

        // 合成完成，单次拷贝到屏幕
        BitBlt(hdc, 0, 0, cw, ch, hdcScreen, 0, 0, SRCCOPY);

        SelectObject(hdcScreen, hbmScreenOld);
        DeleteObject(hbmScreen);
        DeleteDC(hdcScreen);

        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
    {
        StopBGM();
        KillTimer(hWnd, TIMER_ID_DROP);
        KillTimer(hWnd, TIMER_ID_ITEM);

        // 释放贴图资源
        for (int i = 0; i < 10; i++)
        {
            if (g_hBrickBitmaps[i])
            {
                DeleteObject(g_hBrickBitmaps[i]);
                g_hBrickBitmaps[i] = NULL;
            }

        }
        if (g_hEndBitmap) {
            DeleteObject(g_hEndBitmap);
            g_hEndBitmap = NULL;
        }
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 关于对话框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}