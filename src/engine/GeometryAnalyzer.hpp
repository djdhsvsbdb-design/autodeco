#pragma once

#include <Geode/Geode.hpp>
#include "../presets/AIPresetData.hpp"
#include <vector>
#include <memory>

namespace AutoDecoratorAI {

class GeometryAnalyzer {
public:
    static GeometryAnalyzer* get();
    
    std::vector<GeometryCluster> analyzeSelection(const std::vector<geode::Ref<GameObject>>& selectedObjects);
    GeometryCluster classifyCluster(const std::vector<geode::Ref<GameObject>>& objects);
    ShapeType detectShapeType(const GeometryCluster& cluster);
    float computeDensity(const GeometryCluster& cluster);
    float computeVisualImportance(const GeometryCluster& cluster);
    cocos2d::CCPoint computeCentroid(const std::vector<geode::Ref<GameObject>>& objects);
    cocos2d::CCRect computeBoundingBox(const std::vector<geode::Ref<GameObject>>& objects);
    
    std::vector<cocos2d::CCRect> detectEdges(const GeometryCluster& cluster);
    std::vector<cocos2d::CCPoint> detectCorners(const GeometryCluster& cluster);
    cocos2d::CCRect detectSymmetryCenter(const GeometryCluster& cluster);
    
    std::vector<DecorationZone> generateDecorationZones(const GeometryCluster& cluster, const AIPreset& preset);
    
private:
    GeometryAnalyzer();
    ~GeometryAnalyzer();
    
    static GeometryAnalyzer* s_instance;
    
    float distance(const cocos2d::CCPoint& a, const cocos2d::CCPoint& b);
    bool isLinearStructure(const std::vector<geode::Ref<GameObject>>& objects);
    bool isVerticalStructure(const std::vector<geode::Ref<GameObject>>& objects);
    bool isEnclosedShape(const std::vector<geode::Ref<GameObject>>& objects);
    bool isSparsePlacement(const std::vector<geode::Ref<GameObject>>& objects);
    
    std::vector<std::vector<geode::Ref<GameObject>>> clusterObjectsByProximity(const std::vector<geode::Ref<GameObject>>& objects, float threshold);
};

}
