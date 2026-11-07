#pragma once

#include <Geode/Geode.hpp>
#include <algorithm>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>

namespace AutoDecoratorAI {

enum class StyleBias {
    CleanModern,
    NeonGlow,
    DemonExtreme,
    RetroPixel,
    Architectural
};

enum class SymmetryType {
    None,
    Horizontal,
    Vertical,
    Radial,
    Bilateral
};

enum class ShapeType {
    Linear,
    Vertical,
    Enclosed,
    Sparse,
    Complex
};

// Strict allow-list of decorative Geometry Dash 2.2081 object IDs.
// Keeping this list narrow prevents custom presets from inserting gameplay
// objects such as orbs, portals, pads, triggers, or hazards.
inline bool isAllowedDecorationObjectID(int id) {
    switch (id) {
        case 15: case 16: case 17:
        case 48: case 49:
        case 50: case 51: case 52: case 53: case 54:
        case 85: case 86: case 87:
        case 113: case 114: case 115:
        case 157: case 158: case 159:
        case 180: case 181: case 182:
        case 225: case 226: case 227: case 228:
        case 233: case 234: case 235:
        case 245: case 246:
        case 277: case 278:
            return true;
        default:
            return false;
    }
}

struct DecorationObject {
    int objectID;
    float weight;
    std::string category;
    bool requiresGlow;
    bool isDetail;
};

struct DensityCurve {
    float baseDensity;
    float edgeMultiplier;
    float cornerMultiplier;
    float centerMultiplier;
    float flowMultiplier;
    
    DensityCurve() : baseDensity(0.5f), edgeMultiplier(1.5f), cornerMultiplier(2.0f), centerMultiplier(0.8f), flowMultiplier(1.2f) {}
    
    float getDensityAt(float normalizedPosition, bool isEdge, bool isCorner, bool isCenter, bool isFlow) const {
        float density = baseDensity;
        if (isEdge) density *= edgeMultiplier;
        if (isCorner) density *= cornerMultiplier;
        if (isCenter) density *= centerMultiplier;
        if (isFlow) density *= flowMultiplier;
        return std::clamp(density, 0.0f, 1.0f);
    }
};

struct AIPreset {
    std::string id;
    std::string name;
    std::string description;
    
    StyleBias styleBias;
    DensityCurve densityCurve;
    
    float symmetryPreference;
    float chaosTolerance;
    float clusteringThreshold;
    float microDetailLevel;
    
    bool edgeEmphasis;
    bool flowFollowing;
    bool symmetryMirroring;
    bool antiClutterMode;
    
    std::vector<DecorationObject> decorationPool;
    std::unordered_map<std::string, float> styleWeights;
    
    AIPreset() 
        : styleBias(StyleBias::CleanModern)
        , symmetryPreference(0.7f)
        , chaosTolerance(0.3f)
        , clusteringThreshold(0.5f)
        , microDetailLevel(0.4f)
        , edgeEmphasis(true)
        , flowFollowing(true)
        , symmetryMirroring(true)
        , antiClutterMode(false) {}
    
    matjson::Value toJson() const;
    static AIPreset fromJson(const matjson::Value& json);
    
private:
    static StyleBias styleBiasFromString(const std::string& str);
    static std::string styleBiasToString(StyleBias bias);
};

struct GeometryCluster {
    std::vector<geode::Ref<GameObject>> objects;
    cocos2d::CCRect boundingBox;
    ShapeType shapeType;
    float density;
    float visualImportance;
    cocos2d::CCPoint centroid;
    
    GeometryCluster() : shapeType(ShapeType::Sparse), density(0.0f), visualImportance(0.5f), centroid(0, 0) {}
};

struct PatternDetectionResult {
    SymmetryType symmetryType;
    float symmetryStrength;
    bool hasRepetition;
    float repetitionPeriod;
    cocos2d::CCPoint flowDirection;
    float densityImbalance;
    std::vector<cocos2d::CCRect> emptyZones;
    std::vector<cocos2d::CCRect> denseZones;
    
    PatternDetectionResult() 
        : symmetryType(SymmetryType::None)
        , symmetryStrength(0.0f)
        , hasRepetition(false)
        , repetitionPeriod(0.0f)
        , flowDirection(1, 0)
        , densityImbalance(0.0f) {}
};

struct DecorationZone {
    cocos2d::CCRect area;
    float decorationDensity;
    std::string zoneType;
    bool isEdge;
    bool isCorner;
    bool isCenter;
    bool isFlow;
    
    DecorationZone() : decorationDensity(0.5f), zoneType("fill"), isEdge(false), isCorner(false), isCenter(false), isFlow(false) {}
};

struct DecorationPlacement {
    cocos2d::CCPoint position;
    int objectID;
    float rotation;
    float scale;
    int targetGroup;
    
    DecorationPlacement() : objectID(0), rotation(0.0f), scale(1.0f), targetGroup(0) {}
};

}
