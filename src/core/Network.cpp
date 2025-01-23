#include <core/Network.h>

#include <stdio.h>
#include <string.h>
#include <vector>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <enet/enet.h>

#include <common/Timer.h>
#include <common/Message.h>

namespace Network {

    struct Context {
        ENetHost* host;
        ENetPeer* peer;
        std::unordered_map<int, void(*)(void*)> callbacks;
        std::queue<std::pair<NetworkPacket*, uint8_t>> enqueued_msgs;
        std::mutex queue_mutex;

        bool is_initialized = false;
        std::queue<std::shared_ptr<MessageHeader>> receive_queue;
    };

    Context ctx = {};

    void initialize() {
        if (enet_initialize() != 0) {
            fprintf(stderr, "An error occurred while initializing ENet.\n");
            exit(-1);
        }
        ctx.is_initialized = true;
        printf("Network initialized\n");
    }

    bool is_initialized() {
        return ctx.is_initialized;
    }

    void start_host(const char* ip, uint16_t port) {

        ENetAddress address;
        enet_address_set_host(&address, ip);
        address.port = port;
        ctx.host = enet_host_create(&address,
                                    1,      // allow exactly one client to connect,
                                    3,      // allow up to 3 channels to be used, TC, Reliable TM and Unreliable TM
                                    0, 0);   // No assumptions yet on incoming/outgoing bandwidth */,
        if (ctx.host == NULL) {
            fprintf(stderr, "An error occurred while trying to create an ENet host.\n");
            exit(-1);
        }
        char listen_ip[255];
        enet_address_get_host_ip(&address, &listen_ip[0], 255);
        printf("Listening on %s:%u\n", listen_ip, port);
    }

    void connect(const char* ip, uint16_t port) {

        ctx.host = enet_host_create(nullptr,
                                    1,      // allow only a single outgoing connection,
                                    3,      // allow up to 3 channels to be used, TC, Reliable TM and Unreliable TM
                                    0, 0);   // No assumptions yet on incoming/outgoing bandwidth */,
        if (ctx.host == nullptr) {
            fprintf(stderr, "An error occurred while trying to create an ENet host.\n");
            exit(-1);
        }

        ENetAddress address;
        enet_address_set_host(&address, ip);
        address.port = port;

        char hostname[255];
        enet_address_get_host_ip(&address, hostname, 255);
        printf("Connecting to %s:%u\n", hostname, port);
        ctx.peer = enet_host_connect(ctx.host, &address, 3, 0);


        if (ctx.peer == nullptr) {
            fprintf (stderr,
                     "Failed to establish peer for ENet connection.\n");
            exit(-1);
        }
        ENetEvent event;
        /* Wait up to 2 seconds for the connection attempt to succeed, and then retry */
        while (true) {
            if (enet_host_service (ctx.host, &event, 3000) > 0 &&
                event.type == ENET_EVENT_TYPE_CONNECT) {
                printf("Connection established\n");
                break;
            }
            else {
                enet_peer_reset (ctx.peer);
                printf("Connection attempt timed out. Retrying...\n");
            }
        }

    }

    void handle_connect(ENetEvent& event) {
        printf("New connection from %x:%u\n", event.peer->address.host, event.peer->address.port);
        ctx.peer = event.peer;
    }

    void handle_disconnect(ENetEvent& event) {
        printf("Client disconnect\n");
        event.peer->data = nullptr;
        ctx.peer = nullptr;
    }

    void handle_receive(ENetEvent& event) {
        auto mem = malloc(event.packet->dataLength);
        memcpy(mem, event.packet->data, event.packet->dataLength);
        ctx.receive_queue.push(std::shared_ptr<MessageHeader> ((MessageHeader*) mem, ::free));
        //printf("Receiving msg with size: %u\n", ((MessageHeader*) mem)->size);
        enet_packet_destroy(event.packet);
    }

    std::queue<std::shared_ptr<MessageHeader>>& access_received() {
        return ctx.receive_queue;
    }

    void tick() {

        ctx.queue_mutex.lock();
        while (!ctx.enqueued_msgs.empty()) {
            auto elem = ctx.enqueued_msgs.front();
            transmit(elem.first, elem.second);
            ctx.enqueued_msgs.pop();
        }
        ctx.queue_mutex.unlock();

        ENetEvent event;
        uint8_t limit_counter = 0;
        int res;
        while ((res = enet_host_service(ctx.host, &event, 1) > 0) && ++limit_counter < 100) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: handle_connect(event); break;
                case ENET_EVENT_TYPE_DISCONNECT: handle_disconnect(event); break;
                case ENET_EVENT_TYPE_RECEIVE: handle_receive(event); break;
                default: break;
            }
        }
        if (res < 0) {
            printf("ENet host_service error: %d\n", res);
        }
    }

    void destroy(NetworkPacket* msg) {
        ENetPacket* packet = (ENetPacket*) msg;
        //Free no longer necessary since every MessageHeader is now wrapped in a shared_ptr
        //free(packet->data);
        packet->data = nullptr;
        delete (std::shared_ptr<MessageHeader>*) packet->userData;
    }

    NetworkPacket* pack_message(std::shared_ptr<MessageHeader> header) {
        ENetPacket* packet = enet_packet_create(header.get(), header->size, ENET_PACKET_FLAG_NO_ALLOCATE | ENET_PACKET_FLAG_RELIABLE);
        packet->freeCallback = (void(*)(ENetPacket*)) destroy;
        packet->userData = new std::shared_ptr<MessageHeader>(header);
        return (NetworkPacket*) packet;
    }

    void transmit(NetworkPacket* msg, uint8_t channel_id) {
        auto packet = (ENetPacket*) msg;
        if (ctx.peer) {
            enet_peer_send(ctx.peer, channel_id, packet);
        }
    }

    void enqueue(NetworkPacket* msg, uint8_t channel_id) {
        ctx.queue_mutex.lock();
        ctx.enqueued_msgs.push({msg, channel_id});
        ctx.queue_mutex.unlock();
    }

    void shutdown() {
        if (!is_initialized()) return;
        if (ctx.peer) {
            enet_peer_disconnect(ctx.peer, 0);
        }
        enet_host_destroy(ctx.host);
        enet_deinitialize();
    }

}

