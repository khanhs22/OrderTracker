#pragma once
#include "carrier_service.h"
#include "shopee_service.h"
#include "../models/order.h"
#include <memory>

class CarrierFactory {
public:
    static std::unique_ptr<CarrierService> createCarrier(CarrierType type) {
        switch (type) {
        case CarrierType::ShopeeExpress:
            return std::make_unique<ShopeeExpressService>();
        // case CarrierType::ViettelPost: return std::make_unique<ViettelPostService>();
        default:
            return nullptr;
        }
    }
};