// Sửa đổi trong src/ui/main_window.h
#pragma once
#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include "storage/order_repository.h"
#include <QNetworkAccessManager>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onAddOrderClicked();
    void onQueryAllStatus();
    void onDeleteOrderClicked();

private:
    void setupUI();
    Q_INVOKABLE void loadOrdersFromDatabase();

    QLineEdit *m_trackingInput;
    QComboBox *m_carrierSelect;
    QPushButton *m_addButton;
    QPushButton *m_refreshButton;
    QPushButton *m_deleteButton;
    QListWidget *m_historyList;

    OrderRepository m_repo;
    QNetworkAccessManager *m_networkManager;
};