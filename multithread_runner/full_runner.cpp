#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sqlite3.h>
#include "../sqlite3.h"

// SQL script stored in a string for initialization
auto init_sql = R"SQL(
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

// SQL script for tasks that multiple threads will execute
auto task_sql = R"SQL(
BEGIN TRANSACTION;
UPDATE employees SET salary = salary + 1000 WHERE department = 'Engineering';
COMMIT;

SELECT * FROM employees LIMIT 2;
)SQL";

// Function to execute SQL commands
void execute_sql(sqlite3* db, const char* sql) {
    char* err_msg = nullptr;
    if (const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err_msg); rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
}

// Thread task
void thread_task(sqlite3* db) {
    execute_sql(db, task_sql);
}

int main() {
    constexpr int NUM_THREADS = 5;
    sqlite3* db;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // Execute initialization SQL
    execute_sql(db, init_sql);

    // Launch 10 threads to perform the task SQL
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(thread_task, db);
    }

    // Join threads
    for (auto& thread : threads) {
        thread.join();
    }

    sqlite3_close(db);
    return 0;
}
