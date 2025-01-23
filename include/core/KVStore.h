#pragma once

#include <common/Option.h>
#include <string>
#include <mutex>

struct redisContext;

/**
 * KVStore is a C++ wrapper around hiredis.
 * All methods maps directly to the corresponding Redis command
 * The interface is made thread-safe by a monitor-like mutex.
 */
class KVStore {
public:

    void init(std::string ip, uint16_t port);

    void shutdown();

    enum UpdateMode {
        Always,
        NX, //Only set key if it does not already exist
        XX, //Only set key if it already exist
    };

    bool set(UpdateMode mode, std::string const& path, std::string const& value);

    bool del(std::string const& path);

    bool exists(std::string const& path);

    Option<std::string> get(std::string const& path);

    uint32_t  incr(std::string const& path);

    bool zadd(UpdateMode mode, std::string const& path, std::string const& elem, uint32_t index);

    Option<uint32_t> zscore(std::string const& path, std::string const& elem);

private:
    redisContext* ctx;
    std::mutex monitor;
};
