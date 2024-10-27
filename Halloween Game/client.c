#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 256
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PLAYER_SIZE 70

typedef struct {
    int x, y;
    int isGhost;
    int alive;
} PlayerState;

PlayerState players[3];

void parseState(const char *state) {
    sscanf(state, "STATE %d %d %d %d %d %d %d %d %d %d %d %d",
           &players[0].x, &players[0].y, &players[0].isGhost, &players[0].alive,
           &players[1].x, &players[1].y, &players[1].isGhost, &players[1].alive,
           &players[2].x, &players[2].y, &players[2].isGhost, &players[2].alive);
}

void handlePlayerMovement(int socket, int playerIndex) {
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    int dx = 0, dy = 0;

    if (keystate[SDL_SCANCODE_W]) dy -= 5;
    if (keystate[SDL_SCANCODE_S]) dy += 5;
    if (keystate[SDL_SCANCODE_A]) dx -= 5;
    if (keystate[SDL_SCANCODE_D]) dx += 5;

    char buffer[BUFFER_SIZE];
    sprintf(buffer, "MOVE %d %d %d", playerIndex, dx, dy);
    send(socket, buffer, strlen(buffer), 0);
}

void renderGame(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < 3; i++) {
        if (players[i].alive) {
            SDL_Rect playerRect = {players[i].x, players[i].y, PLAYER_SIZE, PLAYER_SIZE};
            SDL_RenderFillRect(renderer, &playerRect);
        }
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    int playerIndex = argc > 1 ? atoi(argv[1]) : 0;
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr = {0};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    connect(socketFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Client Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        handlePlayerMovement(socketFD, playerIndex);

        char buffer[BUFFER_SIZE] = {0};
        recv(socketFD, buffer, BUFFER_SIZE, 0);
        parseState(buffer);

        renderGame(renderer);
        SDL_Delay(16);
    }

    close(socketFD);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}