// src/storage/order_repository.h
#pragma once
#include <QSqlDatabase>
#include <QVector>
#include "models/order.h"

class OrderRepository {
public:
    OrderRepository();
    ~OrderRepository();

    bool initDatabase();
    bool addOrder(const Order& order);
    QVector<Order> getAllOrders();
    bool updateOrderStatus(const QString& trackingNumber, const QString& status, const QDateTime& lastUpdated);
    bool deleteOrder(const QString& trackingNumber);

private:
    QSqlDatabase m_db;
};