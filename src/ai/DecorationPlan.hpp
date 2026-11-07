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

struct ObjectPlacement {
    int objectId = 0;
    float x = 0.f;       // normalized 0..1 within the selected area
    float y = 0.f;       // normalized 0..1 within the selected area
    float scale = 1.f;
    float rotation = 0.f;
    std::string color = "#FFFFFF";
};

struct DecorationPlan {
    std::string style;
    std::string mood;
    std::vector<std::string> palette;
    std::vector<DecorationLayer> layers;
    std::vector<std::string> notes;
    std::vector<ObjectPlacement> placements;

    std::string makeSummary() const;
};

class DecorationPlanParser {
public:
    // Accepts either plain JSON or JSON wrapped in a Markdown code fence.
    // Rejects unknown/oversized structures before the editor can use the plan.
    static bool parse(const std::string& response, const std::vector<int>& allowedObjectIds,
                      DecorationPlan& out, std::string& error);
};

} // namespace AutoDecoratorAI::AI
