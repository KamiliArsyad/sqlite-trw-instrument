#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sqlite3.h>
#include <cstdio> // For std::remove
#include <chrono>
#include <thread>

// SQL script for initializing the database
const char* init_sql = R"SQL(
CREATE TABLE IF NOT EXISTS employees (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    department TEXT NOT NULL,
    salary INTEGER NOT NULL
);

INSERT INTO employees (name, department, salary) VALUES ('Alice', 'HR', 50000);
INSERT INTO employees (name, department, salary) VALUES ('Bob', 'Engineering', 75000);
INSERT INTO employees (name, department, salary) VALUES ('Charlie', 'Marketing', 60000);
)SQL";

// Callback function to display query results
int callback(void* NotUsed, int argc, char** argv, char** azColName){
    for(int i = 0; i < argc; i++){
        std::cout << (azColName[i] ? azColName[i] : "NULL") << " = "
                  << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    std::cout << "--------------------------\n";
    return 0;
}

// Function to execute SQL commands with retry logic
void execute_sql(sqlite3* db, const char* sql, bool display_results = false) {
    char* err_msg = nullptr;
    int rc;
    int retries = 1; // Number of retries
    int delay_ms = 100; // Delay between retries in milliseconds

    while (retries-- > 0) {
        if (display_results) {
            rc = sqlite3_exec(db, sql, callback, nullptr, &err_msg);
        } else {
            rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg);
        }

        if (rc == SQLITE_OK) {
            break; // Success
        } else if (rc == SQLITE_BUSY) {
            // Database is locked, wait and retry
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            continue;
        } else {
            // Some other error occurred
            std::cerr << "SQL error: " << err_msg << "\n";
            sqlite3_free(err_msg);
            break;
        }
    }

    if (rc == SQLITE_BUSY) {
        std::cerr << "SQL error: Database is locked after multiple retries.\n";
    }
}

// Thread task function
void thread_task(const std::string& db_filename, int thread_id) {
    // Open a new database connection for this thread
    setThreadId(thread_id);
    sqlite3* db;
    if (sqlite3_open(db_filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Thread " << thread_id << " can't open database: "
                  << sqlite3_errmsg(db) << "\n";
        return;
    }

    // Set busy timeout (not needed if using retry logic)
    sqlite3_busy_timeout(db, 1000);

    // Each thread performs multiple transactions
    for (int i = 0; i < 3; ++i) {
        // Begin transaction
        execute_sql(db, "BEGIN TRANSACTION;");

        // Update salaries
        std::string update_sql = "UPDATE employees SET salary = salary + "
                                 + std::to_string(1000 * (i + 1))
                                 + " WHERE department = 'Engineering';";
        execute_sql(db, update_sql.c_str());

        // Read
        std::string read_sql = "SELECT name, department FROM employees WHERE salary > 3000";
        execute_sql(db, read_sql.c_str());

        // Insert a new employee
        std::string insert_sql = "INSERT INTO employees (name, department, salary) VALUES ('Employee_"
                                 + std::to_string(thread_id) + "_" + std::to_string(i)
                                 + "', 'Engineering', 70000);";
        execute_sql(db, insert_sql.c_str());

        // Delete employees with high salaries
        execute_sql(db, "DELETE FROM employees WHERE salary > 100000;");

        // Commit transaction
        execute_sql(db, "COMMIT;");
    }

    // Close the database connection
    sqlite3_close(db);
}

int main() {
    constexpr int NUM_THREADS = 5;
    const std::string db_filename = "test.db";

    // Remove existing database file
    std::remove(db_filename.c_str());

    // Open database to initialize it
    sqlite3* db;
    if (sqlite3_open(db_filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    // Execute initialization SQL
    execute_sql(db, init_sql);

    // Close the initialization database connection
    sqlite3_close(db);

    // Launch threads
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(thread_task, db_filename, i);
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Open the database to display final state
    if (sqlite3_open(db_filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    std::cout << "Final state of the employees table:\n";
    execute_sql(db, "SELECT * FROM employees;", true);
    sqlite3_close(db);

    return 0;
}

