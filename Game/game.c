#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SIZE 70
#define GHOST_SIZE 70
#define OBSTACLE_SIZE 50
#define NUM_PLAYERS 3  // 2 players + 1 ghost
#define NUM_OBSTACLES 3
#define TIME_LIMIT 60
#define GHOST_SPEED 2 // Reduced ghost speed

// Structures
typedef struct {
    int x, y;
    int isGhost;
    int alive;
    SDL_Texture* texture;
} Player;

typedef struct {
    int x, y;
    SDL_Texture* texture;
} Obstacle;

// Global variables
Player players[NUM_PLAYERS];
Obstacle obstacles[NUM_OBSTACLES];
SDL_Texture* backgroundTexture;
SDL_Texture* botWinTexture;
SDL_Texture* ghostWinTexture;
TTF_Font* font;
Mix_Music* backgroundMusic;
Mix_Chunk* teleportSound;
Mix_Chunk* catchSound;

// Function prototypes
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);
SDL_Texture* createTextTexture(const char* text, SDL_Color color, SDL_Renderer* renderer);
void initPlayers(Player players[], int numPlayers, SDL_Renderer* renderer);
void initObstacles(SDL_Renderer* renderer);
void initAudio(void);
void teleportPlayer(Player* player);
void handlePlayerMovement(Player* player, const Uint8* keystate, SDL_Scancode up, SDL_Scancode down, SDL_Scancode left, SDL_Scancode right);
void moveGhost(Player* ghost, Player players[]);
void checkGhostCollisions(Player players[]);
void checkCollisions(Player players[], Obstacle obstacles[], int numPlayers);
int checkGameOver(Player players[], int numPlayers);
void renderGame(SDL_Renderer* renderer, Player players[], Obstacle obstacles[], int numPlayers, int remainingTime);
void cleanupAudio(void);

// Function implementations
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path);
    if (!texture) {
        printf("Failed to load texture %s: %s\n", path, SDL_GetError());
        return NULL;
    }
    return texture;
}

SDL_Texture* createTextTexture(const char* text, SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void initAudio(void) {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return;
    }
    backgroundMusic = Mix_LoadMUS("backgroundMusic.wav");
    teleportSound = Mix_LoadWAV("teleportSound.wav");
    catchSound = Mix_LoadWAV("catchSound.wav");
    Mix_PlayMusic(backgroundMusic, -1);
}

void cleanupAudio(void) {
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(teleportSound);
    Mix_FreeChunk(catchSound);
    Mix_CloseAudio();
}

void initPlayers(Player players[], int numPlayers, SDL_Renderer* renderer) {
    const char* texturePaths[] = {"bot.png", "bot.png", "ghost.png"};
    for (int i = 0; i < numPlayers; i++) {
        players[i].x = rand() % (SCREEN_WIDTH - PLAYER_SIZE);
        players[i].y = rand() % (SCREEN_HEIGHT - PLAYER_SIZE);
        players[i].isGhost = (i == 2);
        players[i].alive = 1;
        players[i].texture = loadTexture(texturePaths[i], renderer);
    }
}

void initObstacles(SDL_Renderer* renderer) {
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        obstacles[i].x = rand() % (SCREEN_WIDTH - OBSTACLE_SIZE);
        obstacles[i].y = rand() % (SCREEN_HEIGHT - OBSTACLE_SIZE);
        obstacles[i].texture = loadTexture("wormhole.png", renderer);
    }
}

void teleportPlayer(Player* player) {
    player->x = rand() % (SCREEN_WIDTH - PLAYER_SIZE);
    player->y = rand() % (SCREEN_HEIGHT - PLAYER_SIZE);
    Mix_PlayChannel(-1, teleportSound, 0);
}

void handlePlayerMovement(Player* player, const Uint8* keystate, SDL_Scancode up, SDL_Scancode down, SDL_Scancode left, SDL_Scancode right) {
    if (!player->alive || player->isGhost) return;
    int speed = 5;
    if (keystate[up]) player->y = fmax(0, player->y - speed);
    if (keystate[down]) player->y = fmin(SCREEN_HEIGHT - PLAYER_SIZE, player->y + speed);
    if (keystate[left]) player->x = fmax(0, player->x - speed);
    if (keystate[right]) player->x = fmin(SCREEN_WIDTH - PLAYER_SIZE, player->x + speed);
}

void moveGhost(Player* ghost, Player players[]) {
    if (!ghost->isGhost) return;
    Player* target = NULL;
    float minDist = INFINITY;
    for (int i = 0; i < 2; i++) {
        if (players[i].alive) {
            float dx = players[i].x - ghost->x;
            float dy = players[i].y - ghost->y;
            float dist = sqrt(dx * dx + dy * dy);
            if (dist < minDist) {
                minDist = dist;
                target = &players[i];
            }
        }
    }
    if (target) {
        float dx = target->x - ghost->x;
        float dy = target->y - ghost->y;
        float dist = sqrt(dx * dx + dy * dy);
        if (dist > 0) {
            ghost->x += (dx / dist) * GHOST_SPEED;
            ghost->y += (dy / dist) * GHOST_SPEED;
        }
    }
}

void checkGhostCollisions(Player players[]) {
    Player* ghost = &players[2];
    for (int i = 0; i < 2; i++) {
        if (players[i].alive) {
            int dx = abs(ghost->x - players[i].x);
            int dy = abs(ghost->y - players[i].y);
            if (dx < PLAYER_SIZE / 2 && dy < PLAYER_SIZE / 2) {
                players[i].alive = 0;
                Mix_PlayChannel(-1, catchSound, 0);
            }
        }
    }
}

void checkCollisions(Player players[], Obstacle obstacles[], int numPlayers) {
    for (int i = 0; i < numPlayers; i++) {
        if (!players[i].alive) continue;
        for (int j = 0; j < NUM_OBSTACLES; j++) {
            int dx = abs(players[i].x - obstacles[j].x);
            int dy = abs(players[i].y - obstacles[j].y);
            if (dx < (PLAYER_SIZE + OBSTACLE_SIZE) / 2 && dy < (PLAYER_SIZE + OBSTACLE_SIZE) / 2) {
                teleportPlayer(&players[i]);
            }
        }
    }
}

int checkGameOver(Player players[], int numPlayers) {
    int livingPlayers = 0;
    for (int i = 0; i < numPlayers - 1; i++) { // Checking only the two players
        if (players[i].alive) {
            livingPlayers++;
        }
    }
    return livingPlayers == 0; // Returns true if all players are dead
}

void renderGame(SDL_Renderer* renderer, Player players[], Obstacle obstacles[], int numPlayers, int remainingTime) {
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);
    for (int i = 0; i < NUM_OBSTACLES; i++) {
        SDL_Rect obstacleRect = { obstacles[i].x, obstacles[i].y, OBSTACLE_SIZE, OBSTACLE_SIZE };
        SDL_RenderCopy(renderer, obstacles[i].texture, NULL, &obstacleRect);
    }
    for (int i = 0; i < numPlayers; i++) {
        if (players[i].alive) {
            SDL_Rect playerRect = { players[i].x, players[i].y, PLAYER_SIZE, PLAYER_SIZE };
            SDL_RenderCopy(renderer, players[i].texture, NULL, &playerRect);
        }
    }
    char timerText[32];
    sprintf(timerText, "Time: %d", remainingTime);
    SDL_Color timerColor = {255, 255, 255, 255};
    SDL_Texture* timerTexture = createTextTexture(timerText, timerColor, renderer);
    if (timerTexture) {
        SDL_Rect timerRect = {10, 10, 100, 30};
        SDL_RenderCopy(renderer, timerTexture, NULL, &timerRect);
        SDL_DestroyTexture(timerTexture);
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    TTF_Init();
    SDL_Window* window = SDL_CreateWindow("Halloween Chase Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    backgroundTexture = loadTexture("background.png", renderer);
    botWinTexture = loadTexture("botwin.png", renderer);
    ghostWinTexture = loadTexture("ghostwin.png", renderer);
    font = TTF_OpenFont("deadly_bones.ttf", 24);
    initPlayers(players, NUM_PLAYERS, renderer);
    initObstacles(renderer);
    initAudio();
    int running = 1, startTime = SDL_GetTicks();
while (running) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = 0;

    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    handlePlayerMovement(&players[0], keystate, SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D);
    handlePlayerMovement(&players[1], keystate, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT);
    moveGhost(&players[2], players);
    checkGhostCollisions(players);
    checkCollisions(players, obstacles, NUM_PLAYERS);

    int elapsedTime = (SDL_GetTicks() - startTime) / 1000;
    int remainingTime = TIME_LIMIT - elapsedTime;
    renderGame(renderer, players, obstacles, NUM_PLAYERS, remainingTime);

    if (checkGameOver(players, NUM_PLAYERS)) {
        // Show ghost win screen
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, ghostWinTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(3000);  // Show end screen
        running = 0;
    } else if (remainingTime <= 0) {
        // Show bot win screen if time runs out and ghost hasn't caught players
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, botWinTexture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(3000);  // Show end screen
        running = 0;
    }

    SDL_Delay(16);
}

    cleanupAudio();
    for (int i = 0; i < NUM_PLAYERS; i++) SDL_DestroyTexture(players[i].texture);
    for (int i = 0; i < NUM_OBSTACLES; i++) SDL_DestroyTexture(obstacles[i].texture);
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(botWinTexture);
    SDL_DestroyTexture(ghostWinTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
