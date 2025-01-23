#pragma once

#include <core/Node.h>
#include "ui/View.h"
#include "ScriptRunner.h"

class ScriptRunnerView : public View {
public:
    ScriptRunnerView(Node& node);

    virtual void create(RenderContext& context) override;
    virtual std::string title() override;
    virtual void render(RenderContext& context) override;

private:
    ScriptRunner runner_;
};