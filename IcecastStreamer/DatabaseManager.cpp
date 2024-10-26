// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <iostream>

// Initialize static instance of DatabaseManager
DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

// Constructor (private)
DatabaseManager::DatabaseManager() {}

// Method to fetch list of tracks from the database
std::vector<std::string> DatabaseManager::getTracks(int playlist_id) {
    std::vector<std::string> listOfFiles;
    try {
        // Establish connection if not already open
        if (!conn || !conn->is_open()) {
            conn.reset(new pqxx::connection("dbname=postgres user=postgres password=1234 hostaddr=127.0.0.1 port=5432"));
        }

        // Start a transaction
        pqxx::work txn(*conn);
        pqxx::result result = txn.exec("SELECT filename FROM track WHERE playlist_id = " + std::to_string(playlist_id));

        // Process results
        for (const auto& row : result) {
            listOfFiles.push_back(row["filename"].c_str());
        }

        txn.commit();
    }
    catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }

    return listOfFiles;
}
