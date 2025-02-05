#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <Windows.h>

class CustomerDB {
private:
    const std::string connection_info = "dbname=customers_db user=postgres password=1234 host=localhost port=5432";
 
public:
    void createTables() {
        try {
            pqxx::connection conn(connection_info);
            pqxx::work txn(conn);

            txn.exec(R"SQL(
                CREATE TABLE IF NOT EXISTS customers (
                    id SERIAL PRIMARY KEY,
                    first_name VARCHAR(50) NOT NULL,
                    last_name VARCHAR(50) NOT NULL,
                    email VARCHAR(100) UNIQUE NOT NULL
                );

                CREATE TABLE IF NOT EXISTS phones (
                    id SERIAL PRIMARY KEY,
                    customer_id INTEGER NOT NULL REFERENCES customers(id) ON DELETE CASCADE,
                    phone_number VARCHAR(20) NOT NULL
                );
            )SQL");

            txn.commit();
            std::cout << "Tables created successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error creating tables: " << e.what() << std::endl;
        }
    }

    void addCustomer(const std::string& first_name, const std::string& last_name, const std::string& email) {
        try {
            pqxx::connection conn(connection_info);
            pqxx::work txn(conn);

            txn.exec_params(R"SQL(
                INSERT INTO customers (first_name, last_name, email) VALUES ($1, $2, $3)
            )SQL", first_name, last_name, email);

            txn.commit();
            std::cout << "Customer added successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding customer: " << e.what() << std::endl;
        }
    }

    void addPhone(int customer_id, const std::string& phone_number) {
        try {
            pqxx::connection conn(connection_info);
            pqxx::work txn(conn);

            txn.exec_params(R"SQL(
                INSERT INTO phones (customer_id, phone_number) VALUES ($1, $2)
            )SQL", customer_id, phone_number);

            txn.commit();
            std::cout << "Phone added successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding phone: " << e.what() << std::endl;
        }
    }

    void updateCustomer(int customer_id, const std::string& first_name, const std::string& last_name, const std::string& email) {
        try {
            pqxx::connection conn(connection_info);
            pqxx::work txn(conn);

            txn.exec_params(R"SQL(
                UPDATE customers SET first_name = $1, last_name = $2, email = $3 WHERE id = $4
            )SQL", first_name, last_name, email, customer_id);

            txn.commit();
            std::cout << "Customer updated successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error updating customer: " << e.what() << std::endl;
        }
    }

    void deletePhone(int phone_id) {
        try {
            pqxx::connection conn(connection_info);
            pqxx::work txn(conn);

            txn.exec_params(R"SQL(
                DELETE FROM phones WHERE id = $1
            )SQL", phone_id);

            txn.commit();
            std::cout << "Phone deleted successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error deleting phone: " << e.what() << std::endl;
        }
    }

    void deleteCustomer(int customer_id) {
        try {
            pqxx::connection conn(connection_info);
            pqxx::work txn(conn);

            txn.exec_params(R"SQL(
                DELETE FROM customers WHERE id = $1
            )SQL", customer_id);

            txn.commit();
            std::cout << "Customer deleted successfully." << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error deleting customer: " << e.what() << std::endl;
        }
    }

    void findCustomer(const std::string& search_term) {
        try {
            pqxx::connection conn(connection_info);
            pqxx::work txn(conn);

            pqxx::result result = txn.exec_params(R"SQL(
                SELECT c.id, c.first_name, c.last_name, c.email, p.phone_number
                FROM customers c
                LEFT JOIN phones p ON c.id = p.customer_id
                WHERE c.first_name = $1 OR c.last_name = $1 OR c.email = $1 OR p.phone_number = $1
            )SQL", search_term);

            for (const auto& row : result) {
                std::cout << "ID: " << row[0].as<int>()
                    << ", Name: " << row[1].c_str() << " " << row[2].c_str()
                    << ", Email: " << row[3].c_str()
                    << ", Phone: " << row[4].c_str() << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error finding customer: " << e.what() << std::endl;
        }
    }
};

int main() {
        
    CustomerDB db;

    db.createTables();
    db.addCustomer("Ivan", "Ivanov", "john.jonovich@gmail.ru");
    db.addPhone(1, "+1234567890");
    db.updateCustomer(1, "Ivan", "Ivanov", "john.jonovich@yandex.ru");
    db.findCustomer("john.jonovich@yandex.ru");
    db.deletePhone(1);
    db.deleteCustomer(1);

    return 0;
}
