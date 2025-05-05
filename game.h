#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>

// Kích thước cửa sổ
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SIZE = 10;
const int CELL_SIZE = 50;
const int GRID_OFFSET_X = 50;
const int GRID_OFFSET_Y = 50;

// Màu sắc
const SDL_Color WHITE = {255, 255, 255, 255};
const SDL_Color RED = {255, 0, 0, 255};
const SDL_Color BLUE = {0, 0, 255, 255};
const SDL_Color GRAY = {128, 128, 128, 255};

// Tàu
struct Ship {
    int size;
    std::vector<std::pair<int, int>> positions;
    bool sunk;
};

class Battleship {
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::vector<std::vector<char>> playerBoard; // ' ' = trống, 'H' = trúng, 'M' = trượt
    std::vector<Ship> ships;
    bool gameOver;
    int shotsFired;
    int hits;

public:
    Battleship();
    ~Battleship();
    void placeShips();
    bool isShipAt(int row, int col);
    void handleClick(int x, int y);
    void checkSunkShips();
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void render();
    void run();
};

#endif // GAME_H