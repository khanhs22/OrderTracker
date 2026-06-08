#pragma once
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <functional>

// Interface dùng chung cho tất cả các nhà vận chuyển
class CarrierService {
public:
    virtual ~CarrierService() = default;

    // Hàm thuần ảo để gửi request kiểm tra trạng thái
    virtual void fetchStatus(QNetworkAccessManager* manager,
                             const QString& trackingNumber,
                             std::function<void(QString status, bool success)> callback) = 0;
};