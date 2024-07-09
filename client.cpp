#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include "chat.h" // Include the chat.h header file

#define PORT 8080 
#define SERVER_IP "127.0.0.1" // Change to server IP if needed

void* receiveMessages(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[1024] = { 0 };
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, 1024, 0);
        if (bytesRead > 0) {
            std::cout << buffer << std::endl; // Print messages from server
        }
    }
    return NULL;
}

int main() {
    int clientSocket = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error." << std::endl;
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported." << std::endl;
        return 1;
    }

    // Connect to the server
    if (connect(clientSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        return 1;
    }

    std::string username;
    std::cout << "Enter your username: ";
    std::cin >> username;
    send(clientSocket, username.c_str(), username.size(), 0); // Send username to server

    // Create a thread to receive messages
    pthread_t receiveThread;
    if (pthread_create(&receiveThread, NULL, receiveMessages, (void*)&clientSocket) != 0) {
        std::cerr << "Thread creation failed." << std::endl;
        return 1;
    }

    // Main loop for sending messages
    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    // Close the socket (unreachable in this example)
    close(clientSocket);
    return 0;
}