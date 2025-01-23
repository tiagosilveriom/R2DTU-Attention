#pragma once

#include <core/Types.h>
#include <common/Debug.h>
#include <stdlib.h>
#include <memory>
#include <cstring>

using MsgText = uint64_t;
using MsgBlob = uint64_t;
const uint64_t MSG_OFFSET_MASK = 0x00000000FFFFFFFF;
const uint64_t MSG_LEN_MASK    = 0xFFFFFFFF00000000;
const uint64_t MSG_LEN_SHIFT = 32;

#pragma pack(push, 1)
struct MessageHeader {
    uint32_t size;
    uint32_t channel;
};

static_assert(sizeof(MessageHeader) == 8, "Invalid message header padding");

#pragma pack(pop)

namespace Messages {

    MessageHeader* alloc(uint32_t msg_size);

    void* content_offset(const MessageHeader* header, uint64_t offset);

    template <typename Msg>
    Msg const* content(const MessageHeader* header) {
        return (Msg*) content_offset(header, 0);
    }

    std::string read_string(const MsgText* str);

    const std::pair<uint8_t*, uint32_t> access_blob(const MsgBlob* blob);

    void free(const MessageHeader* header);
}

template <typename Msg>
class MessageBuilder {
public:
    MessageBuilder()
    : msg_size_(sizeof(Msg)), offset_(msg_size_) {}

    Msg* operator->() {
        return &msg;
    }

    std::shared_ptr<MessageHeader> pack() {

        auto header = Messages::alloc(offset_);

        ::memcpy(Messages::content_offset(header, 0), &msg, msg_size_);

        uint64_t counter = msg_size_;
        for (int i = 0; i < next_block_; i++) {
            uint64_t size = block_sizes_[i];
            ::memcpy(Messages::content_offset(header, counter), blocks_[i], size);
            counter += size;
        }

        return std::shared_ptr<MessageHeader>(header, Messages::free);
    }

    void write_string(MsgText* ptr, const std::string& str) {
        auto i = next_block_++;
        ASSERTF(i < MAX_BLOCKS, "A message can at most contain %u complex data types!\n", MAX_BLOCKS);
        uint64_t size = str.size() + 1;
        auto distance = offset_ - ( ((uint8_t*)ptr) - ((uint8_t*) &msg));
        blocks_[i] = (const uint8_t*) str.c_str();
        block_sizes_[i] = size;
        *ptr = (size << MSG_LEN_SHIFT) | distance;
        offset_ += size;
    }

    void write_blob(MsgBlob* ptr, uint8_t* blob, uint64_t blob_size) {
        auto i = next_block_++;
        ASSERTF(i < MAX_BLOCKS, "A message can at most contain %u complex data types!\n", MAX_BLOCKS);
        auto distance = offset_ - ( ((uint8_t*)ptr) - ((uint8_t*) &msg));
        blocks_[i] = blob;
        block_sizes_[i] = blob_size;
        *ptr = (blob_size << MSG_LEN_SHIFT) | distance;
        offset_ += blob_size;
    }

private:
    Msg msg;
    const static uint32_t MAX_BLOCKS = 16;

    const uint64_t msg_size_;

    const uint8_t * blocks_[MAX_BLOCKS];
    uint64_t block_sizes_[MAX_BLOCKS];
    uint8_t next_block_ = 0;
    uint64_t offset_ = 0;
};
