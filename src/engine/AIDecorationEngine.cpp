#include "AIDecorationEngine.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <cmath>
#include <random>
#include <cstdlib>

using namespace geode::prelude;

namespace AutoDecoratorAI {

AIDecorationEngine* AIDecorationEngine::s_instance = nullptr;

AIDecorationEngine::AIDecorationEngine() 
    : m_geometryAnalyzer(nullptr)
    , m_patternDetector(nullptr)
    , m_randomGenerator(std::random_device{}()) {}

AIDecorationEngine::~AIDecorationEngine() {}

AIDecorationEngine* AIDecorationEngine::get() {
    if (!s_instance) {
        s_instance = new AIDecorationEngine();
    }
    return s_instance;
}

void AIDecorationEngine::initialize() {
    m_geometryAnalyzer = GeometryAnalyzer::get();
    m_patternDetector = PatternDetector::get();
}

void AIDecorationEngine::setProgressCallback(std::function<void(const std::string&, float)> callback) {
    m_progressCallback = callback;
}

void AIDecorationEngine::reportProgress(const std::string& status, float progress) {
    if (m_progressCallback) {
        m_progressCallback(status, progress);
    }
}

std::vector<DecorationPlacement> AIDecorationEngine::generateDecorations(
    const std::vector<Ref<GameObject>>& selectedObjects,
    const AIPreset& preset,
    float densityOverride,
    float chaosOverride,
    float symmetryOverride,
    float detailOverride
) {
    std::vector<DecorationPlacement> allPlacements;
    
    if (selectedObjects.empty()) {
        reportProgress("No objects selected", 1.0f);
        return allPlacements;
    }
    
    reportProgress("Analyzing geometry...", 0.1f);
    
    // Apply overrides if provided
    // Keep UI overrides in their documented 0..1 range.
    const float densityScale = std::clamp(densityOverride >= 0 ? densityOverride : 1.0f, 0.0f, 1.0f);
    const float chaosScale = std::clamp(chaosOverride >= 0 ? chaosOverride : preset.chaosTolerance, 0.0f, 1.0f);
    const float symmetryScale = std::clamp(symmetryOverride >= 0 ? symmetryOverride : preset.symmetryPreference, 0.0f, 1.0f);
    const float detailScale = std::clamp(detailOverride >= 0 ? detailOverride : preset.microDetailLevel, 0.0f, 1.0f);
    
    // Analyze geometry
    auto clusters = m_geometryAnalyzer->analyzeSelection(selectedObjects);
    
    reportProgress("Detecting patterns...", 0.2f);
    
    // Detect patterns for each cluster
    std::vector<PatternDetectionResult> patternResults;
    for (const auto& cluster : clusters) {
        patternResults.push_back(m_patternDetector->detectPatterns(cluster.objects));
    }
    
    reportProgress("Generating decorations...", 0.3f);
    
    // Generate decorations for each cluster
    for (size_t i = 0; i < clusters.size(); ++i) {
        float clusterProgress = 0.3f + (0.6f * (i + 1) / clusters.size());
        reportProgress("Processing cluster " + std::to_string(i + 1) + "...", clusterProgress);
        
        auto clusterPlacements = generateDecorationsForCluster(
            clusters[i],
            patternResults[i],
            preset,
            densityScale,
            chaosScale,
            symmetryScale,
            detailScale
        );
        
        allPlacements.insert(allPlacements.end(), clusterPlacements.begin(), clusterPlacements.end());
    }
    
    reportProgress("Validating placements...", 0.9f);
    
    // Validate and filter placements
    std::vector<DecorationPlacement> validatedPlacements;
    for (const auto& placement : allPlacements) {
        // A placement belongs to one cluster, not every cluster. The old
        // all-clusters test rejected nearly all decorations in multi-cluster selections.
        bool insideSelection = false;
        for (const auto& cluster : clusters) {
            if (cluster.boundingBox.containsPoint(placement.position)) {
                insideSelection = true;
                break;
            }
        }

        if (insideSelection &&
            validatePlacement(placement, GeometryCluster{}, validatedPlacements)) {
            validatedPlacements.push_back(placement);
        }
    }
    
    reportProgress("Decoration generation complete", 1.0f);
    
    return validatedPlacements;
}

std::vector<DecorationPlacement> AIDecorationEngine::generateDecorationsForCluster(
    const GeometryCluster& cluster,
    const PatternDetectionResult& patterns,
    const AIPreset& preset,
    float densityScale,
    float chaosScale,
    float symmetryScale,
    float detailScale
) {
    std::vector<DecorationPlacement> placements;
    
    // Generate decoration zones and apply the density slider consistently to
    // edge, fill, corner and micro-detail placement.
    auto zones = m_geometryAnalyzer->generateDecorationZones(cluster, preset);
    for (auto& zone : zones) {
        zone.decorationDensity = std::clamp(zone.decorationDensity * densityScale, 0.0f, 1.0f);
    }
    
    // Generate edge decorations
    if (preset.edgeEmphasis) {
        auto edgeDecorations = generateEdgeDecorations(cluster, zones, preset, chaosScale);
        placements.insert(placements.end(), edgeDecorations.begin(), edgeDecorations.end());
    }
    
    // Generate fill decorations
    if (!preset.antiClutterMode) {
        auto fillDecorations = generateFillDecorations(cluster, zones, preset, chaosScale, detailScale);
        placements.insert(placements.end(), fillDecorations.begin(), fillDecorations.end());
    }
    
    // Generate accent decorations
    auto accentDecorations = generateAccentDecorations(cluster, patterns, preset, symmetryScale);
    placements.insert(placements.end(), accentDecorations.begin(), accentDecorations.end());
    
    // Generate micro details
    if (detailScale > 0.3f) {
        auto microDetails = generateMicroDetails(cluster, zones, preset, detailScale, chaosScale);
        placements.insert(placements.end(), microDetails.begin(), microDetails.end());
    }
    
    // Apply symmetry mirroring
    if (preset.symmetryMirroring && patterns.symmetryStrength > 0.5f) {
        applySymmetryMirroring(placements, cluster, patterns, symmetryScale);
    }
    
    // Apply flow following
    if (preset.flowFollowing) {
        applyFlowFollowing(placements, patterns, preset);
    }
    
    // Apply anti-clutter
    if (preset.antiClutterMode) {
        applyAntiClutter(placements, cluster);
    }
    
    return placements;
}

std::vector<DecorationPlacement> AIDecorationEngine::generateEdgeDecorations(
    const GeometryCluster& cluster,
    const std::vector<DecorationZone>& zones,
    const AIPreset& preset,
    float chaosScale
) {
    std::vector<DecorationPlacement> placements;
    
    for (const auto& zone : zones) {
        if (zone.zoneType != "edge" || zone.decorationDensity <= 0.0f) continue;
        
        int decorationCount = static_cast<int>(zone.area.size.width * zone.area.size.height * zone.decorationDensity * 0.01f * (1.0f + chaosScale * 0.5f));
        decorationCount = std::clamp(decorationCount, 0, 15);
        if (decorationCount == 0) continue;
        
        for (int i = 0; i < decorationCount; ++i) {
            DecorationPlacement placement;
            placement.position = generatePositionInZone(zone, cluster, chaosScale);
            placement.objectID = selectDecorationObject(preset, "edge", chaosScale).objectID;
            placement.rotation = calculateImperfection(chaosScale * 0.2f);
            placement.scale = 1.0f + calculateImperfection(chaosScale * 0.1f);
            placements.push_back(placement);
        }
    }
    
    return placements;
}

std::vector<DecorationPlacement> AIDecorationEngine::generateFillDecorations(
    const GeometryCluster& cluster,
    const std::vector<DecorationZone>& zones,
    const AIPreset& preset,
    float chaosScale,
    float detailScale
) {
    std::vector<DecorationPlacement> placements;
    
    for (const auto& zone : zones) {
        if (zone.zoneType != "fill" || zone.decorationDensity <= 0.0f) continue;
        
        int decorationCount = static_cast<int>(zone.area.size.width * zone.area.size.height * zone.decorationDensity * 0.005f * detailScale);
        decorationCount = std::clamp(decorationCount, 0, 8);
        if (decorationCount == 0) continue;
        
        for (int i = 0; i < decorationCount; ++i) {
            DecorationPlacement placement;
            placement.position = generatePositionInZone(zone, cluster, chaosScale);
            placement.objectID = selectDecorationObject(preset, "fill", chaosScale).objectID;
            placement.rotation = calculateImperfection(chaosScale * 0.3f);
            placement.scale = 0.8f + calculateImperfection(chaosScale * 0.15f);
            placements.push_back(placement);
        }
    }
    
    return placements;
}

std::vector<DecorationPlacement> AIDecorationEngine::generateAccentDecorations(
    const GeometryCluster& cluster,
    const PatternDetectionResult& patterns,
    const AIPreset& preset,
    float symmetryScale
) {
    std::vector<DecorationPlacement> placements;
    
    // Add accents at corners
    auto corners = m_geometryAnalyzer->detectCorners(cluster);
    for (const auto& corner : corners) {
        DecorationPlacement placement;
        placement.position = applyImperfection(corner, 0.1f);
        placement.objectID = selectDecorationObject(preset, "accent", 0.2f).objectID;
        placement.rotation = 0.0f;
        placement.scale = 1.0f;
        placements.push_back(placement);
    }
    
    // Add accents at symmetry center if strong symmetry
    if (patterns.symmetryStrength > 0.7f) {
        auto symCenter = m_geometryAnalyzer->detectSymmetryCenter(cluster);
        if (symCenter.size.width > 0) {
            DecorationPlacement placement;
            placement.position = cocos2d::CCPoint(symCenter.origin.x + symCenter.size.width / 2, 
                                                symCenter.origin.y + symCenter.size.height / 2);
            placement.objectID = selectDecorationObject(preset, "accent", 0.1f).objectID;
            placement.rotation = 0.0f;
            placement.scale = 1.2f;
            placements.push_back(placement);
        }
    }
    
    return placements;
}

std::vector<DecorationPlacement> AIDecorationEngine::generateMicroDetails(
    const GeometryCluster& cluster,
    const std::vector<DecorationZone>& zones,
    const AIPreset& preset,
    float detailScale,
    float chaosScale
) {
    std::vector<DecorationPlacement> placements;
    
    for (const auto& zone : zones) {
        if (zone.zoneType != "edge" && zone.zoneType != "corner") continue;
        
        int detailCount = static_cast<int>(zone.decorationDensity * detailScale * 3.0f);
        detailCount = std::clamp(detailCount, 0, 5);
        if (detailCount == 0) continue;
        
        for (int i = 0; i < detailCount; ++i) {
            DecorationPlacement placement;
            placement.position = generatePositionInZone(zone, cluster, chaosScale);
            placement.objectID = selectDecorationObject(preset, "detail", chaosScale).objectID;
            placement.rotation = calculateImperfection(chaosScale * 0.5f);
            placement.scale = 0.5f + calculateImperfection(chaosScale * 0.2f);
            placements.push_back(placement);
        }
    }
    
    return placements;
}

DecorationPlacement AIDecorationEngine::selectDecorationObject(
    const AIPreset& preset,
    const std::string& category,
    float chaosScale
) {
    DecorationPlacement placement;
    
    // Filter objects by category
    std::vector<DecorationObject> filtered;
    for (const auto& obj : preset.decorationPool) {
        if (category.empty() || obj.category == category) {
            filtered.push_back(obj);
        }
    }
    
    if (filtered.empty()) {
        placement.objectID = 100; // Fallback
        return placement;
    }
    
    // Select weighted random object
    auto selected = selectWeightedObject(filtered, category);
    placement.objectID = selected.objectID;
    
    return placement;
}

cocos2d::CCPoint AIDecorationEngine::generatePositionInZone(
    const DecorationZone& zone,
    const GeometryCluster& cluster,
    float chaosScale
) {
    std::uniform_real_distribution<float> xDist(zone.area.origin.x, zone.area.origin.x + zone.area.size.width);
    std::uniform_real_distribution<float> yDist(zone.area.origin.y, zone.area.origin.y + zone.area.size.height);
    
    cocos2d::CCPoint position(xDist(m_randomGenerator), yDist(m_randomGenerator));
    
    // Apply imperfection for human-like variation
    position = applyImperfection(position, chaosScale * 0.3f);
    
    return position;
}

void AIDecorationEngine::applySymmetryMirroring(
    std::vector<DecorationPlacement>& placements,
    const GeometryCluster& cluster,
    const PatternDetectionResult& patterns,
    float symmetryScale
) {
    if (patterns.symmetryStrength < 0.5f || symmetryScale < 0.3f) return;
    
    std::vector<DecorationPlacement> mirroredPlacements;
    auto bbox = cluster.boundingBox;
    
    for (const auto& placement : placements) {
        if (patterns.symmetryType == SymmetryType::Horizontal) {
            float axis = bbox.origin.y + bbox.size.height / 2;
            DecorationPlacement mirrored = placement;
            mirrored.position.y = 2 * axis - placement.position.y;
            mirroredPlacements.push_back(mirrored);
        } else if (patterns.symmetryType == SymmetryType::Vertical) {
            float axis = bbox.origin.x + bbox.size.width / 2;
            DecorationPlacement mirrored = placement;
            mirrored.position.x = 2 * axis - placement.position.x;
            mirroredPlacements.push_back(mirrored);
        }
    }
    
    // Add mirrored placements with probability based on symmetry strength
    std::uniform_real_distribution<float> probDist(0.0f, 1.0f);
    for (const auto& mirrored : mirroredPlacements) {
        if (probDist(m_randomGenerator) < patterns.symmetryStrength * symmetryScale) {
            placements.push_back(mirrored);
        }
    }
}

void AIDecorationEngine::applyFlowFollowing(
    std::vector<DecorationPlacement>& placements,
    const PatternDetectionResult& patterns,
    const AIPreset& preset
) {
    if (!preset.flowFollowing) return;
    
    // Adjust positions slightly along flow direction using the engine RNG.
    std::uniform_real_distribution<float> offsetDist(5.0f, 14.0f);
    for (auto& placement : placements) {
        float offset = offsetDist(m_randomGenerator);
        placement.position.x += patterns.flowDirection.x * offset;
        placement.position.y += patterns.flowDirection.y * offset;
    }
}

void AIDecorationEngine::applyAntiClutter(
    std::vector<DecorationPlacement>& placements,
    const GeometryCluster& cluster
) {
    const float minDistance = 30.0f;
    
    std::vector<DecorationPlacement> filtered;
    for (const auto& placement : placements) {
        bool tooClose = false;
        for (const auto& existing : filtered) {
            float dx = placement.position.x - existing.position.x;
            float dy = placement.position.y - existing.position.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            if (distance < minDistance) {
                tooClose = true;
                break;
            }
        }
        if (!tooClose) {
            filtered.push_back(placement);
        }
    }
    
    placements = filtered;
}

bool AIDecorationEngine::validatePlacement(
    const DecorationPlacement& placement,
    const GeometryCluster& cluster,
    const std::vector<DecorationPlacement>& existingPlacements
) {
    // Bounds are checked against the union of selected clusters by the caller.
    (void)cluster;

    // Check for overlap with existing placements
    const float minDistance = 20.0f;
    for (const auto& existing : existingPlacements) {
        float dx = placement.position.x - existing.position.x;
        float dy = placement.position.y - existing.position.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        if (distance < minDistance) {
            return false;
        }
    }
    
    return true;
}

float AIDecorationEngine::calculateImperfection(float chaosScale) {
    std::uniform_real_distribution<float> dist(-chaosScale, chaosScale);
    return dist(m_randomGenerator) * 15.0f; // Max 15 degrees of rotation
}

cocos2d::CCPoint AIDecorationEngine::applyImperfection(cocos2d::CCPoint position, float chaosScale) {
    std::uniform_real_distribution<float> dist(-chaosScale, chaosScale);
    float offsetX = dist(m_randomGenerator) * 10.0f;
    float offsetY = dist(m_randomGenerator) * 10.0f;
    return cocos2d::CCPoint(position.x + offsetX, position.y + offsetY);
}

DecorationObject AIDecorationEngine::selectWeightedObject(const std::vector<DecorationObject>& pool, const std::string& category) {
    if (pool.empty()) {
        return DecorationObject{100, 1.0f, category, false, false};
    }
    
    // Calculate total weight
    float totalWeight = 0.0f;
    for (const auto& obj : pool) {
        totalWeight += obj.weight;
    }
    
    // Select based on weight
    std::uniform_real_distribution<float> dist(0.0f, totalWeight);
    float random = dist(m_randomGenerator);
    
    float currentWeight = 0.0f;
    for (const auto& obj : pool) {
        currentWeight += obj.weight;
        if (random <= currentWeight) {
            return obj;
        }
    }
    
    return pool.back();
}

void AIDecorationEngine::applyDecorationsToLevel(const std::vector<DecorationPlacement>& placements, EditorUI* editorUI) {
    if (!editorUI || placements.empty()) {
        return;
    }
    
    auto editorLayer = editorUI->getEditorLayer();
    if (!editorLayer) {
        return;
    }
    
    // Bound one apply operation so a malformed preset cannot create an
    // unreasonably large number of editor objects in a single action.
    const size_t safePlacementLimit = 1500;
    const size_t placementCount = std::min(placements.size(), safePlacementLimit);
    for (size_t j = 0; j < placementCount; ++j) {
            const auto& placement = placements[j];
            
            // Create the object
            auto obj = GameObject::create(placement.objectID);
            if (obj) {
                obj->setPosition(placement.position);
                obj->setRotation(placement.rotation);
                obj->setScale(placement.scale);
                
                // Add to level
                editorLayer->addToObjects(obj);
            }
    }
    
    // Refresh editor
    editorUI->reloadUndo();
    editorLayer->updateObjects();
}

}
