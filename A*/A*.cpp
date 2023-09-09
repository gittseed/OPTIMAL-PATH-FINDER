#include <iostream>
#include <SDL.h>
#include <vector>
#include <queue>
#include <cmath>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 10;
const int ROWS = SCREEN_HEIGHT / GRID_SIZE;
const int COLUMNS = SCREEN_WIDTH / GRID_SIZE;

struct Node {
    int x, y;
    double g; // Cost from start node to current node
    double h; // Heuristic value (Manhattan distance to end node)
    double f; // Total cost (g + h)

    // Operator for priority queue
    bool operator<(const Node& other) const {
        return f > other.f;
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
            start = { gridX, gridY, 0, 0, 0 };
            grid[gridY][gridX] = 2;
            currentState = INPUT_END;
        }
        else if (currentState == INPUT_END) {
            end = { gridX, gridY, 0, 0, 0 };
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
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 0, 255); // Path (yellow)
            }
            else if (grid[i][j] == 5) {
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255); // Path (green)
            }

            SDL_Rect cellRect = { j * GRID_SIZE, i * GRID_SIZE, GRID_SIZE, GRID_SIZE };
            SDL_RenderFillRect(gRenderer, &cellRect);

            // Reset color to white
            SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        }
    }
}

// Function to calculate the Manhattan distance heuristic
double calculateManhattanDistance(const Node& a, const Node& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

// Function to perform A* algorithm with delay
bool aStarWithDelay(int delayMs) {
    std::vector<std::vector<bool>> closedSet(ROWS, std::vector<bool>(COLUMNS, false));
    std::vector<std::vector<Node>> cameFrom(ROWS, std::vector<Node>(COLUMNS)); // Keep track of where each node came from
    std::priority_queue<Node> openSet;

    // Add the start node to the open set
    openSet.push(start);

    while (!openSet.empty()) {
        Node current = openSet.top();
        openSet.pop();

        int x = current.x;
        int y = current.y;

        if (x == end.x && y == end.y) {
            // Reached the end, reconstruct the path
            while (!(x == start.x && y == start.y)) {
                grid[y][x] = 4; // Mark the path as yellow

                Node previous = cameFrom[y][x];
                x = previous.x;
                y = previous.y;
            }
            grid[start.y][start.x] = 2; // Start in green
            grid[end.y][end.x] = 3;     // End in red

            // Highlight the shortest path from start to end in green
            x = end.x;
            y = end.y;
            while (!(x == start.x && y == start.y)) {
                grid[y][x] = 5; // Mark the path as green
                Node previous = cameFrom[y][x];
                x = previous.x;
                y = previous.y;
            }

            return true;
        }

        closedSet[y][x] = true;

        // Generate neighbors
        int dx[] = {-1, 1, 0, 0};
        int dy[] = {0, 0, -1, 1};

        for (int i = 0; i < 4; i++) {
            int newX = x + dx[i];
            int newY = y + dy[i];

            // Check if the neighbor is within bounds
            if (newX >= 0 && newX < COLUMNS && newY >= 0 && newY < ROWS && grid[newY][newX] != 1 && !closedSet[newY][newX]) {
                // Calculate new g value (cost from start)
                double tentativeG = current.g + 1.0;

                bool isBetterPath = false;

                // Check if this is the first visit to the neighbor
                if (grid[newY][newX] != 4) {
                    grid[newY][newX] = 4; // Mark it as part of the path
                    isBetterPath = true;
                }
                else if (tentativeG < current.g) {
                    isBetterPath = true;
                }

                if (isBetterPath) {
                    // Update neighbor node
                    Node neighbor;
                    neighbor.x = newX;
                    neighbor.y = newY;
                    neighbor.g = tentativeG;
                    neighbor.h = calculateManhattanDistance(neighbor, end);
                    neighbor.f = neighbor.g + neighbor.h;

                    openSet.push(neighbor);

                    // Keep track of where the neighbor came from
                    cameFrom[newY][newX] = current;
                }
            }
        }

        // Render the grid with the delay
        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderClear(gRenderer);
        renderGrid();
        SDL_RenderPresent(gRenderer);

        // Delay for visualization
        SDL_Delay(delayMs);
    }

    // Path not found
    return false;
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
                    if (aStarWithDelay(50)) { // Delay of 100 milliseconds between iterations
                        currentState = INPUT_DONE;
                    }
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
