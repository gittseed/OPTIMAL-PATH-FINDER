#include <iostream>
#include <SDL.h>
#include <vector>
#include <queue>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 10;
const int ROWS = SCREEN_HEIGHT / GRID_SIZE;
const int COLUMNS = SCREEN_WIDTH / GRID_SIZE;

struct Node {
    int x, y;
    int distance;

    // Less-than operator for priority queue
    bool operator<(const Node& other) const {
        return distance > other.distance;
    }
};

// SDL window and renderer
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;

// Grid representation
std::vector<std::vector<int>> grid(ROWS, std::vector<int>(COLUMNS, 0));

// Start and end points
Node start, end;

// State variable to track input order
enum InputState { INPUT_START, INPUT_END, INPUT_HURDLES, INPUT_DONE };
InputState currentState = INPUT_START;

// Function to initialize SDL
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    gWindow = SDL_CreateWindow("Pathfinding Visualization", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

// Function to handle mouse clicks
void handleMouseClick(SDL_Event& event) {
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    int gridX = mouseX / GRID_SIZE;
    int gridY = mouseY / GRID_SIZE;

    if (gridX >= 0 && gridX < COLUMNS && gridY >= 0 && gridY < ROWS) {
        if (currentState == INPUT_START) {
            start = { gridX, gridY, 0 };
            grid[gridY][gridX] = 2;
            currentState = INPUT_END;
        }
        else if (currentState == INPUT_END) {
            end = { gridX, gridY, 0 };
            grid[gridY][gridX] = 3;
            currentState = INPUT_HURDLES;
        }
        else if (currentState == INPUT_HURDLES) {
            // Left click sets hurdles
            if (event.button.button == SDL_BUTTON_LEFT) {
                grid[gridY][gridX] = 1;
            }
        }
    }
}

// Function to render the grid
void renderGrid() {
    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            if (grid[i][j] == 1) {
                SDL_SetRenderDrawColor(gRenderer, 139, 69, 19, 255); // Brown for hurdles
            }
            else if (grid[i][j] == 2) {
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255); // Start (green)
            }
            else if (grid[i][j] == 3) {
                SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 255); // End (red)
            }
            else if (grid[i][j] == 4) {
                SDL_SetRenderDrawColor(gRenderer, 225, 255, 0, 255); // Path (yellow)
            }
            else if (grid[i][j] == 5) {
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255); // Path (green)
            }

            SDL_Rect cellRect = { j * GRID_SIZE, i * GRID_SIZE, GRID_SIZE, GRID_SIZE };
            SDL_RenderFillRect(gRenderer, &cellRect);
            SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        }
    }
}

// Function to calculate the Manhattan distance between two nodes
int manhattanDistance(const Node& a, const Node& b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

// Function to perform Dijkstra's algorithm with a specified delay
void dijkstraWithDelay(int delayMilliseconds) {
    std::vector<std::vector<int>> distance(ROWS, std::vector<int>(COLUMNS, INT_MAX));
    std::vector<std::vector<std::pair<int, int>>> parent(ROWS, std::vector<std::pair<int, int>>(COLUMNS, { -1, -1 }));
    std::priority_queue<std::pair<int, Node>> pq;

    distance[start.y][start.x] = 0;
    pq.push({ 0, start });

    bool foundEnd = false; // Flag to check if we've found the end node

    while (!pq.empty()) {
        Node current = pq.top().second;
        pq.pop();

        int x = current.x;
        int y = current.y;
        int dist = current.distance;

        // If we've reached the end, set the flag
        if (x == end.x && y == end.y) {
            foundEnd = true;
        }

        // Visit neighbors (up, down, left, right)
        int dx[] = { -1, 1, 0, 0 };
        int dy[] = { 0, 0, -1, 1 };

        for (int i = 0; i < 4; i++) {
            int newX = x + dx[i];
            int newY = y + dy[i];

            if (newX >= 0 && newX < COLUMNS && newY >= 0 && newY < ROWS && grid[newY][newX] != 1) {
                int newDist = dist + manhattanDistance({ newX, newY }, end); // Manhattan distance as weight

                if (newDist < distance[newY][newX]) {
                    distance[newY][newX] = newDist;
                    parent[newY][newX] = { x, y };
                    pq.push({ -newDist, { newX, newY, newDist } });

                    // Delay for visualization
                    SDL_Delay(delayMilliseconds);

                    // Render the grid after the delay
                    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                    SDL_RenderClear(gRenderer);
                    renderGrid();
                    SDL_RenderPresent(gRenderer);
                }
            }
        }

        // If we've found the end node, break the loop
        if (foundEnd) {
            break;
        }
    }

    // Mark visited nodes (excluding the final path) as yellow
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++) {
            if (grid[i][j] == 4) {
                grid[i][j] = 0; // Reset the color for previously marked path nodes
            }
            if (grid[i][j] == 0 && distance[i][j] != INT_MAX) {
                grid[i][j] = 4; // Mark visited nodes as yellow
            }
        }
    }

    int x = end.x;
    int y = end.y;

    // Reconstruct the final path and mark it as green
    while (x != start.x || y != start.y) {
        grid[y][x] = 5; // Mark the path as green
        int newX = parent[y][x].first;
        int newY = parent[y][x].second;
        x = newX;
        y = newY;
    }
}

int main() {
    if (!init()) {
        return 1;
    }

    SDL_Event e;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN) {
                handleMouseClick(e);
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                if (start.x != -1 && end.x != -1 && currentState == INPUT_HURDLES) {
                    dijkstraWithDelay(0); // You can adjust the delay here
                    currentState = INPUT_DONE;
                }
            }
        }

        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);

        renderGrid();

        SDL_RenderPresent(gRenderer);
    }

    SDL_DestroyWindow(gWindow);
    SDL_Quit();
    return 0;
}