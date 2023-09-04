#include <SDL.h>
#include "AStar.hpp"
#include <iostream>

class Visualization {
public:
    Visualization();
    ~Visualization();

    void run();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;

    AStar::Generator generator;
    AStar::CoordinateList path;
    AStar::Vec2i start, end;

    void getUserInput();
    void handleMouseClick(int mouseX, int mouseY);

    bool placingStart;
    bool placingEnd;
    bool placingObstacles;
};

Visualization::Visualization() : placingStart(true), placingEnd(false), placingObstacles(false) {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("A* Visualization", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    generator.setWorldSize({21, 15});

    getUserInput(); // Get user input for start, end, and hurdles
}

Visualization::~Visualization() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Visualization::getUserInput() {
    std::cout << "Click on the grid to place the start point." << std::endl;
}

void Visualization::handleMouseClick(int mouseX, int mouseY) {
    if (placingStart) {
        start = { mouseX / 40, mouseY / 40 };
        placingStart = false;
        placingEnd = true;
        std::cout << "Click on the grid to place the end point." << std::endl;
    } 
    
    else if (placingEnd) {
        end = { mouseX / 40, mouseY / 40 };
        placingEnd = false;
        placingObstacles = true;
        std::cout << "Click on the grid to place obstacles. Press Enter when done." << std::endl;
    } 
    
    else if (placingObstacles) {
        AStar::Vec2i obstacle = { mouseX / 40, mouseY / 40 };
        
            generator.addCollision(obstacle);
        
    }
}

void Visualization::run() {
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                handleMouseClick(event.button.x, event.button.y);
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                if (placingObstacles) {
                    path = generator.findPath(start, end);
                    placingObstacles = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render grid
        for (int x = 0; x < generator.worldSize.x; ++x) {
            for (int y = 0; y < generator.worldSize.y; ++y) {
                SDL_Rect rect = {x * 40, y * 40, 40, 40};
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &rect);
            }
        }

        // Render obstacles
        for (const auto& obstacle : generator.walls) {
            SDL_Rect rect = {obstacle.x * 40, obstacle.y * 40, 40, 40};
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderFillRect(renderer, &rect);
        }

        // Render path
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (const auto& point : path) {
            SDL_Rect rect = {point.x * 40, point.y * 40, 40, 40};
            SDL_RenderFillRect(renderer, &rect);
        }

        // Render start and end points
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect startRect = {start.x * 40, start.y * 40, 40, 40};
        SDL_RenderFillRect(renderer, &startRect);
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_Rect endRect = {end.x * 40, end.y * 40, 40, 40};
        SDL_RenderFillRect(renderer, &endRect);

        SDL_RenderPresent(renderer);
    }
}

int main(int argc, char* argv[]) {
    Visualization visualization;
    visualization.run();
    return 0;
}
