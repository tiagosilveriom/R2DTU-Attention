#pragma once

#include <core/Types.h>
#include <memory>
#include <queue>

struct NetworkPacket;

/**
 * The network modules provides the P2P interface based upon ENet
 * Currently only exactly two nodes are allowed, one of which must be the host.
 * The plan is a rewrite which allows for an arbitrary number of homogeneous nodes
 */
namespace Network {

    void initialize();

    bool is_initialized();

    void start_host(const char* ip, uint16_t port);

    void connect(const char* ip, uint16_t port);

    void tick();

    std::queue<std::shared_ptr<MessageHeader>>& access_received();

    NetworkPacket* pack_message(std::shared_ptr<MessageHeader> header);

    void transmit(NetworkPacket* msg, uint8_t channel_id);
    void enqueue(NetworkPacket* msg, uint8_t channel_id);

    void shutdown();
}
