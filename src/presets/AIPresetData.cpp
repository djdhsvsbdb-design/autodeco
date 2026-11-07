#include "AIPresetData.hpp"
#include <Geode/Geode.hpp>
#include <cmath>

using namespace geode::prelude;

namespace AutoDecoratorAI {

matjson::Value AIPreset::toJson() const {
    matjson::Value json = matjson::Value::object();

    json["id"] = id;
    json["name"] = name;
    json["description"] = description;
    json["styleBias"] = styleBiasToString(styleBias);

    json["densityCurve"] = matjson::Value::object();
    json["densityCurve"]["baseDensity"] = densityCurve.baseDensity;
    json["densityCurve"]["edgeMultiplier"] = densityCurve.edgeMultiplier;
    json["densityCurve"]["cornerMultiplier"] = densityCurve.cornerMultiplier;
    json["densityCurve"]["centerMultiplier"] = densityCurve.centerMultiplier;
    json["densityCurve"]["flowMultiplier"] = densityCurve.flowMultiplier;

    json["symmetryPreference"] = symmetryPreference;
    json["chaosTolerance"] = chaosTolerance;
    json["clusteringThreshold"] = clusteringThreshold;
    json["microDetailLevel"] = microDetailLevel;

    json["edgeEmphasis"] = edgeEmphasis;
    json["flowFollowing"] = flowFollowing;
    json["symmetryMirroring"] = symmetryMirroring;
    json["antiClutterMode"] = antiClutterMode;

    matjson::Value decorationPoolJson = matjson::Value::array();
    for (const auto& obj : decorationPool) {
        matjson::Value objJson = matjson::Value::object();
        objJson["objectID"] = obj.objectID;
        objJson["weight"] = obj.weight;
        objJson["category"] = obj.category;
        objJson["requiresGlow"] = obj.requiresGlow;
        objJson["isDetail"] = obj.isDetail;
        decorationPoolJson.push(std::move(objJson));
    }
    json["decorationPool"] = std::move(decorationPoolJson);

    matjson::Value styleWeightsJson = matjson::Value::object();
    for (const auto& pair : styleWeights) {
        styleWeightsJson[pair.first] = pair.second;
    }
    json["styleWeights"] = std::move(styleWeightsJson);

    return json;
}

AIPreset AIPreset::fromJson(const matjson::Value& json) {
    AIPreset preset;

    preset.id = json["id"].asString().unwrapOr("");
    preset.name = json["name"].asString().unwrapOr("");
    preset.description = json["description"].asString().unwrapOr("");
    preset.styleBias = styleBiasFromString(
        json["styleBias"].asString().unwrapOr("CleanModern")
    );

    if (json.contains("densityCurve")) {
        const auto& dc = json["densityCurve"];
        preset.densityCurve.baseDensity =
            static_cast<float>(dc["baseDensity"].asDouble().unwrapOr(preset.densityCurve.baseDensity));
        preset.densityCurve.edgeMultiplier =
            static_cast<float>(dc["edgeMultiplier"].asDouble().unwrapOr(preset.densityCurve.edgeMultiplier));
        preset.densityCurve.cornerMultiplier =
            static_cast<float>(dc["cornerMultiplier"].asDouble().unwrapOr(preset.densityCurve.cornerMultiplier));
        preset.densityCurve.centerMultiplier =
            static_cast<float>(dc["centerMultiplier"].asDouble().unwrapOr(preset.densityCurve.centerMultiplier));
        preset.densityCurve.flowMultiplier =
            static_cast<float>(dc["flowMultiplier"].asDouble().unwrapOr(preset.densityCurve.flowMultiplier));
    }

    preset.symmetryPreference = static_cast<float>(
        json["symmetryPreference"].asDouble().unwrapOr(preset.symmetryPreference)
    );
    preset.chaosTolerance = static_cast<float>(
        json["chaosTolerance"].asDouble().unwrapOr(preset.chaosTolerance)
    );
    preset.clusteringThreshold = static_cast<float>(
        json["clusteringThreshold"].asDouble().unwrapOr(preset.clusteringThreshold)
    );
    preset.microDetailLevel = static_cast<float>(
        json["microDetailLevel"].asDouble().unwrapOr(preset.microDetailLevel)
    );

    preset.edgeEmphasis = json["edgeEmphasis"].asBool().unwrapOr(preset.edgeEmphasis);
    preset.flowFollowing = json["flowFollowing"].asBool().unwrapOr(preset.flowFollowing);
    preset.symmetryMirroring = json["symmetryMirroring"].asBool().unwrapOr(preset.symmetryMirroring);
    preset.antiClutterMode = json["antiClutterMode"].asBool().unwrapOr(preset.antiClutterMode);

    if (json.contains("decorationPool")) {
        auto arrayResult = json["decorationPool"].asArray();
        if (arrayResult) {
            for (const auto& objJson : arrayResult.unwrap()) {
                DecorationObject obj{};
                obj.objectID = static_cast<int>(objJson["objectID"].asInt().unwrapOr(0));
                obj.weight = static_cast<float>(objJson["weight"].asDouble().unwrapOr(0.0));
                obj.category = objJson["category"].asString().unwrapOr("");
                obj.requiresGlow = objJson["requiresGlow"].asBool().unwrapOr(false);
                obj.isDetail = objJson["isDetail"].asBool().unwrapOr(false);

                // Reject invalid IDs and weights when loading user-authored files.
                if (isAllowedDecorationObjectID(obj.objectID) &&
                    std::isfinite(obj.weight) && obj.weight > 0.0f) {
                    preset.decorationPool.push_back(std::move(obj));
                }
            }
        }
    }

    if (json.contains("styleWeights")) {
        const auto& weights = json["styleWeights"];
        if (weights.isObject()) {
            for (const auto& [key, valueJson] : weights) {
                const float value = static_cast<float>(
                    valueJson.asDouble().unwrapOr(0.0)
                );
                if (std::isfinite(value) && value >= 0.0f) {
                    preset.styleWeights[key] = value;
                }
            }
        }
    }

    return preset;
}

std::string AIPreset::styleBiasToString(StyleBias bias) {
    switch (bias) {
        case StyleBias::CleanModern: return "CleanModern";
        case StyleBias::NeonGlow: return "NeonGlow";
        case StyleBias::DemonExtreme: return "DemonExtreme";
        case StyleBias::RetroPixel: return "RetroPixel";
        case StyleBias::Architectural: return "Architectural";
        default: return "CleanModern";
    }
}

StyleBias AIPreset::styleBiasFromString(const std::string& str) {
    if (str == "CleanModern") return StyleBias::CleanModern;
    if (str == "NeonGlow") return StyleBias::NeonGlow;
    if (str == "DemonExtreme") return StyleBias::DemonExtreme;
    if (str == "RetroPixel") return StyleBias::RetroPixel;
    if (str == "Architectural") return StyleBias::Architectural;
    return StyleBias::CleanModern;
}

}
