#include "PatternDetector.hpp"
#include "GeometryAnalyzer.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>
#include <map>

using namespace geode::prelude;

namespace AutoDecoratorAI {

PatternDetector* PatternDetector::s_instance = nullptr;

PatternDetector::PatternDetector() {}

PatternDetector::~PatternDetector() {}

PatternDetector* PatternDetector::get() {
    if (!s_instance) {
        s_instance = new PatternDetector();
    }
    return s_instance;
}

PatternDetectionResult PatternDetector::detectPatterns(const std::vector<Ref<GameObject>>& objects) {
    PatternDetectionResult result;
    
    if (objects.empty()) {
        return result;
    }
    
    result.symmetryType = detectSymmetry(objects);
    result.symmetryStrength = calculateSymmetryStrength(objects, result.symmetryType);
    
    result.hasRepetition = detectRepetition(objects, result.repetitionPeriod);
    result.flowDirection = detectFlowDirection(objects);
    
    result.densityImbalance = calculateDensityImbalance(objects);
    result.emptyZones = findEmptyZones(objects);
    result.denseZones = findDenseZones(objects);
    
    return result;
}

SymmetryType PatternDetector::detectSymmetry(const std::vector<Ref<GameObject>>& objects) {
    if (objects.size() < 4) {
        return SymmetryType::None;
    }
    
    float hAxis = calculateAxisPosition(objects, SymmetryType::Horizontal);
    float vAxis = calculateAxisPosition(objects, SymmetryType::Vertical);
    
    int hSymmetricPairs = 0;
    int vSymmetricPairs = 0;
    int totalPairs = 0;
    
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            totalPairs++;
            
            if (isSymmetricPair(objects[i], objects[j], SymmetryType::Horizontal, hAxis)) {
                hSymmetricPairs++;
            }
            if (isSymmetricPair(objects[i], objects[j], SymmetryType::Vertical, vAxis)) {
                vSymmetricPairs++;
            }
        }
    }
    
    float hRatio = totalPairs > 0 ? (float)hSymmetricPairs / totalPairs : 0.0f;
    float vRatio = totalPairs > 0 ? (float)vSymmetricPairs / totalPairs : 0.0f;
    
    if (hRatio > 0.6f && vRatio > 0.6f) {
        return SymmetryType::Bilateral;
    } else if (hRatio > 0.6f) {
        return SymmetryType::Horizontal;
    } else if (vRatio > 0.6f) {
        return SymmetryType::Vertical;
    }
    
    // Check for radial symmetry (objects arranged in circle)
    auto analyzer = GeometryAnalyzer::get();
    auto centroid = analyzer->computeCentroid(objects);
    
    std::vector<float> angles;
    for (const auto& obj : objects) {
        float dx = obj->getPositionX() - centroid.x;
        float dy = obj->getPositionY() - centroid.y;
        float angle = std::atan2(dy, dx);
        angles.push_back(angle);
    }
    
    std::sort(angles.begin(), angles.end());
    
    // Check if angles are evenly distributed
    float avgAngleDiff = 0.0f;
    for (size_t i = 1; i < angles.size(); ++i) {
        avgAngleDiff += angles[i] - angles[i-1];
    }
    avgAngleDiff /= (angles.size() - 1);
    
    constexpr float twoPi = 6.28318530718f;
    float expectedDiff = twoPi / static_cast<float>(objects.size());
    if (std::abs(avgAngleDiff - expectedDiff) < 0.3f) {
        return SymmetryType::Radial;
    }
    
    return SymmetryType::None;
}

float PatternDetector::calculateSymmetryStrength(const std::vector<Ref<GameObject>>& objects, SymmetryType type) {
    if (type == SymmetryType::None || objects.size() < 2) {
        return 0.0f;
    }
    
    float axis = calculateAxisPosition(objects, type);
    int symmetricPairs = 0;
    int totalPairs = 0;
    
    for (size_t i = 0; i < objects.size(); ++i) {
        for (size_t j = i + 1; j < objects.size(); ++j) {
            totalPairs++;
            if (isSymmetricPair(objects[i], objects[j], type, axis)) {
                symmetricPairs++;
            }
        }
    }
    
    return totalPairs > 0 ? (float)symmetricPairs / totalPairs : 0.0f;
}

bool PatternDetector::detectRepetition(const std::vector<Ref<GameObject>>& objects, float& period) {
    if (objects.size() < 6) {
        period = 0.0f;
        return false;
    }
    
    // Sort objects by X position
    std::vector<Ref<GameObject>> sorted = objects;
    std::sort(sorted.begin(), sorted.end(), [](const Ref<GameObject>& a, const Ref<GameObject>& b) {
        return a->getPositionX() < b->getPositionX();
    });
    
    // Calculate distances between consecutive objects
    std::vector<float> distances;
    for (size_t i = 1; i < sorted.size(); ++i) {
        float dist = sorted[i]->getPositionX() - sorted[i-1]->getPositionX();
        if (dist > 0) {
            distances.push_back(dist);
        }
    }
    
    if (distances.empty()) {
        period = 0.0f;
        return false;
    }
    
    // Find most common distance
    std::map<float, int> distanceCounts;
    for (float dist : distances) {
        // Round to nearest 5 units to account for small variations
        float rounded = std::round(dist / 5.0f) * 5.0f;
        distanceCounts[rounded]++;
    }
    
    float mostCommonDist = 0.0f;
    int maxCount = 0;
    for (const auto& pair : distanceCounts) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            mostCommonDist = pair.first;
        }
    }
    
    // Check if this distance repeats consistently
    int consistentCount = 0;
    for (float dist : distances) {
        if (std::abs(dist - mostCommonDist) < 10.0f) {
            consistentCount++;
        }
    }
    
    float consistency = (float)consistentCount / distances.size();
    
    if (consistency > 0.6f && mostCommonDist > 20.0f) {
        period = mostCommonDist;
        return true;
    }
    
    period = 0.0f;
    return false;
}

cocos2d::CCPoint PatternDetector::detectFlowDirection(const std::vector<Ref<GameObject>>& objects) {
    if (objects.size() < 2) {
        return cocos2d::CCPoint(1, 0); // Default rightward
    }
    
    auto analyzer = GeometryAnalyzer::get();
    auto bbox = analyzer->computeBoundingBox(objects);
    
    // Calculate overall direction from first to last object
    std::vector<Ref<GameObject>> sorted = objects;
    std::sort(sorted.begin(), sorted.end(), [](const Ref<GameObject>& a, const Ref<GameObject>& b) {
        return a->getPositionX() < b->getPositionX();
    });
    
    float dx = sorted.back()->getPositionX() - sorted.front()->getPositionX();
    float dy = sorted.back()->getPositionY() - sorted.front()->getPositionY();
    
    float length = std::sqrt(dx * dx + dy * dy);
    if (length > 0) {
        return cocos2d::CCPoint(dx / length, dy / length);
    }
    
    // Fallback: use bounding box aspect ratio
    if (bbox.size.width > bbox.size.height) {
        return cocos2d::CCPoint(1, 0);
    } else {
        return cocos2d::CCPoint(0, 1);
    }
}

float PatternDetector::calculateDensityImbalance(const std::vector<Ref<GameObject>>& objects) {
    if (objects.size() < 4) {
        return 0.0f;
    }
    
    auto analyzer = GeometryAnalyzer::get();
    auto bbox = analyzer->computeBoundingBox(objects);
    
    // Divide bounding box into quadrants
    float midX = bbox.origin.x + bbox.size.width / 2;
    float midY = bbox.origin.y + bbox.size.height / 2;
    
    int q1 = 0, q2 = 0, q3 = 0, q4 = 0;
    
    for (const auto& obj : objects) {
        float x = obj->getPositionX();
        float y = obj->getPositionY();
        
        if (x >= midX && y >= midY) q1++;
        else if (x < midX && y >= midY) q2++;
        else if (x < midX && y < midY) q3++;
        else q4++;
    }
    
    int maxCount = std::max({q1, q2, q3, q4});
    int minCount = std::min({q1, q2, q3, q4});
    
    if (maxCount == 0) return 0.0f;
    
    return 1.0f - ((float)minCount / maxCount);
}

std::vector<cocos2d::CCRect> PatternDetector::findEmptyZones(const std::vector<Ref<GameObject>>& objects) {
    std::vector<cocos2d::CCRect> emptyZones;
    
    if (objects.size() < 2) {
        return emptyZones;
    }
    
    auto analyzer = GeometryAnalyzer::get();
    auto bbox = analyzer->computeBoundingBox(objects);
    
    // Create a grid over the bounding box
    const int gridSize = 5;
    float cellWidth = bbox.size.width / gridSize;
    float cellHeight = bbox.size.height / gridSize;
    
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            cocos2d::CCRect cell(
                bbox.origin.x + i * cellWidth,
                bbox.origin.y + j * cellHeight,
                cellWidth,
                cellHeight
            );
            
            // Count objects in this cell
            int count = 0;
            for (const auto& obj : objects) {
                if (cell.containsPoint(obj->getPosition())) {
                    count++;
                }
            }
            
            // If cell is empty or very sparse, mark as empty zone
            if (count == 0) {
                emptyZones.push_back(cell);
            }
        }
    }
    
    return emptyZones;
}

std::vector<cocos2d::CCRect> PatternDetector::findDenseZones(const std::vector<Ref<GameObject>>& objects) {
    std::vector<cocos2d::CCRect> denseZones;
    
    if (objects.size() < 2) {
        return denseZones;
    }
    
    auto analyzer = GeometryAnalyzer::get();
    auto bbox = analyzer->computeBoundingBox(objects);
    
    // Create a grid over the bounding box
    const int gridSize = 5;
    float cellWidth = bbox.size.width / gridSize;
    float cellHeight = bbox.size.height / gridSize;
    
    float avgObjectsPerCell = (float)objects.size() / (gridSize * gridSize);
    
    for (int i = 0; i < gridSize; ++i) {
        for (int j = 0; j < gridSize; ++j) {
            cocos2d::CCRect cell(
                bbox.origin.x + i * cellWidth,
                bbox.origin.y + j * cellHeight,
                cellWidth,
                cellHeight
            );
            
            // Count objects in this cell
            int count = 0;
            for (const auto& obj : objects) {
                if (cell.containsPoint(obj->getPosition())) {
                    count++;
                }
            }
            
            // If cell has significantly more objects than average, mark as dense
            if (count > avgObjectsPerCell * 1.5f) {
                denseZones.push_back(cell);
            }
        }
    }
    
    return denseZones;
}

bool PatternDetector::isSymmetricPair(const Ref<GameObject>& a, const Ref<GameObject>& b, SymmetryType type, float axis) {
    float tolerance = 15.0f; // Allow 15 units of variation
    
    if (type == SymmetryType::Horizontal) {
        // Reflect across horizontal line y = axis
        float reflectedY = 2 * axis - a->getPositionY();
        return std::abs(reflectedY - b->getPositionY()) < tolerance && 
               std::abs(a->getPositionX() - b->getPositionX()) < tolerance;
    } else if (type == SymmetryType::Vertical) {
        // Reflect across vertical line x = axis
        float reflectedX = 2 * axis - a->getPositionX();
        return std::abs(reflectedX - b->getPositionX()) < tolerance && 
               std::abs(a->getPositionY() - b->getPositionY()) < tolerance;
    }
    
    return false;
}

float PatternDetector::calculateAxisPosition(const std::vector<Ref<GameObject>>& objects, SymmetryType type) {
    if (objects.empty()) {
        return 0.0f;
    }
    
    auto analyzer = GeometryAnalyzer::get();
    auto bbox = analyzer->computeBoundingBox(objects);
    
    if (type == SymmetryType::Horizontal) {
        return bbox.origin.y + bbox.size.height / 2;
    } else if (type == SymmetryType::Vertical) {
        return bbox.origin.x + bbox.size.width / 2;
    }
    
    return 0.0f;
}

}
