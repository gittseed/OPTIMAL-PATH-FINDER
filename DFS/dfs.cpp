#include <iostream>
#include <SDL.h>
#include <vector>
#include <stack>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 600;
const int GRID_SIZE = 10;
const int ROWS = SCREEN_HEIGHT / GRID_SIZE;
const int COLUMNS = SCREEN_WIDTH / GRID_SIZE;

struct Node {
    int x, y;
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
            start = { gridX, gridY };
            grid[gridY][gridX] = 2; // Start
            currentState = INPUT_END;
        }
        else if (currentState == INPUT_END) {
            end = { gridX, gridY };
            grid[gridY][gridX] = 3; // End
            currentState = INPUT_HURDLES;
        }
        else if (currentState == INPUT_HURDLES) {
            // Left click sets hurdles
            if (event.button.button == SDL_BUTTON_LEFT) {
                grid[gridY][gridX] = 1; // Hurdle
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
                SDL_SetRenderDrawColor(gRenderer, 255, 255, 0, 255); // Visited nodes (yellow)
            }
            else if (grid[i][j] == 5) {
                SDL_SetRenderDrawColor(gRenderer, 0, 255, 0, 255); // Final path (green)
            }

            SDL_Rect cellRect = { j * GRID_SIZE, i * GRID_SIZE, GRID_SIZE, GRID_SIZE };
            SDL_RenderFillRect(gRenderer, &cellRect);

            // Reset color to white
            SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
        }
    }
}

bool depthFirstSearchWithDelay(int delayMs) {
    std::vector<std::vector<bool>> visited(ROWS, std::vector<bool>(COLUMNS, false));
    std::stack<Node> stack;

    // Create a 2D array to keep track of the parent node for each cell
    std::vector<std::vector<Node>> parent(ROWS, std::vector<Node>(COLUMNS, { -1, -1 }));

    stack.push(start);

    while (!stack.empty()) {
        Node current = stack.top();
        stack.pop();

        int x = current.x;
        int y = current.y;

        if (x == end.x && y == end.y) {
            // Reached the end, reconstruct the path
            while (x != -1 && y != -1) {
                grid[y][x] = 5; // Mark the final path as green

                // Move to the parent node
                Node parentNode = parent[y][x];
                x = parentNode.x;
                y = parentNode.y;
            }
            grid[start.y][start.x] = 2; // Start in green
            grid[end.y][end.x] = 3;     // End in red
            return true;
        }

        visited[y][x] = true;
        grid[y][x] = 4; // Mark visited nodes as yellow

        // Generate neighbors
        int dx[] = { -1, 1, 0, 0 };
        int dy[] = { 0, 0, -1, 1 };

        for (int i = 0; i < 4; i++) {
            int newX = x + dx[i];
            int newY = y + dy[i];

            // Check if the neighbor is within bounds
            if (newX >= 0 && newX < COLUMNS && newY >= 0 && newY < ROWS && grid[newY][newX] != 1 && !visited[newY][newX]) {
                stack.push({ newX, newY });
                parent[newY][newX] = { x, y }; // Set the parent of the neighbor

                // Render the grid with the delay
                SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
                SDL_RenderClear(gRenderer);
                renderGrid();
                SDL_RenderPresent(gRenderer);

                // Delay for visualization
                SDL_Delay(delayMs);
            }
        }
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
                    if (depthFirstSearchWithDelay(100)) { // Delay of 100 milliseconds between iterations
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