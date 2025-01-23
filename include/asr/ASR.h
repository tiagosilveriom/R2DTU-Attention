#pragma once

#include <string>
#include <optional>
#include <core/Node.h>

class ASR {
public:
    virtual void initialize(Node* node) = 0;

    virtual void start_recording() = 0;

    virtual std::string finish_recording() = 0;

    virtual void shutdown() = 0;

    virtual ~ASR() = 0;
};

class DanishASR : public ASR {
public:
    void initialize(Node* node) override ;

    void start_recording() override ;

    std::string finish_recording() override ;

    void shutdown() override;

    ~DanishASR() = default;
};

class EnglishASR : public ASR {
public:
    void initialize(Node* node) override ;

    void start_recording() override ;

    std::string finish_recording() override ;

    void shutdown() override ;

    ~EnglishASR() = default;
};

std::string cloud_listen();
