// Cập nhật lại trong src/ui/main_window.cpp
#include "main_window.h"
#include "services/carrier_factory.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    setWindowTitle("Logistics Order Tracker v1.1 (SQLite)");
    resize(650, 450);

    m_networkManager = new QNetworkAccessManager(this);

    if (m_repo.initDatabase()) {
        loadOrdersFromDatabase();
    } else {
        QMessageBox::critical(this, "Lỗi hệ thống", "Không thể khởi tạo cơ sở dữ liệu!");
    }
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *formLayout = new QHBoxLayout();
    m_trackingInput = new QLineEdit();
    m_trackingInput->setPlaceholderText("Nhập mã vận đơn...");

    m_carrierSelect = new QComboBox();
    m_carrierSelect->addItem("Shopee Express", static_cast<int>(CarrierType::ShopeeExpress));
    m_carrierSelect->addItem("Viettel Post", static_cast<int>(CarrierType::ViettelPost));

    m_addButton = new QPushButton("Thêm đơn hàng");

    formLayout->addWidget(new QLabel("Mã:"));
    formLayout->addWidget(m_trackingInput);
    formLayout->addWidget(m_carrierSelect);
    formLayout->addWidget(m_addButton);

    m_historyList = new QListWidget();

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_refreshButton = new QPushButton("Cập nhật tất cả trạng thái từ API");
    m_deleteButton = new QPushButton("Xóa đơn đang chọn");
    m_deleteButton->setStyleSheet("color: red;");

    actionLayout->addWidget(m_refreshButton);
    actionLayout->addWidget(m_deleteButton);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(new QLabel("Lịch sử theo dõi đơn hàng:"));
    mainLayout->addWidget(m_historyList);
    mainLayout->addLayout(actionLayout);

    setCentralWidget(centralWidget);

    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddOrderClicked);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::onQueryAllStatus);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteOrderClicked);
}

void MainWindow::loadOrdersFromDatabase() {
    m_historyList->clear();
    QVector<Order> savedOrders = m_repo.getAllOrders();

    for (const auto &order : savedOrders) {
        QString carrierName = (order.carrier == CarrierType::ShopeeExpress) ? "Shopee" : "Viettel";
        QString itemText = QString("[%1] %2 - Trạng thái: %3 (Cập nhật lúc: %4)")
                               .arg(carrierName, order.trackingNumber, order.status, order.lastUpdated.toString("dd/MM hh:mm"));

        QListWidgetItem *item = new QListWidgetItem(itemText, m_historyList);
        item->setData(Qt::UserRole, order.trackingNumber);
    }
}

void MainWindow::onAddOrderClicked() {
    QString code = m_trackingInput->text().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập mã vận đơn!");
        return;
    }

    CarrierType carrier = static_cast<CarrierType>(m_carrierSelect->currentData().toInt());

    // Tạo object mới
    Order newOrder{code, code, carrier, "Chưa kiểm tra", QDateTime::currentDateTime(), ""};

    // Lưu vào database
    if (m_repo.addOrder(newOrder)) {
        loadOrdersFromDatabase(); // Refresh lại danh sách hiển thị
        m_trackingInput->clear();
    } else {
        QMessageBox::warning(this, "Lỗi DB", "Mã đơn hàng này đã tồn tại hoặc có lỗi xảy ra!");
    }
}

void MainWindow::onQueryAllStatus() {
    QVector<Order> currentOrders = m_repo.getAllOrders();

    for (const auto &order : currentOrders) {
        auto service = CarrierFactory::createCarrier(order.carrier);
        if (service) {
            // TRUYỀN THÊM m_networkManager VÀO ĐÂY:
            service->fetchStatus(m_networkManager, order.trackingNumber, [this, order](QString status, bool success) {
                if (success) {
                    QDateTime now = QDateTime::currentDateTime();
                    m_repo.updateOrderStatus(order.trackingNumber, status, now);
                    QMetaObject::invokeMethod(this, "loadOrdersFromDatabase", Qt::QueuedConnection);
                } else {
                    // Nếu lỗi API, hiển thị tạm trạng thái lỗi lên danh sách mà không ghi đè DB cũ
                    qDebug() << "Không thể cập nhật mã đơn:" << order.trackingNumber << "-" << status;
                }
            });
        }
    }
}

void MainWindow::onDeleteOrderClicked() {
    // 1. Lấy dòng hiện tại người dùng đang bấm chọn trên giao diện
    QListWidgetItem *currentItem = m_historyList->currentItem();

    if (!currentItem) {
        QMessageBox::warning(this, "Thông báo", "Vui lòng chọn một đơn hàng trong danh sách để xóa!");
        return;
    }

    // 2. Bốc dữ liệu mã vận đơn ẩn ngầm (UserRole) ra ngoài
    QString trackingNumber = currentItem->data(Qt::UserRole).toString();

    // 3. Hỏi xác nhận người dùng trước khi xóa (Best practice UX)
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Xác nhận xóa",
                                  QString("Bạn có chắc chắn muốn xóa và dừng theo dõi đơn hàng %1 không?").arg(trackingNumber),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 4. Xóa trong SQLite Database
        if (m_repo.deleteOrder(trackingNumber)) {
            // 5. Tải lại danh sách lên giao diện
            loadOrdersFromDatabase();
        } else {
            QMessageBox::critical(this, "Lỗi", "Không thể xóa đơn hàng khỏi Cơ sở dữ liệu!");
        }
    }
}