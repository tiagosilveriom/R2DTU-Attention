#include <core/KVStore.h>
#include <hiredis/hiredis.h>
#include <common/Debug.h>
#include <cstring>
#include <mutex>

void KVStore::init(std::string ip, uint16_t port) {
    std::lock_guard<std::mutex> lock(monitor);
    //TODO Remove this hardcoded IP when the leader system is actually implemented
    ctx = redisConnect(ip.c_str(), port);
    if (!ctx) {
        printf("Failed to allocate redis context!\n");
        std::abort();
    }
    if (ctx->err) {
        printf("Failed to connect to redis at %s:%u: %s\n", ip.c_str(), port, ctx->errstr);
        redisFree(ctx);
        std::abort();
    }
}

void KVStore::shutdown() {
    std::lock_guard<std::mutex> lock(monitor);
    redisFree(ctx);
}

bool KVStore::set(UpdateMode mode, std::string const &path, std::string const &value) {
    std::lock_guard<std::mutex> lock(monitor);
    const char* mode_cmd;
    switch (mode) {
        case Always: mode_cmd = nullptr; break;
        case NX: mode_cmd = "NX"; break;
        case XX: mode_cmd = "XX"; break;
        default: ASSERT(false, "Exhaustive switch hit default!\n");
    }
    redisReply* reply;
    if (mode) {
        reply = (redisReply*) redisCommand(ctx, "SET %s %s %s\n", path.c_str(), value.c_str(), mode_cmd);
    } else {
        reply = (redisReply*) redisCommand(ctx, "SET %s %s\n", path.c_str(), value.c_str());
    }
    ASSERTF(reply->type == REDIS_REPLY_STATUS, "redis SET reply: %u, %s\n", reply->type, reply->str);
    auto res = strncmp("OK", reply->str, 2) == 0;
    freeReplyObject(reply);
    return res;
}

bool KVStore::del(std::string const& path) {
    std::lock_guard<std::mutex> lock(monitor);
    redisReply* reply = (redisReply*) redisCommand(ctx, "DEL %s", path.c_str());
    ASSERTF(reply->type == REDIS_REPLY_INTEGER, "redis DEL reply: %u, %s\n", reply->type, reply->str);
    auto res = reply->integer == 1;
    freeReplyObject(reply);
    return res;
}

bool KVStore::exists(std::string const& path) {
    std::lock_guard<std::mutex> lock(monitor);
    redisReply* reply = (redisReply*) redisCommand(ctx, "EXISTS %s", path.c_str());
    ASSERTF(reply->type == REDIS_REPLY_INTEGER, "redis EXISTS reply: %u, %s\n", reply->type, reply->str);
    auto res = reply->integer == 1;
    freeReplyObject(reply);
    return res;
}

Option<std::string> KVStore::get(std::string const& path) {
    std::lock_guard<std::mutex> lock(monitor);
    redisReply* reply = (redisReply*) redisCommand(ctx, "GET %s", path.c_str());
    ASSERTF(reply->type == REDIS_REPLY_STRING || reply->type == REDIS_REPLY_NIL,
            "redis GET reply: %u, %s\n", reply->type, reply->str);
    Option<std::string> res;
    if (reply->type == REDIS_REPLY_NIL) {
        res = Option<std::string>();
    } else {
        auto s = std::string(reply->str);
        if (s.back() == '\n') {
            s.pop_back();
        }
        res = Option<std::string>(s);
    }
    freeReplyObject(reply);
    return res;
}

uint32_t  KVStore::incr(std::string const& path) {
    std::lock_guard<std::mutex> lock(monitor);
    redisReply* reply = (redisReply*) redisCommand(ctx, "INCR %s", path.c_str());
    ASSERTF(reply->type == REDIS_REPLY_INTEGER, "redis INCR reply: %u, %s\n", reply->type, reply->str);
    auto res = static_cast<uint32_t >(reply->integer);
    freeReplyObject(reply);
    return res;
}

bool KVStore::zadd(UpdateMode mode, std::string const& path, std::string const& elem, uint32_t index) {
    std::lock_guard<std::mutex> lock(monitor);
    const char* mode_cmd = mode == NX ? "NX" : "XX";
    redisReply* reply = (redisReply*) redisCommand(ctx, "ZADD %s %s %u %s", path.c_str(), mode_cmd, index, elem.c_str());
    ASSERTF(reply->type == REDIS_REPLY_INTEGER, "redis ZADD reply: %u, %s\n", reply->type, reply->str);
    bool res = reply->integer == 1;
    freeReplyObject(reply);
    return res;
}

Option<uint32_t> KVStore::zscore(std::string const& path, std::string const& elem) {
    std::lock_guard<std::mutex> lock(monitor);
    redisReply* reply = (redisReply*) redisCommand(ctx, "ZSCORE %s %s", path.c_str(), elem.c_str());
    ASSERTF(reply->type == REDIS_REPLY_NIL || reply->type == REDIS_REPLY_STRING,
            "redis ZSCORE reply: %u, %s\n", reply->type, reply->str);
    auto res = (reply->type == REDIS_REPLY_NIL) ? Option<uint32_t >() : Option<uint32_t>(std::stoi(reply->str));
    freeReplyObject(reply);
    return res;
}
