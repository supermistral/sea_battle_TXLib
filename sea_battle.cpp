#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "TXLib.h"
#define AREA_SIZE 10
using namespace std;

const int windowWidth = 1000, windowHeight = 1000;
char fontMain[] = "Monaco";

enum AreaMod {
    AREA_SHIP, AREA_MAIN
};

struct Points {
    int x1;
    int y1;
    int x2;
    int y2;
};

struct AreaColor {
    COLORREF frame;
    COLORREF area;
    COLORREF line;
};


class GameArea {
private:
    vector<POINT> hits;         // { x, y } - хранит индекс в segments квадрата попадания
    vector<POINT> blunders;     // { x, y } - хранит индекс в segments квадрата промаха
                                // индекс = AREA_SIZE * (y - 1) + x
    vector<Points> segments;    // { x1, y1, x2, y2 }  - хранит координаты на игровом столе
    AreaMod areamod;            // переключалка между полем для кораблей и для выстрелов
    int sizeSegment;            // размер сегмента
    int x0, y0, size;           // точки отсчета (начальные точки расположения поля) и его размер
    int sizeFrame;              // ширина рамки
    AreaColor colors;           // цвет рамки / поля / линий (обводки)

    void _update_segments();
    void _draw_area();

public:
    void update_hits();
    void update_blunders();
    void change_position(int, int);
    Points get_segment(int);

    GameArea(int, int, int, int, COLORREF, COLORREF, COLORREF, AreaMod);
};


class Ship {
private:
    vector<Points> coords;      // хранит координаты относительно окна
    int amountSegments;         // количество палуб
    int sizeSegment;            // размер сегмента

public:
    void change_position(int, int);

    Ship(int, int, int, int);
};


void draw_rectangle(int, int, int, int, int, COLORREF, COLORREF);
void print_text(int, int, char[], unsigned, char[], int, COLORREF);
void draw_text(int, int, int, int, char[], unsigned, char[], int, COLORREF);
void start_window(int, int, int, int, int, int, COLORREF, COLORREF, COLORREF);
bool check_start_window(RECT);
void key_pressed();
void create_rectangle(int, int, int, int, COLORREF);
void draw_line(int, int, int, int);
void game();
void clear_area(int, int, int, int);



GameArea::GameArea(int x1, int y1, int size, int sizeFrame, COLORREF colorFrame, COLORREF colorArea, COLORREF colorLine, AreaMod mod) {
    sizeSegment = size / (AREA_SIZE + 1);
    areamod = mod;
    this->sizeFrame = sizeFrame;
    this->x0 = x1; this->y0 = y1;
    this->size = size;
    colors = { colorFrame, colorArea, colorLine };
    
    _draw_area();
    _update_segments();
}

void GameArea::_update_segments() {
    for (int y = 0; y < AREA_SIZE; y++) {
        for (int x = 0; x < AREA_SIZE; x++) {
            segments.push_back({ x * sizeSegment, y * sizeSegment, (x + 1) * sizeSegment, (y + 1) * sizeSegment });
        }
    }
}

void GameArea::_draw_area() {
    int halfSizeFrame = sizeFrame / 2;
    int sizeLine = sizeFrame * 0.3;

    draw_rectangle(x0, y0, x0 + size, y0 + size, sizeFrame, colors.frame, colors.area);

    txSetColor(colors.line, sizeLine);
    for (int i = x0 + sizeSegment; i <= (AREA_SIZE + 1) * sizeSegment; i += sizeSegment) {
        draw_line(i, y0 + halfSizeFrame, i, y0 + size - halfSizeFrame - sizeLine);
    }
    for (int i = y0 + sizeSegment; i <= (AREA_SIZE + 1) * sizeSegment; i += sizeSegment) {
        draw_line(x0 + halfSizeFrame, i, x0 + size - halfSizeFrame - sizeLine, i);
    }
}

void GameArea::change_position(int x, int y) {
    x0 = x;
    y0 = y;
    _draw_area();
}

Points GameArea::get_segment(int index) {
    return segments[index];
}

void GameArea::update_hits() {

}

void GameArea::update_blunders() {

}



Ship::Ship(int segments, int x, int y, int size) {
    amountSegments = segments;
    sizeSegment = size;

    for (int i = 0; i < segments; i++ ) {
        coords.push_back({ x + i * size, y, x + (i + 1) * size, y + size });
    }

}



int main()
{
    setlocale(LC_ALL, "rus");
    int frame_x0 = 10;
    int centerX = (windowWidth + 1) / 2, centerY = (windowHeight + 1) / 2;

    txCreateWindow(windowWidth, windowHeight);
    txTextCursor(false);
    
    int halfStartButtonX = 50, halfStartButtonY = 30;

    draw_rectangle(frame_x0, frame_x0, windowWidth - frame_x0, windowHeight - frame_x0, 1, TX_WHITE, TX_TRANSPARENT);

    RECT rcStartWindow = { centerX - halfStartButtonX, centerY - halfStartButtonY, centerX + halfStartButtonX, centerY + halfStartButtonY };
    start_window(rcStartWindow.left, rcStartWindow.top, rcStartWindow.right, rcStartWindow.bottom,
        5, 30, TX_WHITE, TX_LIGHTGRAY, TX_YELLOW);

    // Старт окно
    bool running = false;
    while (!running) {
        while (txMouseButtons() != 1) {}
        running = check_start_window(rcStartWindow);
    }

    txSetFillColor(TX_BLACK);
    txClear();

    // Расстановка кораблей
    draw_rectangle(frame_x0, frame_x0, windowWidth - frame_x0, windowHeight - frame_x0, 1, TX_WHITE, TX_TRANSPARENT);
    GameArea gameAreaShip(30, 30, 440, 8, TX_WHITE, TX_BLUE, TX_GRAY, AREA_SHIP);

    //int x1 = 30, y1 = 30;
    //create_rectangle(0, 0, 10, 10, TX_RED);
    //print_text(centerX, centerY, text1, TA_BASELINE | TA_CENTER, fontMain, 80, TX_RED);
    /*txBegin();
    while (!GetAsyncKeyState(VK_ESCAPE)) {
        txSetFillColor(TX_BLACK);
        txClear();
        create_rectangle(x1 - 30, y1 - 30, x1, y1, TX_RED);
        x1 += 10;
        txSleep(30);
    }
    txEnd();*/


    return 0;
}

void draw_rectangle(int x1, int y1, int x2, int y2, int thickness, COLORREF colorFrame, COLORREF colorInside) {
    txSetColor(colorFrame, thickness);
    txSetFillColor(colorInside);
    txRectangle(x1, y1, x2, y2);
}

void print_text(int x, int y, char text[], unsigned align, char font[], int fontSize, COLORREF color) {
    txSetColor(color);
    txSelectFont(font, fontSize);
    txSetTextAlign(align);
    txTextOut(x, y, text);
}

void draw_text(int x1, int y1, int x2, int y2, char text[], unsigned align, char font[], int fontSize, COLORREF color) {
    txSetColor(color);
    txSelectFont(font, fontSize);
    txDrawText(x1, y1, x2, y2, text, align);
}

void create_rectangle(int x1, int y1, int x2, int y2, COLORREF color) {
    txSetColor(color);
    txSetFillColor(color);
    txRectangle(x1, y1, x2, y2);
}

void draw_line(int x1, int y1, int x2, int y2) {
    txLine(x1, y1, x2, y2);
}

bool check_start_window(RECT rc) {
    if (In(txMousePos(), rc))
        return true;
    return false;
}

void start_window(int x1, int y1, int x2, int y2, int thickness, int fontSize, COLORREF colorFrame, COLORREF colorInside, COLORREF colorText) {
    //int centerX = (x2 + x1) / 2, centerY = (y2 + y1) / 2;
    char text[] = "НАЧАТЬ";

    draw_rectangle(x1, y1, x2, y2, thickness, colorFrame, colorInside);
    draw_text(x1, y1, x2, y2, text, DT_CENTER | TA_CENTER, fontMain, fontSize, colorText);
}

void clear_area(int x1, int y1, int x2, int y2) {
    txSetColor(TX_BLACK);
    txSetFillColor(TX_BLACK);
    txRectangle(x1, y1, x2, y2);
}
