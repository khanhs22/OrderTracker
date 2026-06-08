#pragma once
#include <QString>
#include <QDateTime>

enum class CarrierType {
    ShopeeExpress,
    ViettelPost,
    Unknown
};

struct Order {
    QString id;
    QString trackingNumber;
    CarrierType carrier;
    QString status;
    QDateTime lastUpdated;
    QString note;
};