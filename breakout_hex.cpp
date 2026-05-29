#ifdef UNICODE
#undef UNICODE
#endif

#ifdef _UNICODE
#undef _UNICODE
#endif

#include <graphics.h>
#include <conio.h>
#include <math.h>
#include <stdio.h>
#include <tchar.h>

#define WIN_W 960
#define WIN_H 700
#define BOARD_CX 500
#define BOARD_CY 355
#define HEX_SIZE 44.0
#define BOARD_RADIUS 3
#define MAX_STEPS 18
#define PI 3.14159265358979323846

#define CN_TITLE "\xCD\xBB\xCE\xA7"
#define CN_WINDOW_TITLE "\xCD\xBB\xCE\xA7\x20\x2D\x20\xC1\xF9\xB1\xDF\xD0\xCE\xC6\xE5\xC5\xCC\xD0\xA1\xD3\xCE\xCF\xB7"
#define CN_RULE1 "\xCF\xC8\xC4\xC3\xB5\xC0\xBE\xDF\xA3\xAC\xD4\xD9\xB5\xD6\xB4\xEF\xB3\xF6\xBF\xDA\xA1\xA3"
#define CN_RULE2 "\xC1\xF9\xCF\xF2\xD2\xC6\xB6\xAF\xA3\xBA\x57\x2F\x45\x2F\x41\x2F\x44\x2F\x5A\x2F\x58"
#define CN_RULE3 "\x52\x20\xD6\xD8\xBF\xAA\xA3\xAC\x45\x53\x43\x20\xCD\xCB\xB3\xF6"
#define CN_STEPS "\xB2\xBD\xCA\xFD\xA3\xBA\x25\x64\x20\x2F\x20\x25\x64"
#define CN_TASK_EXIT "\xC8\xCE\xCE\xF1\xA3\xBA\xC8\xA5\xB3\xF6\xBF\xDA"
#define CN_TASK_ITEM "\xC8\xCE\xCE\xF1\xA3\xBA\xCA\xB0\xC8\xA1\xB5\xC0\xBE\xDF"
#define CN_ARROW_TASK "\xBB\xC6\xC9\xAB\xBC\xFD\xCD\xB7\xD6\xB8\xCF\xF2\xB5\xB1\xC7\xB0\xC8\xCE\xCE\xF1"
#define CN_ARROW_EXIT "\xC7\xE0\xC9\xAB\xBC\xFD\xCD\xB7\xD6\xB8\xCF\xF2\xB3\xF6\xBF\xDA"
#define CN_WIN "\xCD\xBB\xCE\xA7\xB3\xC9\xB9\xA6"
#define CN_LOSE "\xD0\xD0\xB6\xAF\xCA\xA7\xB0\xDC"
#define CN_EXIT "\xB3\xF6\xBF\xDA"
#define CN_TASK "\xC8\xCE\xCE\xF1"
#define CN_RESTART "\xB0\xB4\x20\x52\x20\xD6\xD8\xD0\xC2\xBF\xAA\xCA\xBC\xA3\xAC\x45\x53\x43\x20\xCD\xCB\xB3\xF6"
#define CN_HAS_ITEM "\xD2\xD1\xBB\xF1\xB5\xC3\xB5\xC0\xBE\xDF"
#define CN_NO_ITEM "\xCE\xB4\xBB\xF1\xB5\xC3\xB5\xC0\xBE\xDF"

struct Cell {
    int q;
    int r;
};

enum GameState {
    RUNNING,
    WIN,
    LOSE
};

static Cell player = { -3, 0 };
static Cell itemCell = { 0, -2 };
static Cell exitCell = { 3, 0 };
static int stepCount = 0;
static bool hasItem = false;
static GameState gameState = RUNNING;

static const Cell blockedCells[] = {
    { -1, 0 }, { 0, 1 }, { 1, -1 }, { 1, 1 }, { -2, 2 }
};

static bool SameCell(Cell a, Cell b) {
    return a.q == b.q && a.r == b.r;
}

static int AbsInt(int v) {
    return v < 0 ? -v : v;
}

static bool IsInsideBoard(Cell c) {
    int s = -c.q - c.r;
    return AbsInt(c.q) <= BOARD_RADIUS &&
           AbsInt(c.r) <= BOARD_RADIUS &&
           AbsInt(s) <= BOARD_RADIUS;
}

static bool IsBlocked(Cell c) {
    int count = sizeof(blockedCells) / sizeof(blockedCells[0]);
    int i;
    for (i = 0; i < count; ++i) {
        if (SameCell(c, blockedCells[i])) {
            return true;
        }
    }
    return false;
}

static POINT HexCenter(Cell c) {
    POINT p;
    double x = HEX_SIZE * 1.5 * c.q;
    double y = HEX_SIZE * sqrt(3.0) * (c.r + c.q / 2.0);
    p.x = BOARD_CX + (int)(x + 0.5);
    p.y = BOARD_CY + (int)(y + 0.5);
    return p;
}

static void BuildHexPoints(POINT center, POINT pts[6], double scale) {
    double size = HEX_SIZE * scale;
    int i;
    for (i = 0; i < 6; ++i) {
        double angle = PI / 180.0 * (60.0 * i);
        pts[i].x = center.x + (int)(cos(angle) * size + 0.5);
        pts[i].y = center.y + (int)(sin(angle) * size + 0.5);
    }
}

static void SetCnFont(int size) {
    settextstyle(size, 0, _T("\xCB\xCE\xCC\xE5"));
}

static void DrawArrow(int x1, int y1, int x2, int y2, COLORREF color, LPCTSTR label) {
    double angle;
    double wing1;
    double wing2;
    POINT arrowHead[3];

    setlinecolor(color);
    setfillcolor(color);
    setlinestyle(PS_SOLID, 4);
    line(x1, y1, x2, y2);

    angle = atan2((double)(y2 - y1), (double)(x2 - x1));
    wing1 = angle + PI * 0.78;
    wing2 = angle - PI * 0.78;
    arrowHead[0].x = x2;
    arrowHead[0].y = y2;
    arrowHead[1].x = x2 + (int)(cos(wing1) * 18.0);
    arrowHead[1].y = y2 + (int)(sin(wing1) * 18.0);
    arrowHead[2].x = x2 + (int)(cos(wing2) * 18.0);
    arrowHead[2].y = y2 + (int)(sin(wing2) * 18.0);
    solidpolygon(arrowHead, 3);

    setbkmode(TRANSPARENT);
    settextcolor(color);
    SetCnFont(18);
    outtextxy((x1 + x2) / 2 + 8, (y1 + y2) / 2 - 24, label);
    setlinestyle(PS_SOLID, 1);
}

static void DrawCell(Cell c) {
    POINT center = HexCenter(c);
    POINT pts[6];
    bool blocked;
    bool isPlayer;
    bool isItem;
    bool isExit;
    COLORREF fill = RGB(95, 18, 24);
    COLORREF edge = RGB(205, 72, 68);

    BuildHexPoints(center, pts, 0.96);
    blocked = IsBlocked(c);
    isPlayer = SameCell(c, player);
    isItem = SameCell(c, itemCell) && !hasItem;
    isExit = SameCell(c, exitCell);

    if (blocked) {
        fill = RGB(45, 11, 16);
        edge = RGB(112, 31, 38);
    } else if (isExit) {
        fill = hasItem ? RGB(26, 98, 78) : RGB(78, 39, 36);
        edge = RGB(80, 226, 177);
    } else if (isItem) {
        fill = RGB(198, 148, 40);
        edge = RGB(255, 224, 92);
    }

    setfillcolor(fill);
    setlinecolor(edge);
    setlinestyle(PS_SOLID, 2);
    fillpolygon(pts, 6);
    polygon(pts, 6);

    if (blocked) {
        settextcolor(RGB(168, 62, 70));
        settextstyle(22, 0, _T("Consolas"));
        outtextxy(center.x - 7, center.y - 13, _T("X"));
    }

    if (isItem) {
        setfillcolor(RGB(255, 227, 80));
        solidcircle(center.x, center.y, 12);
        setlinecolor(RGB(96, 57, 9));
        circle(center.x, center.y, 12);
        settextcolor(RGB(96, 57, 9));
        SetCnFont(14);
        outtextxy(center.x - 14, center.y - 34, _T("\xB5\xC0\xBE\xDF"));
    }

    if (isExit) {
        settextcolor(RGB(158, 255, 218));
        SetCnFont(20);
        outtextxy(center.x - 20, center.y - 13, _T(CN_EXIT));
    }

    if (isPlayer) {
        setfillcolor(RGB(255, 54, 63));
        solidcircle(center.x, center.y, 17);
        setlinecolor(RGB(255, 224, 224));
        circle(center.x, center.y, 17);
        settextcolor(WHITE);
        settextstyle(18, 0, _T("Consolas"));
        outtextxy(center.x - 5, center.y - 11, _T("P"));
    }
}

static void DrawBackground() {
    int y;
    int x;
    setbkcolor(RGB(32, 5, 11));
    cleardevice();

    for (y = 0; y < WIN_H; ++y) {
        int r = 32 + y * 36 / WIN_H;
        int g = 5 + y * 7 / WIN_H;
        int b = 11 + y * 12 / WIN_H;
        setlinecolor(RGB(r, g, b));
        line(0, y, WIN_W, y);
    }

    setlinecolor(RGB(92, 22, 30));
    for (x = -80; x < WIN_W + 160; x += 80) {
        line(x, 0, x - 210, WIN_H);
    }

    setfillcolor(RGB(82, 13, 19));
    solidcircle(820, 120, 145);
    setfillcolor(RGB(124, 21, 28));
    solidcircle(120, 610, 115);

    setlinecolor(RGB(160, 35, 39));
    setlinestyle(PS_SOLID, 3);
    line(285, 70, 910, 70);
    line(285, 630, 910, 630);
    setlinestyle(PS_SOLID, 1);
}

static void DrawPanel() {
    TCHAR buffer[128];

    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 228, 217));
    SetCnFont(46);
    outtextxy(52, 42, _T(CN_TITLE));

    SetCnFont(18);
    settextcolor(RGB(235, 176, 166));
    outtextxy(55, 104, _T(CN_RULE1));
    outtextxy(55, 132, _T(CN_RULE2));
    outtextxy(55, 160, _T(CN_RULE3));

    _stprintf(buffer, _T(CN_STEPS), stepCount, MAX_STEPS);
    SetCnFont(24);
    settextcolor(stepCount >= MAX_STEPS - 3 ? RGB(255, 196, 78) : RGB(255, 228, 217));
    outtextxy(55, 218, buffer);

    SetCnFont(22);
    settextcolor(hasItem ? RGB(116, 255, 197) : RGB(255, 205, 87));
    outtextxy(55, 260, hasItem ? _T(CN_TASK_EXIT) : _T(CN_TASK_ITEM));

    SetCnFont(18);
    settextcolor(RGB(223, 126, 120));
    outtextxy(55, 315, _T(CN_ARROW_TASK));
    outtextxy(55, 342, _T(CN_ARROW_EXIT));

    settextcolor(hasItem ? RGB(116, 255, 197) : RGB(255, 205, 87));
    outtextxy(55, 382, hasItem ? _T(CN_HAS_ITEM) : _T(CN_NO_ITEM));

    if (gameState == WIN) {
        SetCnFont(34);
        settextcolor(RGB(122, 255, 193));
        outtextxy(55, 442, _T(CN_WIN));
    } else if (gameState == LOSE) {
        SetCnFont(34);
        settextcolor(RGB(255, 92, 92));
        outtextxy(55, 442, _T(CN_LOSE));
    }
}

static void DrawBoard() {
    int q;
    int r;
    POINT p;
    POINT item;
    POINT exitP;
    POINT target;

    for (q = -BOARD_RADIUS; q <= BOARD_RADIUS; ++q) {
        for (r = -BOARD_RADIUS; r <= BOARD_RADIUS; ++r) {
            Cell c;
            c.q = q;
            c.r = r;
            if (IsInsideBoard(c)) {
                DrawCell(c);
            }
        }
    }

    p = HexCenter(player);
    item = HexCenter(itemCell);
    exitP = HexCenter(exitCell);
    target = hasItem ? exitP : item;

    if (gameState == RUNNING) {
        DrawArrow(p.x, p.y - 28, target.x, target.y - 28, RGB(255, 214, 74), _T(CN_TASK));
        DrawArrow(p.x, p.y + 28, exitP.x, exitP.y + 28, RGB(97, 235, 211), _T(CN_EXIT));
    }
}

static void DrawOverlay() {
    if (gameState == RUNNING) {
        return;
    }

    setfillcolor(RGB(24, 5, 8));
    setlinecolor(RGB(255, 86, 82));
    setlinestyle(PS_SOLID, 2);
    solidroundrect(310, 246, 650, 446, 12, 12);
    roundrect(310, 246, 650, 446, 12, 12);

    setbkmode(TRANSPARENT);
    SetCnFont(36);
    if (gameState == WIN) {
        settextcolor(RGB(118, 255, 195));
        outtextxy(388, 292, _T(CN_WIN));
    } else {
        settextcolor(RGB(255, 91, 91));
        outtextxy(388, 292, _T(CN_LOSE));
    }

    SetCnFont(20);
    settextcolor(RGB(241, 186, 174));
    outtextxy(366, 360, _T(CN_RESTART));
}

static void DrawGame() {
    BeginBatchDraw();
    DrawBackground();
    DrawPanel();
    DrawBoard();
    DrawOverlay();
    EndBatchDraw();
}

static void ResetGame() {
    player.q = -3;
    player.r = 0;
    stepCount = 0;
    hasItem = false;
    gameState = RUNNING;
}

static void CheckGameResult() {
    if (SameCell(player, itemCell)) {
        hasItem = true;
    }

    if (hasItem && SameCell(player, exitCell)) {
        gameState = WIN;
        return;
    }

    if (stepCount >= MAX_STEPS) {
        gameState = LOSE;
    }
}

static void TryMove(int dq, int dr) {
    Cell next;
    if (gameState != RUNNING) {
        return;
    }

    next.q = player.q + dq;
    next.r = player.r + dr;
    if (!IsInsideBoard(next) || IsBlocked(next)) {
        return;
    }

    player = next;
    ++stepCount;
    CheckGameResult();
}

static void HandleKey(int key) {
    if (key >= 'a' && key <= 'z') {
        key -= 32;
    }

    switch (key) {
    case 'W':
        TryMove(0, -1);
        break;
    case 'E':
        TryMove(1, -1);
        break;
    case 'D':
        TryMove(1, 0);
        break;
    case 'X':
        TryMove(0, 1);
        break;
    case 'Z':
        TryMove(-1, 1);
        break;
    case 'A':
        TryMove(-1, 0);
        break;
    case 'R':
        ResetGame();
        break;
    default:
        break;
    }
}

static bool KeyPressedOnce(int vk) {
    static int lastState[256] = { 0 };
    int down = (GetAsyncKeyState(vk) & 0x8000) != 0;
    bool pressed = down && !lastState[vk];
    lastState[vk] = down;
    return pressed;
}

static bool HandleInput() {
    bool changed = false;

    if (KeyPressedOnce(VK_ESCAPE)) {
        return true;
    }

    if (KeyPressedOnce('W')) {
        TryMove(0, -1);
        changed = true;
    }
    if (KeyPressedOnce('E')) {
        TryMove(1, -1);
        changed = true;
    }
    if (KeyPressedOnce('D')) {
        TryMove(1, 0);
        changed = true;
    }
    if (KeyPressedOnce('X')) {
        TryMove(0, 1);
        changed = true;
    }
    if (KeyPressedOnce('Z')) {
        TryMove(-1, 1);
        changed = true;
    }
    if (KeyPressedOnce('A')) {
        TryMove(-1, 0);
        changed = true;
    }
    if (KeyPressedOnce('R')) {
        ResetGame();
        changed = true;
    }

    if (changed) {
        DrawGame();
    }

    return false;
}

int main() {
    initgraph(WIN_W, WIN_H);
    SetWindowText(GetHWnd(), _T(CN_WINDOW_TITLE));
    SetForegroundWindow(GetHWnd());
    ResetGame();
    DrawGame();

    while (true) {
        if (HandleInput()) {
            break;
        }
        Sleep(10);
    }

    closegraph();
    return 0;
}
