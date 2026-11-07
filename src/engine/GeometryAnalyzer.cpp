#include "GeometryAnalyzer.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>
#include <cfloat>

using namespace geode::prelude;

namespace AutoDecoratorAI {

GeometryAnalyzer* GeometryAnalyzer::s_instance = nullptr;

GeometryAnalyzer::GeometryAnalyzer() {}

GeometryAnalyzer::~GeometryAnalyzer() {}

GeometryAnalyzer* GeometryAnalyzer::get() {
    if (!s_instance) {
        s_instance = new GeometryAnalyzer();
    }
    return s_instance;
}

std::vector<GeometryCluster> GeometryAnalyzer::analyzeSelection(const std::vector<Ref<GameObject>>& selectedObjects) {
    std::vector<GeometryCluster> clusters;
    
    if (selectedObjects.empty()) {
        return clusters;
    }
    
    auto objectClusters = clusterObjectsByProximity(selectedObjects, 150.0f);
    
    for (const auto& clusterObjects : objectClusters) {
        GeometryCluster cluster = classifyCluster(clusterObjects);
        clusters.push_back(cluster);
    }
    
    return clusters;
}

GeometryCluster GeometryAnalyzer::classifyCluster(const std::vector<Ref<GameObject>>& objects) {
    GeometryCluster cluster;
    cluster.objects = objects;
    cluster.boundingBox = computeBoundingBox(objects);
    cluster.shapeType = detectShapeType(cluster);
    cluster.density = computeDensity(cluster);
    cluster.visualImportance = computeVisualImportance(cluster);
    cluster.centroid = computeCentroid(objects);
    
    return cluster;
}

ShapeType GeometryAnalyzer::detectShapeType(const GeometryCluster& cluster) {
    if (isLinearStructure(cluster.objects)) {
        return ShapeType::Linear;
    }
    if (isVerticalStructure(cluster.objects)) {
        return ShapeType::Vertical;
    }
    if (isEnclosedShape(cluster.objects)) {
        return ShapeType::Enclosed;
    }
    if (isSparsePlacement(cluster.objects)) {
        return ShapeType::Sparse;
    }
    return ShapeType::Complex;
}

float GeometryAnalyzer::computeDensity(const GeometryCluster& cluster) {
    if (cluster.objects.empty() || cluster.boundingBox.size.width <= 0 || cluster.boundingBox.size.height <= 0) {
        return 0.0f;
    }
    
    float area = cluster.boundingBox.size.width * cluster.boundingBox.size.height;
    float objectArea = cluster.objects.size() * 900.0f; // Approximate 30x30 per object
    
    return std::min(objectArea / area, 1.0f);
}

float GeometryAnalyzer::computeVisualImportance(const GeometryCluster& cluster) {
    float importance = 0.5f;
    
    // Edge objects get higher importance
    auto edges = detectEdges(cluster);
    importance += edges.size() * 0.1f;
    
    // Corner objects get even higher importance
    auto corners = detectCorners(cluster);
    importance += corners.size() * 0.15f;
    
    // Symmetry center gets high importance
    auto symCenter = detectSymmetryCenter(cluster);
    if (symCenter.size.width > 0) {
        importance += 0.2f;
    }
    
    // Density affects importance (moderate density is most important)
    if (cluster.density > 0.3f && cluster.density < 0.7f) {
        importance += 0.1f;
    }
    
    return std::clamp(importance, 0.0f, 1.0f);
}

cocos2d::CCPoint GeometryAnalyzer::computeCentroid(const std::vector<Ref<GameObject>>& objects) {
    if (objects.empty()) {
        return cocos2d::CCPoint(0, 0);
    }
    
    float sumX = 0.0f;
    float sumY = 0.0f;
    
    for (const auto& obj : objects) {
        sumX += obj->getPositionX();
        sumY += obj->getPositionY();
    }
    
    return cocos2d::CCPoint(sumX / objects.size(), sumY / objects.size());
}

cocos2d::CCRect GeometryAnalyzer::computeBoundingBox(const std::vector<Ref<GameObject>>& objects) {
    if (objects.empty()) {
        return cocos2d::CCRect(0, 0, 0, 0);
    }
    
    float minX = FLT_MAX;
    float minY = FLT_MAX;
    float maxX = -FLT_MAX;
    float maxY = -FLT_MAX;
    
    for (const auto& obj : objects) {
        float x = obj->getPositionX();
        float y = obj->getPositionY();
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }
    
    return cocos2d::CCRect(minX, minY, maxX - minX, maxY - minY);
}

std::vector<cocos2d::CCRect> GeometryAnalyzer::detectEdges(const GeometryCluster& cluster) {
    std::vector<cocos2d::CCRect> edges;
    
    if (cluster.objects.size() < 2) {
        return edges;
    }
    
    auto bbox = cluster.boundingBox;
    float edgeThreshold = 30.0f;
    
    // Detect top edge
    cocos2d::CCRect topEdge(bbox.origin.x, bbox.origin.y + bbox.size.height - edgeThreshold, 
                           bbox.size.width, edgeThreshold);
    edges.push_back(topEdge);
    
    // Detect bottom edge
    cocos2d::CCRect bottomEdge(bbox.origin.x, bbox.origin.y, 
                              bbox.size.width, edgeThreshold);
    edges.push_back(bottomEdge);
    
    // Detect left edge
    cocos2d::CCRect leftEdge(bbox.origin.x, bbox.origin.y, 
                             edgeThreshold, bbox.size.height);
    edges.push_back(leftEdge);
    
    // Detect right edge
    cocos2d::CCRect rightEdge(bbox.origin.x + bbox.size.width - edgeThreshold, bbox.origin.y, 
                              edgeThreshold, bbox.size.height);
    edges.push_back(rightEdge);
    
    return edges;
}

std::vector<cocos2d::CCPoint> GeometryAnalyzer::detectCorners(const GeometryCluster& cluster) {
    std::vector<cocos2d::CCPoint> corners;
    
    auto bbox = cluster.boundingBox;
    
    corners.push_back(cocos2d::CCPoint(bbox.origin.x, bbox.origin.y));
    corners.push_back(cocos2d::CCPoint(bbox.origin.x + bbox.size.width, bbox.origin.y));
    corners.push_back(cocos2d::CCPoint(bbox.origin.x, bbox.origin.y + bbox.size.height));
    corners.push_back(cocos2d::CCPoint(bbox.origin.x + bbox.size.width, bbox.origin.y + bbox.size.height));
    
    return corners;
}

cocos2d::CCRect GeometryAnalyzer::detectSymmetryCenter(const GeometryCluster& cluster) {
    auto centroid = cluster.centroid;
    auto bbox = cluster.boundingBox;
    
    // Check if centroid is near the center of bounding box
    cocos2d::CCPoint bboxCenter(bbox.origin.x + bbox.size.width / 2, 
                                bbox.origin.y + bbox.size.height / 2);
    
    float dist = distance(centroid, bboxCenter);
    
    if (dist < bbox.size.width * 0.1f) {
        return cocos2d::CCRect(centroid.x - 20, centroid.y - 20, 40, 40);
    }
    
    return cocos2d::CCRect(0, 0, 0, 0);
}

std::vector<DecorationZone> GeometryAnalyzer::generateDecorationZones(const GeometryCluster& cluster, const AIPreset& preset) {
    std::vector<DecorationZone> zones;
    
    auto bbox = cluster.boundingBox;
    auto edges = detectEdges(cluster);
    auto corners = detectCorners(cluster);
    auto symCenter = detectSymmetryCenter(cluster);
    
    // Create edge zones
    if (preset.edgeEmphasis) {
        for (const auto& edge : edges) {
            DecorationZone edgeZone;
            edgeZone.area = edge;
            edgeZone.decorationDensity = preset.densityCurve.getDensityAt(0.8f, true, false, false, false);
            edgeZone.zoneType = "edge";
            edgeZone.isEdge = true;
            zones.push_back(edgeZone);
        }
    }
    
    // Create corner zones
    for (const auto& corner : corners) {
        DecorationZone cornerZone;
        cornerZone.area = cocos2d::CCRect(corner.x - 30, corner.y - 30, 60, 60);
        cornerZone.decorationDensity = preset.densityCurve.getDensityAt(0.9f, true, true, false, false);
        cornerZone.zoneType = "corner";
        cornerZone.isCorner = true;
        zones.push_back(cornerZone);
    }
    
    // Create center zone
    if (symCenter.size.width > 0) {
        DecorationZone centerZone;
        centerZone.area = symCenter;
        centerZone.decorationDensity = preset.densityCurve.getDensityAt(0.5f, false, false, true, false);
        centerZone.zoneType = "center";
        centerZone.isCenter = true;
        zones.push_back(centerZone);
    }
    
    // Create fill zones (interior areas)
    if (!preset.antiClutterMode) {
        DecorationZone fillZone;
        fillZone.area = cocos2d::CCRect(bbox.origin.x + 40, bbox.origin.y + 40, 
                                        bbox.size.width - 80, bbox.size.height - 80);
        if (fillZone.area.size.width > 0 && fillZone.area.size.height > 0) {
            fillZone.decorationDensity = preset.densityCurve.getDensityAt(0.3f, false, false, false, false);
            fillZone.zoneType = "fill";
            zones.push_back(fillZone);
        }
    }
    
    return zones;
}

float GeometryAnalyzer::distance(const cocos2d::CCPoint& a, const cocos2d::CCPoint& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

bool GeometryAnalyzer::isLinearStructure(const std::vector<Ref<GameObject>>& objects) {
    if (objects.size() < 3) return false;
    
    auto bbox = computeBoundingBox(objects);
    float aspectRatio = bbox.size.width / bbox.size.height;
    
    return aspectRatio > 3.0f || aspectRatio < 0.33f;
}

bool GeometryAnalyzer::isVerticalStructure(const std::vector<Ref<GameObject>>& objects) {
    if (objects.size() < 3) return false;
    
    auto bbox = computeBoundingBox(objects);
    float aspectRatio = bbox.size.height / bbox.size.width;
    
    return aspectRatio > 2.0f;
}

bool GeometryAnalyzer::isEnclosedShape(const std::vector<Ref<GameObject>>& objects) {
    if (objects.size() < 4) return false;
    
    auto bbox = computeBoundingBox(objects);
    float aspectRatio = bbox.size.width / bbox.size.height;
    
    // Check if objects form a roughly rectangular boundary
    float perimeter = 2 * (bbox.size.width + bbox.size.height);
    float expectedObjectCount = perimeter / 30.0f; // One object every 30 units
    
    return std::abs((int)objects.size() - (int)expectedObjectCount) < objects.size() * 0.3f;
}

bool GeometryAnalyzer::isSparsePlacement(const std::vector<Ref<GameObject>>& objects) {
    if (objects.empty()) return true;
    
    auto bbox = computeBoundingBox(objects);
    float area = bbox.size.width * bbox.size.height;
    
    if (area <= 0) return true;
    
    float density = objects.size() / (area / 900.0f); // Objects per 30x30 area
    
    return density < 0.2f;
}

std::vector<std::vector<Ref<GameObject>>> GeometryAnalyzer::clusterObjectsByProximity(
    const std::vector<Ref<GameObject>>& objects,
    float threshold
) {
    std::vector<std::vector<Ref<GameObject>>> clusters;
    std::vector<bool> visited(objects.size(), false);

    // Breadth-first connected-component search: objects linked by a chain of
    // nearby neighbors stay in the same cluster.
    for (size_t i = 0; i < objects.size(); ++i) {
        if (visited[i] || !objects[i]) continue;

        std::vector<Ref<GameObject>> cluster;
        std::vector<size_t> pending{i};
        visited[i] = true;

        for (size_t cursor = 0; cursor < pending.size(); ++cursor) {
            const size_t current = pending[cursor];
            cluster.push_back(objects[current]);

            for (size_t j = 0; j < objects.size(); ++j) {
                if (visited[j] || !objects[j]) continue;
                if (distance(objects[current]->getPosition(), objects[j]->getPosition()) <= threshold) {
                    visited[j] = true;
                    pending.push_back(j);
                }
            }
        }

        if (!cluster.empty()) {
            clusters.push_back(std::move(cluster));
        }
    }

    return clusters;
}

}
