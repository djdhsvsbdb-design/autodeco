#include "DecorationPlan.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <utility>

namespace AutoDecoratorAI::AI {
namespace {

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

bool readString(const matjson::Value& object, const char* key, std::string& out,
                std::size_t maxLength, bool required, std::string& error) {
    const auto& value = object[key];
    if (!value.isString()) {
        if (!required && value.isNull()) { out.clear(); return true; }
        error = std::string("Field '") + key + "' must be a string.";
        return false;
    }
    out = trim(value.asString().unwrapOr(""));
    if (required && out.empty()) {
        error = std::string("Field '") + key + "' cannot be empty.";
        return false;
    }
    if (out.size() > maxLength) {
        error = std::string("Field '") + key + "' is too long.";
        return false;
    }
    return true;
}

bool readStringArray(const matjson::Value& object, const char* key,
                     std::vector<std::string>& out, std::size_t maxItems,
                     std::size_t maxLength, bool required, std::string& error) {
    const auto& value = object[key];
    if (!value.isArray()) {
        if (!required && value.isNull()) { out.clear(); return true; }
        error = std::string("Field '") + key + "' must be an array.";
        return false;
    }
    if (value.size() == 0 && required) {
        error = std::string("Field '") + key + "' cannot be empty.";
        return false;
    }
    if (value.size() > maxItems) {
        error = std::string("Field '") + key + "' has too many entries.";
        return false;
    }
    out.clear();
    for (const auto& item : value) {
        if (!item.isString()) {
            error = std::string("Every entry in '") + key + "' must be a string.";
            return false;
        }
        auto text = trim(item.asString().unwrapOr(""));
        if (text.empty() || text.size() > maxLength) {
            error = std::string("An entry in '") + key + "' is empty or too long.";
            return false;
        }
        out.push_back(std::move(text));
    }
    return true;
}

std::string unwrapFence(std::string input) {
    input = trim(std::move(input));
    if (input.rfind("```", 0) == 0) {
        const auto firstLine = input.find('\n');
        if (firstLine != std::string::npos) input.erase(0, firstLine + 1);
        const auto fence = input.rfind("```");
        if (fence != std::string::npos) input.erase(fence);
    }
    const auto first = input.find('{');
    const auto last = input.rfind('}');
    if (first != std::string::npos && last != std::string::npos && last >= first)
        return input.substr(first, last - first + 1);
    return input;
}

} // namespace

std::string DecorationPlan::makeSummary() const {
    std::ostringstream out;
    out << "STYLE: " << style << "\n";
    out << "MOOD: " << mood << "\n\n";
    out << "PALETTE: ";
    for (std::size_t i = 0; i < palette.size(); ++i) {
        if (i) out << ", ";
        out << palette[i];
    }
    out << "\n\nLAYERS:\n";
    for (const auto& layer : layers) {
        out << "- " << layer.name << ": " << layer.purpose << "\n";
        out << "  Placement: " << layer.placement << "\n";
        out << "  Color: " << layer.color << "\n";
        out << "  Elements: ";
        for (std::size_t i = 0; i < layer.elements.size(); ++i) {
            if (i) out << ", ";
            out << layer.elements[i];
        }
        out << "\n";
    }
    if (!notes.empty()) {
        out << "\nNOTES:\n";
        for (const auto& note : notes) out << "- " << note << "\n";
    }
    return out.str();
}

bool DecorationPlanParser::parse(const std::string& response, const std::vector<int>& allowedObjectIds,
                                 DecorationPlan& out, std::string& error) {
    error.clear();
    if (response.empty() || response.size() > 24000) {
        error = "The AI response is empty or exceeds the 24 KB plan limit.";
        return false;
    }

    auto json = matjson::Value::parse(unwrapFence(response));
    if (!json) {
        error = "The AI response is not valid JSON.";
        return false;
    }
    auto root = json.unwrapOr(matjson::Value{});
    if (!root.isObject()) {
        error = "The decoration plan must be a JSON object.";
        return false;
    }

    DecorationPlan candidate;
    if (!readString(root, "style", candidate.style, 120, true, error) ||
        !readString(root, "mood", candidate.mood, 160, true, error) ||
        !readStringArray(root, "palette", candidate.palette, 8, 48, true, error) ||
        !readStringArray(root, "notes", candidate.notes, 8, 180, false, error)) {
        return false;
    }

    const auto& layers = root["layers"];
    if (!layers.isArray() || layers.size() == 0 || layers.size() > 8) {
        error = "The plan must contain between 1 and 8 decoration layers.";
        return false;
    }

    for (const auto& item : layers) {
        if (!item.isObject()) {
            error = "Each decoration layer must be an object.";
            return false;
        }
        DecorationLayer layer;
        if (!readString(item, "name", layer.name, 64, true, error) ||
            !readString(item, "purpose", layer.purpose, 180, true, error) ||
            !readStringArray(item, "elements", layer.elements, 8, 64, true, error) ||
            !readString(item, "placement", layer.placement, 180, true, error) ||
            !readString(item, "color", layer.color, 64, true, error)) {
            return false;
        }
        candidate.layers.push_back(std::move(layer));
    }

    const auto& placements = root["placements"];
    if (!placements.isArray() || placements.size() == 0 || placements.size() > 120) {
        error = "The plan must contain between 1 and 120 object placements.";
        return false;
    }

    for (const auto& item : placements) {
        if (!item.isObject()) {
            error = "Each placement must be a JSON object.";
            return false;
        }

        if (!item["object_id"].isNumber() || !item["x"].isNumber() ||
            !item["y"].isNumber() || !item["scale"].isNumber() ||
            !item["rotation"].isNumber() || !item["color"].isString()) {
            error = "A placement is missing a required numeric or color field.";
            return false;
        }

        auto objectId = item["object_id"].asInt().unwrapOr(-1);
        auto x = item["x"].asDouble().unwrapOr(-1.0);
        auto y = item["y"].asDouble().unwrapOr(-1.0);
        auto scale = item["scale"].asDouble().unwrapOr(1.0);
        auto rotation = item["rotation"].asDouble().unwrapOr(0.0);
        auto color = item["color"].asString().unwrapOr("");

        if (objectId <= 0 || objectId > 1000000 ||
            std::find(allowedObjectIds.begin(), allowedObjectIds.end(), static_cast<int>(objectId)) == allowedObjectIds.end()) {
            error = "A placement references an object that is not in the verified decoration catalogue.";
            return false;
        }
        if (x < 0.0 || x > 1.0 || y < 0.0 || y > 1.0) {
            error = "Placement coordinates must be normalized between 0 and 1.";
            return false;
        }
        if (scale < 0.25 || scale > 3.0 || rotation < -360.0 || rotation > 360.0) {
            error = "A placement has an unsafe scale or rotation value.";
            return false;
        }
        if (color.size() != 7 || color[0] != '#' ||
            !std::all_of(color.begin() + 1, color.end(), [](unsigned char ch) {
                return std::isxdigit(ch) != 0;
            })) {
            error = "Placement colors must use #RRGGBB hex format.";
            return false;
        }

        candidate.placements.push_back(ObjectPlacement{
            static_cast<int>(objectId),
            static_cast<float>(x),
            static_cast<float>(y),
            static_cast<float>(scale),
            static_cast<float>(rotation),
            color
        });
    }

    out = std::move(candidate);
    return true;
}

} // namespace AutoDecoratorAI::AI
