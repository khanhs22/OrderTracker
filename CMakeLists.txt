cmake_minimum_required(VERSION 3.16)
project(OrderTracker VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tự động tìm kiếm các thành phần Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Sql)
qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/models/order.h
    src/services/carrier_service.h
    src/services/carrier_factory.h
    src/services/shopee_service.h
    src/storage/order_repository.h
    src/storage/order_repository.cpp
    src/ui/main_window.h
    src/ui/main_window.cpp
)

# CẢI TIẾN: Thêm WIN32 và MACOSX_BUNDLE để định hình file xuất ra theo OS
# - Trên Windows: Giúp ẩn cửa sổ console CMD khi mở app.
# - Trên macOS: Tạo ra file .app thay vì file thực thi Unix thô.
qt_add_executable(OrderTracker WIN32 MACOSX_BUNDLE ${PROJECT_SOURCES})

target_include_directories(OrderTracker PRIVATE src)

target_link_libraries(OrderTracker PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Sql
)