-- This will create a new database in the specified directory
-- It assumes the directory exists; SQLite will not create the directory
.open ./tests/mydatabase.db

-- Create a simple table named 'employees'
DROP TABLE IF EXISTS employees;
CREATE TABLE IF NOT EXISTS employees (
                                         id INTEGER PRIMARY KEY,
                                         name TEXT NOT NULL,
                                         department TEXT NOT NULL,
                                         salary INTEGER NOT NULL
);

-- PRAGMA vdbe_trace = ON;

-- Start a transaction
BEGIN TRANSACTION;

-- Insert some sample data into the table
INSERT INTO employees (name, department, salary) VALUES ('Alice', 'HR', 50000);
INSERT INTO employees (name, department, salary) VALUES ('Bob', 'Engineering', 75000);
INSERT INTO employees (name, department, salary) VALUES ('Charlie', 'Marketing', 60000);

-- Commit the transaction
COMMIT;

-- Start another transaction for updates
BEGIN TRANSACTION;

-- Update an employee's salary
UPDATE employees SET salary = salary + 5000 WHERE name = 'Alice';

-- Insert another employee
INSERT INTO employees (name, department, salary) VALUES ('Dana', 'HR', 55000);

-- Roll back the transaction
ROLLBACK;

-- Perform a simple query to retrieve all entries after rollback
SELECT * FROM employees;

-- Start another transaction
BEGIN TRANSACTION;

-- Perform updates
UPDATE employees SET salary = salary + 1000 WHERE department = 'Engineering';

-- Commit the final changes
COMMIT;

-- Query to see the final state of the table
SELECT * FROM employees LIMIT 2;