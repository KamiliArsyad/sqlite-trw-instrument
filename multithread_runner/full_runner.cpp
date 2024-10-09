#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <sqlite3.h>
#include <cstdio> // For std::remove
#include <chrono>
#include <mutex>

// Constants
constexpr int NUM_THREADS = 10;
const std::string DB_FILENAME = "test.db";

// Utility function to execute SQL commands with optional retry logic
bool execute_sql(sqlite3* db, const std::string& sql, const bool use_callback = false, int retries = 1,
                 const int delay_ms = 100) {
    char* err_msg = nullptr;
    int rc;

    while (retries-- > 0) {
        if (use_callback) {
            rc = sqlite3_exec(db, sql.c_str(), [](void*, const int argc, char** argv, char** colNames) {
                for (int i = 0; i < argc; ++i) {
                    std::cout << colNames[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
                }
                std::cout << "--------------------------\n";
                return 0;
            }, nullptr, &err_msg);
        } else {
            rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err_msg);
        }

        if (rc == SQLITE_OK)
        {
            return true;
        }
        if (rc == SQLITE_BUSY)
        {
            // Database is locked, wait and retry
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            continue;
        }
        // Some other error occurred
        std::cerr << "SQL error: " << err_msg << "\n";
        sqlite3_free(err_msg);
        return false;
    }

    if (rc == SQLITE_BUSY) {
        std::cerr << "SQL error: Database is locked after multiple retries.\n";
    }
    return false;
}

// Initialize the database and set it to WAL mode
void initialize_database() {
    // Remove existing database file
    std::remove(DB_FILENAME.c_str());

    sqlite3* db;
    if (sqlite3_open(DB_FILENAME.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    // Enable WAL mode
    if (!execute_sql(db, "PRAGMA journal_mode = WAL;")) {
        std::cerr << "Failed to enable WAL mode.\n";
        sqlite3_close(db);
        return;
    }

    // Initialize the database schema and data
    const char* init_sql = R"SQL(
    CREATE TABLE IF NOT EXISTS employees (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        department TEXT NOT NULL,
        salary INTEGER NOT NULL
    );
    INSERT INTO employees (name, department, salary) VALUES ('Alice', 'HR', 50000);
    INSERT INTO employees (name, department, salary) VALUES ('Bob', 'Engineering', 75000);
    INSERT INTO employees (name, department, salary) VALUES ('Charlie', 'Marketing', 60000);
    )SQL";

    if (!execute_sql(db, init_sql)) {
        std::cerr << "Failed to initialize the database.\n";
    }

    sqlite3_close(db);
}

// Transaction types
enum class TransactionType {
    READ_ONLY,
    READ_WRITE
};

// Base class for transactions
class Transaction {
public:
    explicit Transaction(sqlite3* db) : db_(db) {}
    virtual ~Transaction() = default;
    virtual void execute() = 0;

protected:
    sqlite3* db_;
};

// Read-only transaction to test for anomalies
class ReadOnlyTransaction final : public Transaction {
public:
    explicit ReadOnlyTransaction(sqlite3* db) : Transaction(db) {}

    void execute() override {
        // Start a read-only transaction
        if (!execute_sql(db_, "BEGIN TRANSACTION;")) return;

        // Read data multiple times to test for non-repeatable reads
        const std::string select_sql = "SELECT salary FROM employees WHERE name = 'Bob';";
        int initial_salary = 0;
        int final_salary = 0;

        // First read
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db_, select_sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                initial_salary = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        // Simulate some delay
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Second read
        if (sqlite3_prepare_v2(db_, select_sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                final_salary = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        // Commit transaction
        if (!execute_sql(db_, "COMMIT;")) return;

        // Check for non-repeatable read
        if (initial_salary != final_salary) {
            std::cout << "Non-repeatable read detected: Initial salary = " << initial_salary
                      << ", Final salary = " << final_salary << "\n";
        }
    }
};

// Read-write transaction
class ReadWriteTransaction final : public Transaction {
public:
    explicit ReadWriteTransaction(sqlite3* db) : Transaction(db) {}

    void execute() override {
        // Start a read-write transaction
        if (!execute_sql(db_, "BEGIN TRANSACTION;")) return;

        // Update salary
        if (std::string update_sql = "UPDATE employees SET salary = salary + 5000 WHERE name = 'Bob';";
            !execute_sql(db_, update_sql)) {
            execute_sql(db_, "ROLLBACK;");
            return;
        }

        // Simulate some delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Commit transaction
        if (!execute_sql(db_, "COMMIT;")) return;
    }
};

// Thread function
void thread_function(TransactionType type, const int thread_id) {
    sqlite3* db;
    if (sqlite3_open(DB_FILENAME.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Thread can't open database: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    // Set busy timeout
    sqlite3_busy_timeout(db, 5000);
    setThreadId(thread_id);

    // Execute the transaction
    if (type == TransactionType::READ_ONLY) {
        ReadOnlyTransaction read_only_tx(db);
        read_only_tx.execute();
    } else {
        ReadWriteTransaction read_write_tx(db);
        read_write_tx.execute();
    }

    sqlite3_close(db);
}

int main() {
    initialize_database();

    // Create threads with different transaction types
    std::vector<std::thread> threads;
    int id_maker = 0;

    // Half of the threads will perform read-only transactions
    for (int i = 0; i < NUM_THREADS / 2; ++i) {
        threads.emplace_back(thread_function, TransactionType::READ_ONLY, id_maker++);
    }

    // The other half will perform read-write transactions
    for (int i = 0; i < NUM_THREADS / 2; ++i) {
        threads.emplace_back(thread_function, TransactionType::READ_WRITE, id_maker++);
    }

    // If NUM_THREADS is odd, add one more read-write transaction
    if constexpr (NUM_THREADS % 2 != 0) {
        threads.emplace_back(thread_function, TransactionType::READ_WRITE, id_maker++);
    }

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    // Open the database to display final state
    sqlite3* db;
    if (sqlite3_open(DB_FILENAME.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    std::cout << "Final state of the employees table:\n";
    execute_sql(db, "SELECT * FROM employees;", true);
    sqlite3_close(db);

    return 0;
}
