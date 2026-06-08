// src/storage/order_repository.cpp
#include "order_repository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

OrderRepository::OrderRepository() {
    // Chuẩn macOS: Lưu database vào thư mục AppData quy định bởi hệ điều hành (Tránh lỗi phân quyền)
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dir.filePath("orders.db"));
}

OrderRepository::~OrderRepository() {
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool OrderRepository::initDatabase() {
    if (!m_db.open()) {
        qDebug() << "Không thể mở kết nối Database:" << m_db.lastError().text();
        return false;
    }

    QSqlQuery query;
    // Tạo bảng orders nếu chưa tồn tại
    QString createTableQuery =
        "CREATE TABLE IF NOT EXISTS orders ("
        "id TEXT PRIMARY KEY, "
        "tracking_number TEXT, "
        "carrier INTEGER, "
        "status TEXT, "
        "last_updated TEXT, "
        "note TEXT"
        ")";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Lỗi tạo bảng:" << query.lastError().text();
        return false;
    }
    return true;
}

bool OrderRepository::addOrder(const Order& order) {
    QSqlQuery query;
    query.prepare("INSERT INTO orders (id, tracking_number, carrier, status, last_updated, note) "
                  "VALUES (:id, :tracking_number, :carrier, :status, :last_updated, :note)");
    query.bindValue(":id", order.id);
    query.bindValue(":tracking_number", order.trackingNumber);
    query.bindValue(":carrier", static_cast<int>(order.carrier));
    query.bindValue(":status", order.status);
    query.bindValue(":last_updated", order.lastUpdated.toString(Qt::ISODate));
    query.bindValue(":note", order.note);

    if (!query.exec()) {
        qDebug() << "Lỗi thêm đơn hàng vào DB:" << query.lastError().text();
        return false;
    }
    return true;
}

QVector<Order> OrderRepository::getAllOrders() {
    QVector<Order> orders;
    QSqlQuery query("SELECT id, tracking_number, carrier, status, last_updated, note FROM orders");

    while (query.next()) {
        Order order;
        order.id = query.value(0).toString();
        order.trackingNumber = query.value(1).toString();
        order.carrier = static_cast<CarrierType>(query.value(2).toInt());
        order.status = query.value(3).toString();
        order.lastUpdated = QDateTime::fromString(query.value(4).toString(), Qt::ISODate);
        order.note = query.value(5).toString();
        orders.append(order);
    }
    return orders;
}

bool OrderRepository::updateOrderStatus(const QString& trackingNumber, const QString& status, const QDateTime& lastUpdated) {
    QSqlQuery query;
    query.prepare("UPDATE orders SET status = :status, last_updated = :last_updated WHERE tracking_number = :tracking_number");
    query.bindValue(":status", status);
    query.bindValue(":last_updated", lastUpdated.toString(Qt::ISODate));
    query.bindValue(":tracking_number", trackingNumber);

    if (!query.exec()) {
        qDebug() << "Lỗi cập nhật trạng thái đơn hàng:" << query.lastError().text();
        return false;
    }
    return true;
}

bool OrderRepository::deleteOrder(const QString &trackingNumber)
{
    QSqlQuery query;
    query.prepare("DELETE FROM orders WHERE tracking_number = :tracking_number");
    query.bindValue(":tracking_number", trackingNumber);

    if (!query.exec()) {
        qDebug() << "Lỗi xóa đơn hàng khỏi DB:" << query.lastError().text();
        return false;
    }
    return true;
}