#pragma once

#include "ui/View.h"

class EmptyView : public View {
public:
    EmptyView() : View("Empty") {}

    virtual void create(RenderContext&) override {};
    virtual std::string title() override;
    virtual void render(RenderContext& context) override;
};

