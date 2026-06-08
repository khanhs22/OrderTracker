// src/services/shopee_service.h
#pragma once
#include "carrier_service.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

class ShopeeExpressService : public CarrierService {
public:
    void fetchStatus(QNetworkAccessManager* manager, const QString& trackingNumber,
                     std::function<void(QString, bool)> callback) override {

        // 1. Tạo URL động bằng cách truyền mã vận đơn người dùng nhập vào query 'spx_tn'
        QString urlString = QString("https://spx.vn/shipment/order/open/order/get_order_info?spx_tn=%1&language_code=vi")
                                .arg(trackingNumber);
        QUrl url(urlString);
        QNetworkRequest request(url);

        // CẢI TIẾN 1: Bật tính năng hỗ trợ HTTP/2 chuẩn mã hóa tương thích với SPX
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);

        // CẢI TIẾN 2: Bổ sung bộ Headers giả lập trình duyệt ở mức độ sâu hơn
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/148.0.0.0 Safari/537.36");
        request.setRawHeader("Accept", "application/json, text/plain, */*");
        request.setRawHeader("Accept-Language", "vi-VN,vi;q=0.9,en-US;q=0.8");
        request.setRawHeader("Connection", "keep-alive");
        request.setRawHeader("Referer", "https://spx.vn/");

        qDebug() << "Đang gửi request HTTP/2 thực tế tới SPX cho đơn hàng:" << trackingNumber;

        // 3. Thực hiện lệnh GET bất đồng bộ
        QNetworkReply* reply = manager->get(request);

        // 4. Xử lý dữ liệu khi có phản hồi từ máy chủ SPX
        QObject::connect(reply, &QNetworkReply::finished, [reply, callback]() {
            // reply->deleteLater(); // Giải phóng bộ nhớ của Reply giải quyết vấn đề Memory Leak

            // Kiểm tra lỗi kết nối mạng (mất mạng, timeout, sập server...)
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "Lỗi kết nối mạng thật:" << reply->errorString();
                callback(QString("Lỗi mạng (%1)").arg(reply->errorString()), false);
                return;
            }

            // Đọc dữ liệu JSON thô từ Server trả về
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

            if (jsonDoc.isNull() || !jsonDoc.isObject()) {
                callback("Dữ liệu không đúng định dạng JSON", false);
                return;
            }

            QJsonObject rootObj = jsonDoc.object();
            bool parseSuccess = false;

            if (rootObj.contains("data") && rootObj["data"].isObject()) {
                QJsonObject dataObj = rootObj["data"].toObject();
                if (dataObj.contains("sls_tracking_info") && dataObj["sls_tracking_info"].isObject()) {
                    QJsonObject slsObj = dataObj["sls_tracking_info"].toObject();
                    if (slsObj.contains("records") && slsObj["records"].isArray()) {
                        QJsonArray recordsArray = slsObj["records"].toArray();
                        if (!recordsArray.isEmpty()) {
                            QJsonObject latestRecord = recordsArray.at(0).toObject();
                            QString statusText = latestRecord["description"].toString();
                            if (statusText.isEmpty()) {
                                statusText = latestRecord["buyer_description"].toString();
                            }

                            if (!statusText.isEmpty()) {
                                callback(statusText, true);
                                parseSuccess = true;
                            }
                        }
                    }
                }
            }

            if (!parseSuccess) {
                callback("Không tìm thấy thông tin vận đơn hoặc mã sai", false);
            }

            //  ĐẶT ĐỂ Ở ĐÂY (DÒNG CUỐI CÙNG CỦA HÀM)
            reply->deleteLater();
        });
    }
};