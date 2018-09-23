#pragma once

#include "base_connector.h"

class EthernetConnector : public BaseConnector {
public:
    EthernetConnector() = default;
    ~EthernetConnector() = default;

    void init(std::shared_ptr<config::Segment> settings) noexcept override {};

    bool connect() override {return false;};
    void disconnect() override {};

    void write(const QByteArray &data) override {};

private:
};