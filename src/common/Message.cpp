#include <common/Message.h>
#include <assert.h>
#include <cstdio>

namespace Messages {

    MessageHeader* alloc(uint32_t msg_size) {
        auto size = sizeof(MessageHeader) + msg_size;
        auto header = (MessageHeader*) malloc(size);
        header->size = size;
        header->channel = 0;
        return header;
    }

    void* content_offset(const MessageHeader* header, uint64_t offset) {
        return &(((uint8_t*) header)[sizeof(MessageHeader) + offset]);
    }

    std::string read_string(const MsgText* str) {
        auto offset = *str & MSG_OFFSET_MASK;
        auto len = (*str & MSG_LEN_MASK) >> MSG_LEN_SHIFT;
        auto base_ptr = (uint8_t*) str;
        auto target_ptr = (const char*) (base_ptr + offset);
        std::string res(target_ptr, len-1);
        return res;
    }

    const std::pair<uint8_t*, uint32_t> access_blob(const MsgBlob* blob) {
        auto offset = *blob & MSG_OFFSET_MASK;
        auto len = (*blob & MSG_LEN_MASK) >> MSG_LEN_SHIFT;
        auto base_ptr = (uint8_t*) blob;
        return {(base_ptr + offset), len};
    }

    void free(const MessageHeader* header) {
        ::free((MessageHeader*) header);
    }
}