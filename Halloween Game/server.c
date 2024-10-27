#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Global variables for managing clients
int clientSockets[MAX_CLIENTS];
int clientCount = 0;
pthread_mutex_t clientMutex = PTHREAD_MUTEX_INITIALIZER;

// Function to broadcast game state to all clients
void broadcastState(const char* message) {
    pthread_mutex_lock(&clientMutex); // Lock for thread-safe access to client list

    for (int i = 0; i < clientCount; i++) {
        if (send(clientSockets[i], message, strlen(message), 0) < 0) {
            perror("Failed to send message to client");
        }
    }

    pthread_mutex_unlock(&clientMutex); // Unlock after sending
}

// Handle client connection
void* handleClient(void* socket) {
    int clientSocket = *(int*)socket;
    char buffer[BUFFER_SIZE];
    ssize_t readSize;

    // Adding the client socket to the global client list
    pthread_mutex_lock(&clientMutex);
    clientSockets[clientCount++] = clientSocket;
    pthread_mutex_unlock(&clientMutex);

    // Main loop to handle client communication
    while ((readSize = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[readSize] = '\0';
        printf("Received from client: %s\n", buffer);

        // Example of broadcasting received message to all clients
        broadcastState(buffer);
    }

    // Remove client from list on disconnect
    pthread_mutex_lock(&clientMutex);
    for (int i = 0; i < clientCount; i++) {
        if (clientSockets[i] == clientSocket) {
            clientSockets[i] = clientSockets[--clientCount]; // Remove client and adjust count
            break;
        }
    }
    pthread_mutex_unlock(&clientMutex);

    close(clientSocket);
    free(socket);
    printf("Client disconnected\n");
    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Set server address and port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Bind the socket to the address and port
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Failed to listen on socket");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", SERVER_PORT);

    // Accept clients and create threads to handle them
    while ((clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen))) {
        printf("New client connected\n");

        // Allocate memory for new client socket
        int* newClientSocket = malloc(sizeof(int));
        if (!newClientSocket) {
            perror("Failed to allocate memory for client socket");
            close(clientSocket);
            continue;
        }
        *newClientSocket = clientSocket;

        // Create a new thread to handle the client
        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClient, (void*)newClientSocket) != 0) {
            perror("Failed to create client thread");
            free(newClientSocket);
            close(clientSocket);
            continue;
        }

        // Detach thread to handle cleanup automatically
        pthread_detach(clientThread);
    }

    if (clientSocket < 0) {
        perror("Failed to accept client connection");
    }

    // Close server socket
    close(serverSocket);
    return 0;
}