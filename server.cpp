#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "chat.h"

#define PORT 8080

std::unordered_map<std::string, int> clientSockets;

void* handleClient(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[1024] = { 0 };
    std::string clientUsername;

    recv(clientSocket, buffer, 1024, 0);
    clientUsername = buffer;
    clientSockets[clientUsername] = clientSocket;

    Chat<std::string> chat("chat.db"); // Assuming "chat.db" is your database file

    std::cout << clientUsername << " connected." << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, 1024, 0);
        if (bytesRead <= 0) {
            std::cout << clientUsername << " disconnected." << std::endl;
            clientSockets.erase(clientUsername);
            close(clientSocket);
            pthread_exit(NULL);
        }

        std::string message = buffer;
        std::cout << "Received: " << message << std::endl;
        std::string sender, receiver, msg;
        size_t pos = message.find(":");
        sender = message.substr(0, pos);
        message = message.substr(pos + 1);
        pos = message.find(":");
        receiver = message.substr(0, pos);
        msg = message.substr(pos + 1);

        User<std::string>* senderUser = chat.loginUser(sender, ""); // Replace "" with actual password
        User<std::string>* receiverUser = chat.loginUser(receiver, "");

        chat.sendMessage(*senderUser, *receiverUser, msg);

        // ... (Broadcast message to all clients)
    }

    return NULL;
}

int main() {
    int serverSocket, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == 0) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    // Forcefully attach socket to the port
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "Setsockopt failed." << std::endl;
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        return 1;
    }

    // Listen
    if (listen(serverSocket, 3) < 0) {
        std::cerr << "Listen failed." << std::endl;
        return 1;
    }

    std::cout << "Server started on port " << PORT << std::endl;

    while (true) {
        newSocket = accept(serverSocket, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (newSocket < 0) {
            std::cerr << "Accept failed." << std::endl;
            return 1;
        }

        // Create a new thread for each client
        pthread_t threadId;
        if (pthread_create(&threadId, NULL, handleClient, (void*)&newSocket) != 0) {
            std::cerr << "Thread creation failed." << std::endl;
            return 1;
        }
    }

    return 0;
}