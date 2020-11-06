#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include "TXLib.h"
#define AREA_SIZE 10
#define MAX_SHIP_SIZE 4
using namespace std;

const int windowWidth = 1000, windowHeight = 1000;
const int gameAreaSize = 440;
char fontMain[] = "Monaco";

enum AreaMod {
    AREA_SHIP, AREA_MAIN
};

enum Keys {
    KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN
};

struct Points {
    int x1;
    int y1;
    int x2;
    int y2;
};

struct AreaColors {
    COLORREF frame;
    COLORREF area;
    COLORREF line;
};

struct AreaSizes {
    int segment;
    int area;
    int frame;
};


class GameArea {
private:
    vector<POINT> hits;         // { x, y } - хранит индекс в segments квадрата попадания
    vector<POINT> blunders;     // { x, y } - хранит индекс в segments квадрата промаха
                                // индекс = AREA_SIZE * (y - 1) + x
    vector<Points> segments;    // { x1, y1, x2, y2 }  - хранит координаты на игровом столе
    POINT pos;
    AreaMod areamod;            // переключалка между полем для кораблей и для выстрелов
    // int sizeSegment;            // размер сегмента
    //int x0, y0, size;           // точки отсчета (начальные точки расположения поля) и его размер
    //int sizeFrame;              // ширина рамки
    AreaColors colors;           // цвет рамки / поля / линий (обводки)
    AreaSizes sizes;

    void _update_segments();
    void _draw_area();

public:
    void update_hits();
    void update_blunders();
    void change_position(int, int);
    Points get_segment(int);
    int get_segment_size();

    GameArea(int, int, int, int, COLORREF, COLORREF, COLORREF, AreaMod);
};


class Ship {
private:
    Points pos;                 // хранит координаты относительно окна
    int amountSegments;         // количество палуб
    int sizeSegment;            // размер сегмента
    POINT size;                 // изменение конечных x, y для построения фигуры
    AreaColors colors;

public:
    void change_position(int, int);
    void change_size(Keys);
    void draw_ship();

    Ship(int, int, int, int, COLORREF, COLORREF);
};


void draw_rectangle(Points, int, COLORREF, COLORREF);
void print_text(int, int, char[], unsigned, char[], int, COLORREF);
void draw_text(Points, char[], unsigned, char[], int, COLORREF);
void start_window(Points, int, int, COLORREF, COLORREF, COLORREF);
bool check_start_window(RECT);
void create_rectangle(int, int, int, int, COLORREF);
void draw_line(Points);
void clear_area(Points);
void draw_ship_window(Points, int, AreaColors, vector<Ship>&, AreaColors, int);



GameArea::GameArea(int x1, int y1, int size, int sizeFrame, COLORREF colorFrame, COLORREF colorArea, COLORREF colorLine, AreaMod mod) {
    areamod = mod;
    pos = { x1, y1 };
    sizes = { size / (AREA_SIZE + 1), size, sizeFrame };
    colors = { colorFrame, colorArea, colorLine };
    
    _draw_area();
    _update_segments();
}

void GameArea::_update_segments() {
    for (int y = 0; y < AREA_SIZE; y++) {
        for (int x = 0; x < AREA_SIZE; x++) {
            segments.push_back({ x * sizes.segment, y * sizes.segment, (x + 1) * sizes.segment, (y + 1) * sizes.segment });
        }
    }
}

void GameArea::_draw_area() {
    int halfSizeFrame = sizes.frame / 2;
    int sizeLine = sizes.frame * 0.3;

    draw_rectangle({ pos.x, pos.y, pos.x + sizes.area, pos.y + sizes.area }, sizes.frame, colors.frame, colors.area);

    txSetColor(colors.line, sizeLine);
    for (int i = pos.x + sizes.segment; i <= (AREA_SIZE + 1) * sizes.segment; i += sizes.segment) {
        draw_line({ i, pos.y + halfSizeFrame, i, pos.y + sizes.area - halfSizeFrame - sizeLine });
    }
    for (int i = pos.y + sizes.segment; i <= (AREA_SIZE + 1) * sizes.segment; i += sizes.segment) {
        draw_line({ pos.x + halfSizeFrame, i, pos.x + sizes.area - halfSizeFrame - sizeLine, i });
    }
}

void GameArea::change_position(int x, int y) {
    pos = { x, y };
    _draw_area();
}

Points GameArea::get_segment(int index) {
    return segments[index];
}

int GameArea::get_segment_size() {
    return sizes.segment;
}

void GameArea::update_hits() {

}

void GameArea::update_blunders() {

}



Ship::Ship(int segments, int x, int y, int size, COLORREF colorLine, COLORREF colorInside) {
    amountSegments = segments;
    sizeSegment = size;
    this->size = { sizeSegment * amountSegments, sizeSegment };
    pos = { x, y, x + this->size.x, y + this->size.y };
    colors = { TX_TRANSPARENT, colorInside, colorLine };
    /*for (int i = 0; i < segments; i++ ) {
        coords.push_back({ x + i * size, y, x + (i + 1) * size, y + size });
    }*/

    draw_ship();
}

void Ship::change_position(int x, int y) {
    pos = { x, y, x + size.x, y + size.y };
}

void Ship::change_size(Keys key) {
    switch (key) {
    case KEY_LEFT:
    case KEY_RIGHT:
        size = { sizeSegment * amountSegments, sizeSegment };
        break;
    case KEY_UP:
    case KEY_DOWN:
        size = { sizeSegment, sizeSegment * amountSegments };
        break;
    }
    pos = { pos.x1, pos.y1, pos.x1 + size.x, pos.y1 + size.y };
}

void Ship::draw_ship() {
    draw_rectangle(pos, 1, colors.frame, colors.area);
}



int main()
{
    setlocale(LC_ALL, "rus");
    int frame_x0 = 10;
    int centerX = (windowWidth + 1) / 2, centerY = (windowHeight + 1) / 2;

    txCreateWindow(windowWidth, windowHeight);
    txTextCursor(false);
    
    int halfStartButtonX = 50, halfStartButtonY = 30;

    draw_rectangle({ frame_x0, frame_x0, windowWidth - frame_x0, windowHeight - frame_x0 }, 1, TX_WHITE, TX_TRANSPARENT);

    RECT rcStartWindow = { centerX - halfStartButtonX, centerY - halfStartButtonY, centerX + halfStartButtonX, centerY + halfStartButtonY };
    start_window({ rcStartWindow.left, rcStartWindow.top, rcStartWindow.right, rcStartWindow.bottom },
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
    int gameAreaX0 = 30, gameAreaY0 = 30;
    int shipWindowX0 = gameAreaX0 + gameAreaSize + 50, shipWindowY0 = gameAreaY0;
    vector<Ship> ships;

    GameArea gameAreaShip(gameAreaX0, gameAreaY0, gameAreaSize, 8, TX_WHITE, TX_BLUE, TX_GRAY, AREA_SHIP);
    int sizeSegment = gameAreaShip.get_segment_size();

    draw_rectangle({ frame_x0, frame_x0, windowWidth - frame_x0, windowHeight - frame_x0 }, 1, TX_WHITE, TX_TRANSPARENT);
    draw_ship_window(
        { shipWindowX0, shipWindowY0, shipWindowX0 + sizeSegment * (MAX_SHIP_SIZE + 2), shipWindowY0 + sizeSegment * (MAX_SHIP_SIZE + 4) }, 
        3, { TX_YELLOW, TX_RED, NULL }, ships, { NULL, TX_BROWN, TX_CYAN }, sizeSegment
    );

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

void draw_rectangle(Points points, int thickness, COLORREF colorFrame, COLORREF colorInside) {
    txSetColor(colorFrame, thickness);
    txSetFillColor(colorInside);
    txRectangle(points.x1, points.y1, points.x2, points.y2);
}

void print_text(int x, int y, char text[], unsigned align, char font[], int fontSize, COLORREF color) {
    txSetColor(color);
    txSelectFont(font, fontSize);
    txSetTextAlign(align);
    txTextOut(x, y, text);
}

void draw_text(Points points, char text[], unsigned align, char font[], int fontSize, COLORREF color) {
    txSetColor(color);
    txSelectFont(font, fontSize);
    txDrawText(points.x1, points.y1, points.x2, points.y2, text, align);
}

void create_rectangle(int x1, int y1, int x2, int y2, COLORREF color) {
    txSetColor(color);
    txSetFillColor(color);
    txRectangle(x1, y1, x2, y2);
}

void draw_line(Points points) {
    txLine(points.x1, points.y1, points.x2, points.y2);
}

bool check_start_window(RECT rc) {
    if (In(txMousePos(), rc))
        return true;
    return false;
}

void start_window(Points points, int thickness, int fontSize, COLORREF colorFrame, COLORREF colorInside, COLORREF colorText) {
    //int centerX = (x2 + x1) / 2, centerY = (y2 + y1) / 2;
    char text[] = "НАЧАТЬ";

    draw_rectangle(points, thickness, colorFrame, colorInside);
    draw_text(points, text, DT_CENTER | TA_CENTER, fontMain, fontSize, colorText);
}

void clear_area(Points points) {
    txSetColor(TX_BLACK);
    txSetFillColor(TX_BLACK);
    txRectangle(points.x1, points.y1, points.x2, points.y2);
}

void draw_ship_window(Points points, int thickness, AreaColors colorsRect, vector<Ship>& ships, AreaColors colorsShip, int size) {
    draw_rectangle(points, thickness, colorsRect.frame, colorsRect.area);

    //int size = (points.x2 - points.x1) / MAX_SHIP_SIZE;
    int tempX, tempY = points.y1 + size;

    for (int i = 1; i <= MAX_SHIP_SIZE; i++) {
        tempX = points.x1 + size;
        for (int j = MAX_SHIP_SIZE; j >= i; j--) {
            if (tempX + size * i > points.x2) {
                tempX = points.x1 + size;
                tempY += size;
            }
            ships.push_back(Ship(i, tempX, tempY, size, colorsShip.line, colorsShip.area));
            tempX += size * i;
        }
        tempY += size;
    }
}

// сделать второй вектор у поля с меньшим квадратом для кораблей
