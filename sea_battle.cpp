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


class Button {
private:
    Points pos;
    AreaColors colors;
    char* text = new char;
    int fontSize;
    int thickness;
    unsigned align;

public:
    void draw();
    RECT get_rect();
    bool check_mouse();

    Button(Points, char[], int, int, AreaColors, unsigned);
};


class GameArea {
private:
    vector<POINT> hits;             // { x, y } - хранит индекс в segments квадрата попадания
    vector<POINT> blunders;         // { x, y } - хранит индекс в segments квадрата промаха
                                    // индекс = AREA_SIZE * (y - 1) + x
    vector<Points> segments;        // { x1, y1, x2, y2 }  - хранит координаты на игровом столе
    vector<bool> segmentsFull;      //  Параметр заполненности сегмента
    POINT pos;
    AreaMod areamod;                // переключалка между полем для кораблей и для выстрелов
    // int sizeSegment;             // размер сегмента
    //int x0, y0, size;             // точки отсчета (начальные точки расположения поля) и его размер
    //int sizeFrame;                // ширина рамки
    AreaColors colors;              // цвет рамки / поля / линий (обводки)
    AreaSizes sizes;

    void _update_segments();
    void _draw_area();

public:
    void update_hits();
    void update_blunders();
    void change_position(int, int);
    POINT get_segment(int);
    int get_segment_size();
    int get_space_frame();
    void draw();
    POINT get_end_position();

    GameArea(int, int, int, int, AreaColors, AreaMod);
};


class Ship {
private:
    Points pos;                 // хранит координаты относительно окна
    int amountSegments;         // количество палуб
    int sizeSegment;            // размер сегмента
    POINT size;                 // изменение конечных x, y для построения фигуры
    AreaColors colors;
    int spaceFrame;
    bool onArea = false;

public:
    void change_position(POINT);
    void change_size(Keys);
    void draw();
    bool check_mouse();
    Points get_real_size();
    POINT get_size();

    Ship(int, int, int, int, int, COLORREF, COLORREF);
};



void draw_rectangle(Points, int, COLORREF, COLORREF);
void print_text(int, int, char[], unsigned, char[], int, COLORREF);
void draw_text(Points, char[], unsigned, char[], int, COLORREF);
void start_window(Points, int, int, COLORREF, COLORREF, COLORREF);
bool check_press_button(RECT);
//void create_rectangle(int, int, int, int, COLORREF);
void draw_line(Points);
void clear_area(Points, COLORREF = TX_BLACK);
void draw_ship_window(Points, int, AreaColors);
void create_ships(Points, vector<Ship>&, AreaColors, int, int);



GameArea::GameArea(int x1, int y1, int size, int sizeFrame, AreaColors colors, AreaMod mod) {
    areamod = mod;
    pos = { x1, y1 };
    sizes = { size / (AREA_SIZE + 1), size, sizeFrame };
    this->colors = colors;
    
    _draw_area();
    _update_segments();
}

void GameArea::_update_segments() {
    for (int y = 1; y <= AREA_SIZE; y++) {
        for (int x = 1; x <= AREA_SIZE; x++) {
            segments.push_back({ pos.x + x * sizes.segment, pos.y + y * sizes.segment, 
                pos.x + (x + 1) * sizes.segment, pos.y + (y + 1) * sizes.segment });
            segmentsFull.push_back(false);
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

POINT GameArea::get_segment(int index) {
    return { segments[index].x1, segments[index].y1 };
}

int GameArea::get_segment_size() {
    return sizes.segment;
}

int GameArea::get_space_frame() {
    return sizes.frame * 0.3 * 3;
}

void GameArea::update_hits() {

}

void GameArea::update_blunders() {

}

void GameArea::draw() {
    _draw_area();
}

POINT GameArea::get_end_position() {
    return { pos.x + sizes.area, pos.y + sizes.area };
}



Ship::Ship(int segments, int x, int y, int size, int spaceFrame, COLORREF colorLine, COLORREF colorInside) {
    amountSegments = segments;
    sizeSegment = size;
    this->size = { sizeSegment * amountSegments, sizeSegment };
    this->spaceFrame = spaceFrame;
    pos = { x, y, x + this->size.x, y + this->size.y };
    colors = { TX_TRANSPARENT, colorInside, colorLine };
    /*for (int i = 0; i < segments; i++ ) {
        coords.push_back({ x + i * size, y, x + (i + 1) * size, y + size });
    }*/

    draw();
}

void Ship::change_position(POINT points) {
    pos = { points.x, points.y, points.x + size.x, points.y + size.y };
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

Points Ship::get_real_size() {
    return { pos.x1 + spaceFrame, pos.y1 + spaceFrame, pos.x2 - spaceFrame, pos.y2 - spaceFrame };
}

void Ship::draw() {
    draw_rectangle(get_real_size(), 1, colors.frame, colors.area);
}

bool Ship::check_mouse() {
    Points realSize = get_real_size();
    RECT rc = { realSize.x1, realSize.y1, realSize.x2, realSize.y2 };
    if (In(txMousePos(), rc))
        return true;
    return false;
}

POINT Ship::get_size() {
    return size;
}



Button::Button(Points points, char* text, int fontSize, int thickness, 
    AreaColors colors = {TX_WHITE, TX_YELLOW, TX_RED}, unsigned align = DT_CENTER | TA_CENTER) {
    pos = points;
    this->text = text;
    this->fontSize = fontSize;
    this->colors = colors;
    this->thickness = thickness;
    this->align = align;
}

void Button::draw() {
    draw_rectangle(pos, thickness, colors.frame, colors.area);
    draw_text(pos, text, align, fontMain, fontSize, colors.line);
}

RECT Button::get_rect() {
    return { pos.x1, pos.y1, pos.x2, pos.y2 };
}

bool Button::check_mouse() {
    RECT rc = { pos.x1, pos.y1, pos.x2, pos.y2 };
    if (In(txMousePos(), rc))
        return true;
    return false;
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

    char startText[] = "НАЧАТЬ";
    Points pointsStartWindow = { centerX - halfStartButtonX, centerY - halfStartButtonY,
        centerX + halfStartButtonX, centerY + halfStartButtonY };
    Button startButton(pointsStartWindow, startText, 30, 5, { TX_WHITE, TX_LIGHTGRAY, TX_YELLOW });
    startButton.draw();

    // Старт окно
    bool running = false;
    while (!running) {
        while (txMouseButtons() != 1) {}
        running = startButton.check_mouse();
    }

    txSetFillColor(TX_BLACK);
    txClear();

    // Расстановка кораблей на своем поле
    int gameAreaX0 = 30, gameAreaY0 = 30;
    int shipWindowX0 = gameAreaX0 + gameAreaSize + 50, shipWindowY0 = gameAreaY0;
    vector<Ship> ships;

    GameArea gameAreaShip(gameAreaX0, gameAreaY0, gameAreaSize, 8, { TX_WHITE, TX_BLUE, TX_GRAY }, AREA_SHIP);
    int sizeSegment = gameAreaShip.get_segment_size();
    int spaceFrameShip = gameAreaShip.get_space_frame();
    Points pointsShipWindow = { shipWindowX0, shipWindowY0, shipWindowX0 + sizeSegment * (MAX_SHIP_SIZE + 2), 
        shipWindowY0 + sizeSegment * (MAX_SHIP_SIZE + 4) };

    draw_rectangle({ frame_x0, frame_x0, windowWidth - frame_x0, windowHeight - frame_x0 }, 1, TX_WHITE, TX_TRANSPARENT);
    draw_ship_window(pointsShipWindow, 3, { TX_YELLOW, TX_RED, NULL });
    create_ships(pointsShipWindow, ships, { NULL, TX_BROWN, NULL }, sizeSegment, spaceFrameShip);

    char shipText[] = "ЗАПУСТИТЬ";
    int sizeShipButtonX = 150, sizeShipButtonY = 80;
    Points pointsShipButton = { pointsShipWindow.x1, pointsShipWindow.y2 + 100, 
        pointsShipWindow.x1 + sizeShipButtonX, pointsShipWindow.y2 + 100 + sizeShipButtonY };
    Button shipButton(pointsShipButton, shipText, 30, 5);
    shipButton.draw();

    // Расстановка кораблей на игровом поле
    int currentShipIndex = -1;
    running = true;
    //POINT gameAreaShipSize = gameAreaShip.get_end_position();
    while (1) {
        while (txMouseButtons() != 1) {
            running = true;
            // Если сейчас выбран корабль, он двигается по полю согласно курсору
            if (currentShipIndex >= 0 && running) {
                POINT currentShipSize = ships[currentShipIndex].get_size();
                // До следующего клика по полю - по нему корабль устанавливается на поле
                txBegin();
                while (txMouseButtons() != 1) {
                    // Движение корабля по игровому полю
                    //cout << gameAreaShipSize.x << " " << gameAreaShipSize.y << endl;
                    POINT mousePos1 = txMousePos();
                    POINT mousePos2 = ships[currentShipIndex].get_size();
                    mousePos2 = { mousePos1.x + mousePos2.x, mousePos1.y + mousePos2.y };

                    mousePos1 = { (mousePos1.x - gameAreaX0) / sizeSegment - 1, (mousePos1.y - gameAreaY0) / sizeSegment - 1 };
                    mousePos2 = { (mousePos2.x - gameAreaX0) / sizeSegment - 1, (mousePos2.y - gameAreaY0) / sizeSegment - 1 };
                    //cout << mousePos2.x << " " << mousePos2.y << endl;
                    if (mousePos1.x < 0 || mousePos1.x >= AREA_SIZE || mousePos1.y < 0 || mousePos1.y >= AREA_SIZE ||
                        mousePos2.x - 1 >= AREA_SIZE || mousePos2.y - 1 >= AREA_SIZE)
                        continue;
                    POINT newShipPos = gameAreaShip.get_segment(AREA_SIZE * mousePos1.y + mousePos1.x);
                    ships[currentShipIndex].change_position(newShipPos);

                    txSetFillColor(TX_BLACK);
                    txClear();

                    //clear_area(ships[currentShipIndex].get_real_size());
                    draw_ship_window(pointsShipWindow, 3, { TX_YELLOW, TX_RED, NULL });
                    gameAreaShip.draw();
                    shipButton.draw();
                    for (size_t i = 0; i < ships.size(); i++)
                        ships[i].draw();

                    txSleep();
                }
                txEnd();
                currentShipIndex = -1;
                running = false;
                break;
            }
        }
        if (running) {
            if (shipButton.check_mouse())
                break;
            // Чек на клик мышки по кораблю
            for (size_t i = 0; i < ships.size(); i++) {
                if (ships[i].check_mouse()) {
                    currentShipIndex = i;
                    break;
                }
            }
        }
        //running = true;
        //txSleep();
    }
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

void draw_line(Points points) {
    txLine(points.x1, points.y1, points.x2, points.y2);
}

bool check_press_button(RECT rc) {
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

void clear_area(Points points, COLORREF color) {
    txSetColor(color);
    txSetFillColor(color);
    txRectangle(points.x1, points.y1, points.x2, points.y2);
}

void draw_ship_window(Points points, int thickness, AreaColors colorsRect) {
    draw_rectangle(points, thickness, colorsRect.frame, colorsRect.area);
}

void create_ships(Points points, vector<Ship>& ships, AreaColors colorsShip, int size, int spaceFrame) {
    int tempX, tempY = points.y1 + size;

    for (int i = 1; i <= MAX_SHIP_SIZE; i++) {
        tempX = points.x1 + size;
        for (int j = MAX_SHIP_SIZE; j >= i; j--) {
            if (tempX + size * i > points.x2) {
                tempX = points.x1 + size;
                tempY += size;
            }
            ships.push_back(Ship(i, tempX, tempY, size, spaceFrame, colorsShip.line, colorsShip.area));
            tempX += size * i;
        }
        tempY += size;
    }
}

// сделать второй вектор у поля с меньшим квадратом для кораблей
