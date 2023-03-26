#include <cstring>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <SDL.h>
#include <SDL_image.h>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int CELL_SIZE = 20;
const int ROWS = SCREEN_HEIGHT / CELL_SIZE;
const int COLUMNS = SCREEN_WIDTH / CELL_SIZE;

class Snake {
public:
    Snake() {
        // Initialize the snake with a starting position and direction
        srand(time(NULL));
        x = floor(COLUMNS / 2);
        y = floor(ROWS/2);
        direction = rand() % 4;
        // Set the initial length of the snake to 1
        length = 1;
        // Add the initial segment to the vector
        segments.push_back(make_pair(x, y));
    }

    void move() {
        // Move the snake in the current direction
        switch (direction) {
        case 0:
            y--;
            break;
        case 1:
            x++;
            break;
        case 2:
            y++;
            break;
        case 3:
            x--;
            break;
        }
        // Check if the snake has collided with the edges of the screen
        if (x < 0 || x >= COLUMNS || y < 0 || y >= ROWS) {
            alive = false;
        }
        // Check if the snake has collided with itself
        for (int i = 1; i < segments.size(); i++) {
            if (x == segments[i].first && y == segments[i].second) {
                alive = false;
            }
        }
        // Add the new segment to the front of the vector
        segments.insert(segments.begin(), make_pair(x, y));
        // Remove the last segment if the snake is longer than its length
        while (segments.size() > length) {
            segments.pop_back();
        }
    }

    void grow() {
        // Increase the length of the snake
        length++;
    }

    int getX() const {
        return x;
    }

    int getY() const {
        return y;
    }

    bool isAlive() const {
        return alive;
    }

    vector<pair<int, int>> getSegments() const {
        return segments;
    }

    void setDirection(int newDirection) {
        // Change the direction of the snake, but disallow reversing
        if (newDirection != (direction + 2) % 4) {
            direction = newDirection;
        }
    }

private:
    int x;
    int y;
    int direction;
    int length;
    bool alive = true;
    vector<pair<int, int>> segments;
};

class Food {
public:
    Food() {
        // Initialize the food with a random position
        srand(time(NULL));
        x = rand() % COLUMNS;
        y = rand() % ROWS;
    }

    int getX() const {
        return x;
    }

    int getY() const {
        return y;
    }

    void respawn() {
        // Respawn the food with a random position
        x = rand() % COLUMNS;
        y = rand() % ROWS;
    }

private:
    int x;
    int y;
};

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a window
    window = SDL_CreateWindow("Window Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return 1;
    }

    // Create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return 1;
    }

    // Load the background image
    SDL_Surface* backgroundSurface = IMG_Load("background.png");
    if (backgroundSurface == NULL) {
        printf("Failed to load background image: %s\n", SDL_GetError());
        return 1;
    }

    // Convert the surface to a texture
    SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    if (backgroundTexture == NULL) {
        printf("Failed to create background texture: %s\n", SDL_GetError());
        return 1;
    }

    // Load the apple image
    SDL_Surface* appleSurface = IMG_Load("apple.png");
    if (appleSurface == NULL) {
        printf("Failed to load apple image: %s\n", SDL_GetError());
        return 1;
    }

    // Convert the surface to a texture
    SDL_Texture* appleTexture = SDL_CreateTextureFromSurface(renderer, appleSurface);
    if (appleTexture == NULL) {
        printf("Failed to create apple texture: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Rect destinationRect = { 0, 0, CELL_SIZE, CELL_SIZE };

    // Create the snake and food objects
    Snake snake;
    Food food;

    // Set up the game loop
    bool quit = false;
    SDL_Event event;
    Uint32 lastUpdateTime = SDL_GetTicks();
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    snake.setDirection(0);
                    break;
                case SDLK_RIGHT:
                    snake.setDirection(1);
                    break;
                case SDLK_DOWN:
                    snake.setDirection(2);
                    break;
                case SDLK_LEFT:
                    snake.setDirection(3);
                    break;
                }
                break;
            }
        }

        // Update the game state
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastUpdateTime >= 125) {
            // Move the snake
            snake.move();
            // Check if the snake has collided with the food
            if (snake.getX() == food.getX() && snake.getY() == food.getY()) {
                snake.grow();
                food.respawn();
            }
            // Check if the snake is still alive
            if (!snake.isAlive()) {
                cout << "Game over!" << endl;
                quit = true;
            }
            // Update the last update time
            lastUpdateTime = currentTime;
        }

        // Render the game
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);

        // Tile the background texture over the window
        for (int x = 0; x < SCREEN_WIDTH; x += CELL_SIZE) {
            for (int y = 0; y < SCREEN_HEIGHT; y += CELL_SIZE) {
                destinationRect.x = x;
                destinationRect.y = y;
                SDL_RenderCopy(renderer, backgroundTexture, NULL, &destinationRect);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0x70, 0xE0, 0x00, 0xFF);
        // Render the snake
        vector<pair<int, int>> segments = snake.getSegments();
        for (int i = 0; i < segments.size(); i++) {
            SDL_Rect rect = { segments[i].first * CELL_SIZE, segments[i].second * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(renderer, &rect);
        }

        // Render the food
        destinationRect.x = food.getX() * CELL_SIZE;
        destinationRect.y = food.getY() * CELL_SIZE;
        SDL_RenderCopy(renderer, appleTexture, NULL, &destinationRect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(backgroundTexture);
    SDL_FreeSurface(backgroundSurface);
    SDL_DestroyTexture(appleTexture);
    SDL_FreeSurface(appleSurface);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}
