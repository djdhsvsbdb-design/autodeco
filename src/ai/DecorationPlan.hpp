#pragma once

#include <string>
#include <vector>

namespace AutoDecoratorAI::AI {

struct DecorationLayer {
    std::string name;
    std::string purpose;
    std::vector<std::string> elements;
    std::string placement;
    std::string color;
};

struct DecorationPlan {
    std::string style;
    std::string mood;
    std::vector<std::string> palette;
    std::vector<DecorationLayer> layers;
    std::vector<std::string> notes;

    std::string makeSummary() const;
};

class DecorationPlanParser {
public:
    // Accepts either plain JSON or JSON wrapped in a Markdown code fence.
    // Rejects unknown/oversized structures before the editor can use the plan.
    static bool parse(const std::string& response, DecorationPlan& out, std::string& error);
};

} // namespace AutoDecoratorAI::AI
