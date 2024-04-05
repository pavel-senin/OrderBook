#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <map>
#include <functional>


//Переменные для расчета времени
auto start = std::chrono::high_resolution_clock::now();
auto end = std::chrono::high_resolution_clock::now();

enum class OrderType { Buy, Sell };

class Order {
private:
    double price;
    int volume;
    OrderType type;

public:
    // Конструктор
    Order(double price, int volume, OrderType type) : price(price), volume(volume), type(type) {}
    Order(){};

    // Геттеры
    double getPrice() const { return price; }
    int getVolume() const { return volume; }
    OrderType getType() const { return type; }

    // Сеттеры
    void setPrice(double newPrice) { price = newPrice; }
    void setVolume(int newVolume) { volume = newVolume; }
    void setType(OrderType newType) { type = newType; }

    Order& operator=(const Order& other) {
        if (this != &other) {
            price = other.price;
            volume = other.volume;
            type = other.type;
        }
        return *this;
    }
    
};

//Генерация случайного заказа
Order generateOrder() {
    Order order;

    // Генерация случайной цены в диапазоне от 50 до 200
    std::srand(std::time(nullptr));
    double randomPrice = (std::rand() % 151) + 50;
    order.setPrice(randomPrice);

    // Генерация случайного объема заказа
    int randomVolume = (std::rand() % 1000) + 1; // Пример: объем от 1 до 1000
    order.setVolume(randomVolume);

    // Генерация случайного типа заказа
    std::srand(std::time(nullptr));
    int randomValue = std::rand() % 2;
    OrderType randomOrderType = (randomValue == 0) ? OrderType::Buy : OrderType::Sell;
    order.setType(randomOrderType);

    return order;
}

OrderType determineOrderType (int newType){
    if (newType == 1) {
        return OrderType::Buy;
    } else if (newType == 2) {
        return OrderType::Sell;
    } else {
        std::cerr << "Invalid type entered\n";
        return OrderType::Buy;
    }
};


//Книга заявок с одним контейнером
class OrderBook {
private:
    std::vector<Order> orders;

public:
    OrderBook() {};

    void addOrder(Order order) {
        orders.push_back(order);
    }

    void modifyOrder(int index, double newPrice, int newVolume, OrderType type) {
        if (index >= 0 && index < orders.size()) {
            orders[index].setPrice(newPrice);
            orders[index].setVolume(newVolume);
            orders[index].setType(type);
        } else {
            std::cerr << "Invalid index for modification\n";
        }
    }

    void deleteOrder(int index) {
        if (index >= 0 && index < orders.size()) {
            orders.erase(orders.begin() + index);
        } else {
            std::cerr << "Invalid index for deletion\n";
        }
    }

    void displayTop10() {
        // Сортировка заказов по цене в порядке убывания для покупок и по возрастанию для продаж
        std::sort(orders.begin(), orders.end(), [](const Order& a, const Order& b) {
            if (a.getType() == OrderType::Buy && b.getType() == OrderType::Buy) {
                return a.getPrice() > b.getPrice(); // Сортировка по убыванию для покупок
            } else if (a.getType() == OrderType::Sell && b.getType() == OrderType::Sell) {
                return a.getPrice() < b.getPrice(); // Сортировка по возрастанию для продаж
            } else {
                return (a.getType() == OrderType::Buy);
            }
        });

        // Вывод 5 лучших на покупку
        std::cout << "Top 5 Buy orders:\n";
        int buyCount = 0;
        for (const auto& order : orders) {
            if (order.getType() == OrderType::Buy && buyCount < 5) {
                std::cout << "Price: " << order.getPrice() << ", Volume: " << order.getVolume();
                std::cout << ", Type: Buy\n";
                buyCount++;
            }
        }

        // Вывод 5 лучших на продажу
        std::cout << "\nTop 5 Sell orders:\n";
        int sellCount = 0;
        for (const auto& order : orders) {
            if (order.getType() == OrderType::Sell && sellCount < 5) {
                std::cout << "Price: " << order.getPrice() << ", Volume: " << order.getVolume();
                std::cout << ", Type: Sell\n";
                sellCount++;
            }
        }
    }

    //Сохранение заказов в файл orders.txt
    void save(const std::string& filename) {
        std::ofstream outFile(filename);
        if (outFile.is_open()) {
            for (const auto& order : orders) {
                outFile << order.getPrice() << " " << order.getVolume() << " ";
                if (order.getType() == OrderType::Buy) {
                    outFile << "Buy\n";
                } else {
                    outFile << "Sell\n";
                }
            }
            outFile.close();
            std::cout << "Orders saved to " << filename << std::endl;
        } else {
            std::cerr << "Unable to open file " << filename << std::endl;
        }
    }

    //Загрузка заказов из orders.txt
    void load(const std::string& filename) {
        std::ifstream inFile(filename);
        if (inFile.is_open()) {
            orders.clear();
            double price;
            int volume;
            std::string type;
            while (inFile >> price >> volume >> type) {
                OrderType orderType = (type == "Buy") ? OrderType::Buy : OrderType::Sell;
                orders.emplace_back(price, volume, orderType);
            }
            inFile.close();
            std::cout << "Orders loaded from " << filename << std::endl;
        } else {
            std::cerr << "Unable to open file " << filename << std::endl;
        }
    }
};


//Дерево заявок с двойным контейнером map
class OrderTree {
private:
    std::map<OrderType, std::vector<Order>> ordersMap;

public:
    OrderTree() {}

    void addOrder(const Order& order) {
        std::vector<Order>& orders = ordersMap[order.getType()];
    
        if (order.getType() == OrderType::Buy) {
            auto it = std::lower_bound(orders.begin(), orders.end(), order,
                [](const Order& a, const Order& b) {
                    return a.getPrice() > b.getPrice();
                });
            orders.insert(it, order);
        } else {
            auto it = std::lower_bound(orders.begin(), orders.end(), order,
                [](const Order& a, const Order& b) {
                    return a.getPrice() < b.getPrice();
                });
            orders.insert(it, order);
        }

    }

    void modifyOrder(int index, double newPrice, int newVolume, OrderType type) {
        std::vector<Order>& orders = ordersMap[type];
        if (index >= 0 && index < orders.size()) {
            orders[index].setPrice(newPrice);
            orders[index].setVolume(newVolume);
        } else {
            std::cerr << "Invalid index for modification\n";
        }
    }

    void deleteOrder(int index, OrderType type) {
        std::vector<Order>& orders = ordersMap[type];
        if (index >= 0 && index < orders.size()) {
            orders.erase(orders.begin() + index);
        } else {
            std::cerr << "Invalid index for deletion\n";
        }
    }

    void displayTop5(OrderType type) {
        std::vector<Order>& orders = ordersMap[type];

        int count = std::min(static_cast<int>(orders.size()), 5);
        for (int i = 0; i < count; ++i) {
            std::cout << "Price: " << orders[i].getPrice() << ", Volume: " << orders[i].getVolume();
            std::cout << ", Type: " << (type == OrderType::Buy ? "Buy" : "Sell") << "\n";
        }
    }

    void save(const std::string& filename) {
        std::ofstream outFile(filename);
        if (outFile.is_open()) {
            for (const auto& pair : ordersMap) {
                OrderType type = pair.first;
                for (const auto& order : pair.second) {
                    outFile << order.getPrice() << " " << order.getVolume() << " ";
                    outFile << (type == OrderType::Buy ? "Buy\n" : "Sell\n");
                }
            }
            outFile.close();
            std::cout << "Orders saved to " << filename << std::endl;
        } else {
            std::cerr << "Unable to open file " << filename << std::endl;
        }
    }

    void load(const std::string& filename) {
        std::ifstream inFile(filename);
        if (inFile.is_open()) {
            ordersMap.clear();
            double price;
            int volume;
            std::string type;
            while (inFile >> price >> volume >> type) {
                OrderType orderType = (type == "Buy") ? OrderType::Buy : OrderType::Sell;
                ordersMap[orderType].emplace_back(price, volume, orderType);
            }
            inFile.close();

            // Сортировка заявок по цене при загрузке
            for (auto& pair : ordersMap) {
                auto& orders = pair.second;
                if (pair.first == OrderType::Buy) {
                    std::sort(orders.begin(), orders.end(), [](const Order& a, const Order& b) {
                        return a.getPrice() > b.getPrice(); // Сортировка по убыванию для покупок
                    });
                } else {
                    std::sort(orders.begin(), orders.end(), [](const Order& a, const Order& b) {
                        return a.getPrice() < b.getPrice(); // Сортировка по возрастанию для продаж
                    });
                }
            }

            std::cout << "Orders loaded from " << filename << std::endl;
        } else {
            std::cerr << "Unable to open file " << filename << std::endl;
        }
    }
};
    



//Function for OrderBook
void checkOrderBook(){
    OrderBook orderBook;
    // Считываем заявки из файла
    orderBook.load("orders.txt");
    int option = 0;
    do {
        // Предложение выбора опций
        std::cout << "Options:\n";
        std::cout << "1. Add order\n";
        std::cout << "2. Modify order\n";
        std::cout << "3. Delete order\n";
        std::cout << "4. Display top 10 orders\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter option: ";
        std::cin >> option;

        if (option == 1) {
            Order order;
            start = std::chrono::high_resolution_clock::now();
            orderBook.addOrder(generateOrder());
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 2) {
            int index, newPrice, newVolume, newType;
            std::cout << "Enter the index of order to modify:\n";
            std::cin >> index;
            std::cout << "Enter the new price:\n";
            std::cin >> newPrice;
            std::cout << "Enter the new volume:\n";
            std::cin >> newVolume;
            std::cout << "Enter the new type (1 - Buy or 2 - Sell):\n";
            std::cin >> newType;
            OrderType type = determineOrderType(newType);
            start = std::chrono::high_resolution_clock::now();
            orderBook.modifyOrder(index, newPrice, newVolume, type);
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 3) {
            int delIndex;
            std::cout << "Enter the index of order to delete:\n";       
            std::cin >> delIndex;
            start = std::chrono::high_resolution_clock::now();
            orderBook.deleteOrder(delIndex);
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 4) {
            start = std::chrono::high_resolution_clock::now();
            orderBook.displayTop10();
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 5) {
            // выход
        } else {
            std::cerr << "Invalid option\n";
        }
    } while (option != 5);

    orderBook.save("orders.txt");
}


//Function for OrderTree
void checkOrderTree(){
    OrderTree orderTree;
    // Считываем заявки из файла
    orderTree.load("orders.txt");
    
    
    int option = 0;
    do {
        // Предложение выбора опций
        std::cout << "Options:\n";
        std::cout << "1. Add order\n";
        std::cout << "2. Modify order\n";
        std::cout << "3. Delete order\n";
        std::cout << "4. Display top 10 orders\n";
        std::cout << "5. Exit\n";
        std::cout << "Enter option: ";
        std::cin >> option;

        if (option == 1) {
            start = std::chrono::high_resolution_clock::now();
            orderTree.addOrder(generateOrder());
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 2) {
            int index, newPrice, newVolume;
            int newType;
            std::cout << "Enter the index of order to modify:\n";
            std::cin >> index;
            std::cout << "Enter the new price:\n";
            std::cin >> newPrice;
            std::cout << "Enter the new volume:\n";
            std::cin >> newVolume;
            std::cout << "Enter the new type (1 - Buy or 2 - Sell):\n";
            std::cin >> newType;
            OrderType type = determineOrderType(newType);
            start = std::chrono::high_resolution_clock::now();
            orderTree.modifyOrder(index, newPrice, newVolume, type);
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 3) {
            int delIndex, delType;
            std::cout << "Enter the index of order to delete:\n";
            std::cin >> delIndex;
            std::cout << "Enter the type (1 - Buy or 2 - Sell):\n";
            std::cin >> delType;
            start = std::chrono::high_resolution_clock::now();
            orderTree.deleteOrder(delIndex, determineOrderType(delType));
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 4) {
            start = std::chrono::high_resolution_clock::now();
            orderTree.displayTop5(OrderType::Buy);
            orderTree.displayTop5(OrderType::Sell);
            end = std::chrono::high_resolution_clock::now();
            std::cout << "Time elapsed: " << (end - start).count() << " nanoseconds" << std::endl;
        } else if (option == 5) {
            // выход
        } else {
            std::cerr << "Invalid option\n";
        }
    } while (option != 5);
    orderTree.save("orders.txt");

}


int main() {
    
    int classOption;
    std::cout << "Type 1 for OrderBook;\nType 2 for OrderTree\n";
    std::cin >> classOption;
    switch (classOption)
    {
    case 1:
        checkOrderBook();
        break;
    case 2:
        checkOrderTree();
        break;
    default:
        break;
    }

    return 0;
}


