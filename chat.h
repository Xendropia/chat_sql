#ifndef CHAT_H
#define CHAT_H

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <stdexcept>
#include <unordered_map>
#include <sqlite3.h>

template<typename T>
class User {
public:
    User(T username, T password, T name);
    const T& getUsername() const;
    const T& getPassword() const;
    const T& getName() const;
    std::vector<std::pair<std::string, T>>& getMessages();

private:
    T username;
    T password;
    T name;
    std::vector<std::pair<std::string, T>> messages;
};

template<typename T>
class Chat {
public:
    Chat(const std::string& dbPath);
    void registerUser(const T& username, const T& password, const T& name);
    User<T>* loginUser(const T& username, const T& password);
    void sendMessage(User<T>& sender, User<T>& receiver, const T& message);
    void broadcastMessage(User<T>& sender, const T& message);
    const std::vector<User<T>>& getUsers() const;

private:
    std::vector<User<T>> users;
    User<T>* currentUser;
    sqlite3* db;
    std::string dbPath;

    void saveMessageToDatabase(const User<T>& sender, const User<T>& receiver, const T& message);
};