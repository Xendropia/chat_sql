#include "chat.h"
#include <utility>

// User class implementation
template<typename T>
User<T>::User(T username, T password, T name)
    : username(std::move(username)), password(std::move(password)), name(std::move(name)) {}

template<typename T>
const T& User<T>::getUsername() const { return username; }

template<typename T>
const T& User<T>::getPassword() const { return password; }

template<typename T>
const T& User<T>::getName() const { return name; }

template<typename T>
std::vector<std::pair<std::string, T>>& User<T>::getMessages() { return messages; }

// Chat class implementation
template<typename T>
Chat<T>::Chat(const std::string& dbPath) : dbPath(dbPath) {
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
}

template<typename T>
void Chat<T>::registerUser(const T& username, const T& password, const T& name) {
    // Check if the username already exists.
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            throw std::invalid_argument("Username already exists. Please choose a different username.");
        }
    }

    // Insert user into database
    std::string sql = "INSERT INTO users (username, password, name) VALUES ('" +
                      std::string(username) + "', '" + std::string(password) + "', '" +
                      std::string(name) + "')";

    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errmsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errmsg << std::endl;
        sqlite3_free(errmsg);
        return;
    }

    users.emplace_back(username, password, name);
}


template<typename T>
User<T>* Chat<T>::loginUser(const T& username, const T& password) {
    for (auto& user : users) {
        if (user.getUsername() == username && user.getPassword() == password) {
            currentUser = &user;

            // Display messages sent to the user
            std::cout << "Login successful. Welcome, " << currentUser->getName() << "!\n";
            std::cout << "Messages:\n";
            for (const auto& message : currentUser->getMessages()) {
                std::cout << message.first << " " << message.second << '\n';
            }
            return currentUser;
        }
    }
    return nullptr;
}

template<typename T>
void Chat<T>::saveMessageToDatabase(const User<T>& sender, const User<T>& receiver, const T& message) {
    std::string sql = "INSERT INTO messages (sender_id, receiver_id, message) VALUES ("
                      + std::to_string(sender.getUsername()) + ", "
                      + std::to_string(receiver.getUsername()) + ", '"
                      + std::string(message) + "')";

    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errmsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errmsg << std::endl;
        sqlite3_free(errmsg);
    }
}

template<typename T>
void Chat<T>::broadcastMessage(User<T>& sender, const T& message) {
    // Add timestamp to the broadcast message
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(timeinfo, "[%H:%M:%S]");
    std::string timestamp = oss.str();

    for (auto& user : users) {
        if (&user != &sender) {
            user.getMessages().emplace_back(timestamp, sender.getName() + " (Broadcast): " + message);
        }
    }
}

template<typename T>
const std::vector<User<T>>& Chat<T>::getUsers() const {
    return users;
}