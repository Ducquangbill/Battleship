#include "game.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

// Hàm khởi tạo
Battleship::Battleship() : window(nullptr), renderer(nullptr), font(nullptr), gameOver(false), shotsFired(0), hits(0) {
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Lỗi khởi tạo SDL: " << SDL_GetError() << std::endl;
        return;
    }
    if (TTF_Init() < 0) {
        std::cerr << "Lỗi khởi tạo TTF: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    // Tạo cửa sổ và bộ dựng
    window = SDL_CreateWindow("Battleship PvE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Lỗi tạo cửa sổ: " << SDL_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Lỗi tạo bộ dựng: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    font = TTF_OpenFont("fonts/dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf", 24);
    if (!font) {
        std::cerr << "Lỗi tải phông chữ: " << TTF_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return;
    }

    // Khởi tạo bảng người chơi
    playerBoard = std::vector<std::vector<char>>(GRID_SIZE, std::vector<char>(GRID_SIZE, ' '));
    srand(time(0));
    placeShips();
}

// Hàm hủy
Battleship::~Battleship() {
    if (font) TTF_CloseFont(font);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

// Đặt tàu ngẫu nhiên
void Battleship::placeShips() {
    int sizes[] = {5, 4, 3, 3, 2};
    for (int size : sizes) {
        Ship ship;
        ship.size = size;
        ship.sunk = false;
        bool placed = false;
        while (!placed) {
            // Vị trí và hướng ngẫu nhiên
            int row = rand() % GRID_SIZE;
            int col = rand() % GRID_SIZE;
            bool horizontal = rand() % 2;

            // Kiểm tra xem vị trí đặt có được không
            bool fits = true;
            std::vector<std::pair<int, int>> tempPositions;
            for (int i = 0; i < size; ++i) {
                int r = row + (horizontal ? 0 : i);
                int c = col + (horizontal ? i : 0);
                if (r >= GRID_SIZE || c >= GRID_SIZE || isShipAt(r, c)) {
                    fits = false;
                    break;
                }
                tempPositions.push_back({r, c});
            }

            // Đặt tàu nếu được
            if (fits) {
                ship.positions = tempPositions;
                ships.push_back(ship);
                placed = true;
            }
        }
    }
}

// Kiểm tra xem có tàu ở vị trí đã cho không
bool Battleship::isShipAt(int row, int col) {
    for (const auto& ship : ships) {
        for (const auto& pos : ship.positions) {
            if (pos.first == row && pos.second == col) {
                return true;
            }
        }
    }
    return false;
}

// Xử lý nhấp chuột
void Battleship::handleClick(int x, int y) {
    if (gameOver) return;

    int row = (y - GRID_OFFSET_Y) / CELL_SIZE;
    int col = (x - GRID_OFFSET_X) / CELL_SIZE;

    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) return;
    if (playerBoard[row][col] != ' ') return; // Đã bị tấn công

    shotsFired++;
    if (isShipAt(row, col)) {
        playerBoard[row][col] = 'H';
        hits++;
        checkSunkShips();
    } else {
        playerBoard[row][col] = 'M';
    }

    // Kiểm tra điều kiện thắng
    if (hits == 17) {
        gameOver = true;
    }
}

// Kiểm tra xem có tàu nào bị chìm không
void Battleship::checkSunkShips() {
    for (auto& ship : ships) {
        if (ship.sunk) continue;
        bool allHit = true;
        for (const auto& pos : ship.positions) {
            if (playerBoard[pos.first][pos.second] != 'H') {
                allHit = false;
                break;
            }
        }
        if (allHit) ship.sunk = true;
    }
}

// Hiển thị văn bản
void Battleship::renderText(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, nullptr, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Hiển thị trò chơi
void Battleship::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Vẽ lưới
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int i = 0; i <= GRID_SIZE; ++i) {
        SDL_RenderDrawLine(renderer, GRID_OFFSET_X, GRID_OFFSET_Y + i * CELL_SIZE,
                           GRID_OFFSET_X + GRID_SIZE * CELL_SIZE, GRID_OFFSET_Y + i * CELL_SIZE);
        SDL_RenderDrawLine(renderer, GRID_OFFSET_X + i * CELL_SIZE, GRID_OFFSET_Y,
                           GRID_OFFSET_X + i * CELL_SIZE, GRID_OFFSET_Y + GRID_SIZE * CELL_SIZE);
    }

    // Vẽ các ô
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            SDL_Rect cell = {GRID_OFFSET_X + j * CELL_SIZE, GRID_OFFSET_Y + i * CELL_SIZE, CELL_SIZE, CELL_SIZE};
            if (playerBoard[i][j] == 'H') {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                SDL_RenderFillRect(renderer, &cell);
            } else if (playerBoard[i][j] == 'M') {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }

    // Vẽ tàu bị chìm
    for (const auto& ship : ships) {
        if (ship.sunk) {
            for (const auto& pos : ship.positions) {
                SDL_Rect cell = {GRID_OFFSET_X + pos.second * CELL_SIZE, GRID_OFFSET_Y + pos.first * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }

    // Hiển thị trạng thái
    renderText("Shots Fired: " + std::to_string(shotsFired), 600, 50, WHITE);
    renderText("Hits: " + std::to_string(hits), 600, 80, WHITE);
    if (gameOver) {
        renderText("You Won!", 600, 110, WHITE);
    }

    SDL_RenderPresent(renderer);
}

// Vòng lặp trò chơi
void Battleship::run() {
    SDL_Event event;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                handleClick(event.button.x, event.button.y);
            }
        }
        render();
        SDL_Delay(10);
    }
}

int main(int argc, char* argv[]) {
    Battleship game;
    game.run();
    return 0;
}