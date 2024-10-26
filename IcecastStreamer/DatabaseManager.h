// DatabaseManager.h
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <pqxx/pqxx>
#include <vector>
#include <string>
#include <memory>

class DatabaseManager {
public:
    static DatabaseManager& getInstance();

    std::vector<std::string> getTracks(int playlist_id);

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

private:
    DatabaseManager();

    std::unique_ptr<pqxx::connection> conn;
};

#endif 
