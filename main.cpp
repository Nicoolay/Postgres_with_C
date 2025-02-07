#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <stdexcept>

class CustomerDB {
private:
    const std::string connection_info = "dbname=customers_db user=postgres password=1234 host=localhost port=5432";;

public:
    void createTables() {
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
    }

    void clearTables() {
        pqxx::connection conn(connection_info);
        pqxx::work txn(conn);
        txn.exec(R"SQL(
            DELETE FROM phones;
            DELETE FROM customers;
            ALTER SEQUENCE customers_id_seq RESTART WITH 1;
            ALTER SEQUENCE phones_id_seq RESTART WITH 1;
        )SQL");
        txn.commit();
    }

    void addCustomer(const std::string& first_name, const std::string& last_name, const std::string& email) {
        pqxx::connection conn(connection_info);
        pqxx::work txn(conn);
        txn.exec_params(R"SQL(
            INSERT INTO customers (first_name, last_name, email) VALUES ($1, $2, $3)
        )SQL", first_name, last_name, email);
        txn.commit();
    }

    void addPhone(int customer_id, const std::string& phone_number) {
        pqxx::connection conn(connection_info);
        pqxx::work txn(conn);
        txn.exec_params(R"SQL(
            INSERT INTO phones (customer_id, phone_number) VALUES ($1, $2)
        )SQL", customer_id, phone_number);
        txn.commit();
    }

    void updateCustomer(int customer_id, const std::string& first_name, const std::string& last_name, const std::string& email) {
        pqxx::connection conn(connection_info);
        pqxx::work txn(conn);
        txn.exec_params(R"SQL(
            UPDATE customers SET first_name = $1, last_name = $2, email = $3 WHERE id = $4
        )SQL", first_name, last_name, email, customer_id);
        txn.commit();
    }

    void deletePhone(int phone_id) {
        pqxx::connection conn(connection_info);
        pqxx::work txn(conn);
        txn.exec_params(R"SQL(
            DELETE FROM phones WHERE id = $1
        )SQL", phone_id);
        txn.commit();
    }

    void deleteCustomer(int customer_id) {
        pqxx::connection conn(connection_info);
        pqxx::work txn(conn);
        txn.exec_params(R"SQL(
            DELETE FROM customers WHERE id = $1
        )SQL", customer_id);
        txn.commit();
    }

    void findCustomer(const std::string& search_term) {
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
};

int main() {
    try {
        CustomerDB db;

        std::cout << "Clearing existing data and creating tables..." << std::endl;
        db.clearTables();
        db.createTables();

        std::cout << "Adding customers..." << std::endl;
        db.addCustomer("Ivan", "Ivanov", "Ivan.Ivanov@google.com");
        db.addCustomer("Petr", "Petrov", "Petr.Petrov@ya.ru");

        std::cout << "Adding phone numbers..." << std::endl;
        db.addPhone(1, "+1234567890");
        db.addPhone(1, "+0987654321");

        std::cout << "Updating customer..." << std::endl;
        db.updateCustomer(1, "Ivan", "Ivanov", "Ivan.Ivanov@ya.ru");

        std::cout << "Finding customer by email..." << std::endl;
        db.findCustomer("Ivan.Ivanov@ya.ru");

        std::cout << "Deleting phone..." << std::endl;
        db.deletePhone(1);

        std::cout << "Deleting customer..." << std::endl;
        db.deleteCustomer(1);

    }
    catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Query: " << e.query() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}
