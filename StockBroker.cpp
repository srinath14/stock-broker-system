#include <iostream>
#include <string>
#include <unordered_map>
#include <ctime>

using namespace std;

typedef enum
{
    buy = 0,
    sell
} txnType;

typedef enum
{
    Market = 0,
    Limit
} OrderType;

typedef enum
{
    BSE = 0,
    NSE
} StockExchange;

class Stock
{
public:
    int id;
    int price;
    string name;
};

class OrderMetaData
{
public:
    Stock stockinfo;
    OrderType type;
    StockExchange exchangeInfo;
    txnType txn;
};

class Order
{
    Stock info;
    StockExchange exchangeInfo;
    int orderId;
    OrderType type;
    time_t timestamp;
    size_t checksum;
    txnType txn;

public:
    Order(OrderMetaData *pData)
        : info(pData->stockinfo),
          exchangeInfo(pData->exchangeInfo),
          orderId(pData->type),
          type(pData->type),
          txn(pData->txn)
    {
        timestamp = time(nullptr);
        checksum = calculateChecksum();
    }

    virtual ~Order() {}

    int getOrderID() const { return orderId; }
    OrderType getType() const { return type; }
    time_t getTimestamp() const { return timestamp; }
    size_t getChecksum() const { return checksum; }

    void display() const
    {
        cout << "Order ID: " << orderId << ", Stock: " << info.name
             << ", Price: " << info.price << ", Exchange: " << exchangeInfo
             << ", Type: " << (type == Market ? "Market" : "Limit")
             << ", Txn: " << (txn == buy ? "Buy" : "Sell")
             << ", Time: " << ctime(&timestamp)
             << "Checksum: " << checksum << endl;
    }

private:
    size_t calculateChecksum() const
    {
        return hash<string>{}(info.name) ^ info.id ^ info.price ^ type;
    }
};

class MarketOrder : public Order
{
public:
    MarketOrder(OrderMetaData *pData) : Order(pData) {}
};

class LimitOrder : public Order
{
public:
    LimitOrder(OrderMetaData *pData) : Order(pData) {}
};

class OrderFactory
{
public:
    virtual Order *createOrder(OrderMetaData *pData) = 0;
    virtual ~OrderFactory() {}
};

class MarketOrderFactory : public OrderFactory
{
public:
    Order *createOrder(OrderMetaData *pData) override
    {
        return new MarketOrder(pData);
    }
};

class LimitOrderFactory : public OrderFactory
{
public:
    Order *createOrder(OrderMetaData *pData) override
    {
        return new LimitOrder(pData);
    }
};

class OrderProcessor
{
    unordered_map<OrderType, OrderFactory *> factoryMap;

public:
    void registerFactory(OrderType type, OrderFactory *factory)
    {
        factoryMap[type] = factory;
    }

    Order *placeOrder(OrderMetaData *pData)
    {
        if (factoryMap.find(pData->type) != factoryMap.end())
        {
            return factoryMap[pData->type]->createOrder(pData);
        }
        return nullptr;
    }
};

class OrderManager
{
    unordered_map<int, Order *> orderMgr;

public:
    void addOrder(Order *order)
    {
        orderMgr[order->getOrderID()] = order;
    }

    void cancelOrder(int orderId)
    {
        auto it = orderMgr.find(orderId);
        if (it != orderMgr.end())
        {
            delete it->second;
            orderMgr.erase(it);
            cout << "Order ID " << orderId << " canceled.\n";
        }
        else
        {
            cout << "Order ID " << orderId << " not found.\n";
        }
    }

    void printDashboard()
    {
        cout << "\n📊 Order Dashboard:\n";
        for (const auto &pair : orderMgr)
        {
            pair.second->display();
        }
    }

    ~OrderManager()
    {
        for (auto &pair : orderMgr)
        {
            delete pair.second;
        }
    }
};

class StockBrokerSystem
{
    OrderProcessor processor;
    OrderManager manager;

public:
    void registerOrderFactory(OrderType type, OrderFactory *factory)
    {
        processor.registerFactory(type, factory);
    }

    void placeOrder(OrderMetaData *meta)
    {
        Order *order = processor.placeOrder(meta);
        if (order)
        {
            manager.addOrder(order);
            cout << "Order placed successfully.\n";
        }
    }

    void cancelOrder(int id)
    {
        manager.cancelOrder(id);
    }

    void showDashboard()
    {
        manager.printDashboard();
    }
};

int main()
{
    StockBrokerSystem system;

    // Register factories
    system.registerOrderFactory(Market, new MarketOrderFactory());
    system.registerOrderFactory(Limit, new LimitOrderFactory());

    // Example Stock and Metadata
    Stock stock1 = {101, 2500, "Infosys"};
    OrderMetaData *meta1 = new OrderMetaData{stock1, Market, NSE, buy};

    Stock stock2 = {102, 1850, "Reliance"};
    OrderMetaData *meta2 = new OrderMetaData{stock2, Limit, BSE, sell};

    // Place orders
    system.placeOrder(meta1);
    system.placeOrder(meta2);

    // Dashboard
    system.showDashboard();

    // Cancel one order
    system.cancelOrder(Market); // using orderId = type here for simplicity

    // Show dashboard after cancellation
    system.showDashboard();

    delete meta1;
    delete meta2;

    return 0;
}
