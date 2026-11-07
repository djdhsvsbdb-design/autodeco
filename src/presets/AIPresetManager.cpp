#include "AIPresetManager.hpp"
#include <Geode/Geode.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cctype>
#include <ctime>

using namespace geode::preload;

namespace AutoDecoratorAI {

namespace {
bool isSafePresetId(const std::string& id) {
    if (id.empty() || id.size() > 80) return false;
    for (unsigned char ch : id) {
        if (!std::isalnum(ch) && ch != '_' && ch != '-') return false;
    }
    return true;
}
}

AIPresetManager* AIPresetManager::s_instance = nullptr;

AIPresetManager::AIPresetManager() {
    m_presetDirectory = (geode::dirs::getModSaveDir() / "Alt3rDECO" / "presets").string();
}

AIPresetManager::~AIPresetManager() {}

AIPresetManager* AIPresetManager::get() {
    if (!s_instance) {
        s_instance = new AIPresetManager();
    }
    return s_instance;
}

void AIPresetManager::initialize() {
    // Create preset directory if it doesn't exist
    if (!std::filesystem::exists(m_presetDirectory)) {
        std::filesystem::create_directories(m_presetDirectory);
    }
    
    loadBuiltInPresets();
    loadCustomPresets();
}

void AIPresetManager::loadBuiltInPresets() {
    addPreset(createModernRedlinePreset());
    addPreset(createMinimalStructuralPreset());
    addPreset(createHighDetailDemonPreset());
    addPreset(createCleanArchitecturalPreset());
    addPreset(createRetroGridSymmetryPreset());
}

void AIPresetManager::loadCustomPresets() {
    if (!std::filesystem::exists(m_presetDirectory)) {
        return;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(m_presetDirectory)) {
        if (entry.path().extension() == ".json") {
            loadPresetFromFile(entry.path().string());
        }
    }
}

void AIPresetManager::saveCustomPresets() {
    for (auto& pair : m_presets) {
        // Only save custom presets (not built-in ones)
        if (pair.first.find("builtin_") == std::string::npos) {
            savePresetToFile(pair.second);
        }
    }
}

std::vector<AIPreset> AIPresetManager::getAllPresets() const {
    std::vector<AIPreset> presets;
    for (const auto& pair : m_presets) {
        presets.push_back(pair.second);
    }

    // Keep the default Modern Redline preset at the top, then use stable
    // alphabetical ordering instead of unordered_map iteration order.
    std::sort(presets.begin(), presets.end(), [](const AIPreset& a, const AIPreset& b) {
        if (a.id == "builtin_modern_redline") return b.id != "builtin_modern_redline";
        if (b.id == "builtin_modern_redline") return false;
        return a.name < b.name;
    });
    return presets;
}

AIPreset* AIPresetManager::getPresetById(const std::string& id) {
    auto it = m_presets.find(id);
    if (it != m_presets.end()) {
        return &it->second;
    }
    return nullptr;
}

AIPreset* AIPresetManager::getPresetByName(const std::string& name) {
    for (auto& pair : m_presets) {
        if (pair.second.name == name) {
            return &pair.second;
        }
    }
    return nullptr;
}

void AIPresetManager::addPreset(const AIPreset& preset) {
    m_presets[preset.id] = preset;
}

void AIPresetManager::removePreset(const std::string& id) {
    auto it = m_presets.find(id);
    if (it != m_presets.end()) {
        // Delete file if it's a custom preset
        if (id.find("builtin_") == std::string::npos && isSafePresetId(id)) {
            const auto filePath = getPresetFilePath(id);
            if (!filePath.empty() && std::filesystem::exists(filePath)) {
                std::filesystem::remove(filePath);
            }
        }
        m_presets.erase(it);
    }
}

void AIPresetManager::updatePreset(const AIPreset& preset) {
    m_presets[preset.id] = preset;
    if (preset.id.find("builtin_") == std::string::npos) {
        savePresetToFile(preset);
    }
}

AIPreset AIPresetManager::createCustomPreset(const std::string& name, const std::string& description) {
    AIPreset preset;
    preset.id = "custom_" + std::to_string(std::hash<std::string>{}(name + std::to_string(time(nullptr))));
    preset.name = name;
    preset.description = description;
    preset.styleBias = StyleBias::CleanModern;
    preset.symmetryPreference = 0.7f;
    preset.chaosTolerance = 0.3f;
    preset.clusteringThreshold = 0.5f;
    preset.microDetailLevel = 0.4f;
    preset.edgeEmphasis = true;
    preset.flowFollowing = true;
    preset.symmetryMirroring = true;
    preset.antiClutterMode = false;
    
    return preset;
}

AIPreset AIPresetManager::createModernRedlinePreset() {
    AIPreset preset;
    preset.id = "builtin_modern_redline";
    preset.name = "Modern Redline";
    preset.description = "Clean red/black/gray-ready geometric decoration. Uses curated non-gameplay object IDs and no glow objects.";
    preset.styleBias = StyleBias::CleanModern;
    preset.symmetryPreference = 0.6f;
    preset.chaosTolerance = 0.4f;
    preset.clusteringThreshold = 0.4f;
    preset.microDetailLevel = 0.6f;
    preset.edgeEmphasis = true;
    preset.flowFollowing = true;
    preset.symmetryMirroring = true;
    preset.antiClutterMode = false;
    
    preset.densityCurve.baseDensity = 0.6f;
    preset.densityCurve.edgeMultiplier = 1.8f;
    preset.densityCurve.cornerMultiplier = 2.2f;
    preset.densityCurve.centerMultiplier = 0.7f;
    preset.densityCurve.flowMultiplier = 1.5f;
    
    preset.styleWeights["contrast"] = 1.0f;
    preset.styleWeights["structure"] = 0.9f;
    preset.styleWeights["accent"] = 0.7f;
    
    // IDs verified against the 2.2081 object catalogue. These are decorative
    // rods, clouds, gears, bricks, wavy trims and pipe/highlight pieces.
    // No orb, portal, pad, trigger, hazard or glow object is included.
    preset.decorationPool = {
        {15, 0.5f, "detail", false, true},
        {16, 0.7f, "edge", false, false},
        {17, 0.5f, "detail", false, true},
        {48, 0.3f, "fill", false, false},
        {49, 0.3f, "fill", false, false},
        {85, 0.8f, "accent", false, false},
        {86, 0.7f, "accent", false, false},
        {87, 0.6f, "detail", false, true},
        {113, 0.8f, "fill", false, false},
        {114, 0.7f, "fill", false, false},
        {115, 0.6f, "detail", false, true},
        {157, 0.7f, "edge", false, false},
        {158, 0.4f, "edge", false, false},
        {159, 0.4f, "edge", false, false},
        {225, 0.7f, "edge", false, false},
        {226, 0.5f, "accent", false, false},
        {227, 0.7f, "detail", false, true},
        {228, 0.5f, "detail", false, true},
        {233, 0.8f, "fill", false, false},
        {234, 0.6f, "fill", false, false},
        {235, 0.5f, "detail", false, true},
        {245, 0.8f, "accent", false, false},
        {246, 0.5f, "detail", false, true},
        {277, 0.7f, "fill", false, false},
        {278, 0.5f, "detail", false, true}
    };
    
    return preset;
}

AIPreset AIPresetManager::createMinimalStructuralPreset() {
    AIPreset preset;
    preset.id = "builtin_minimal_structural";
    preset.name = "Minimal Structural Balance";
    preset.description = "Clean, symmetric decoration with low clutter and balanced placement";
    preset.styleBias = StyleBias::CleanModern;
    preset.symmetryPreference = 0.9f;
    preset.chaosTolerance = 0.1f;
    preset.clusteringThreshold = 0.7f;
    preset.microDetailLevel = 0.2f;
    preset.edgeEmphasis = true;
    preset.flowFollowing = false;
    preset.symmetryMirroring = true;
    preset.antiClutterMode = true;
    
    preset.densityCurve.baseDensity = 0.3f;
    preset.densityCurve.edgeMultiplier = 1.3f;
    preset.densityCurve.cornerMultiplier = 1.5f;
    preset.densityCurve.centerMultiplier = 0.5f;
    preset.densityCurve.flowMultiplier = 1.0f;
    
    preset.styleWeights["symmetry"] = 1.0f;
    preset.styleWeights["minimal"] = 0.9f;
    preset.styleWeights["balance"] = 0.8f;
    
    preset.decorationPool = {
        {15, 0.8f, "structure", false, false},
        {16, 0.7f, "accent", false, false},
        {17, 0.6f, "detail", false, true},
        {157, 0.5f, "edge", false, false},
        {227, 0.4f, "detail", false, true}
    };
    
    return preset;
}

AIPreset AIPresetManager::createHighDetailDemonPreset() {
    AIPreset preset;
    preset.id = "builtin_high_detail_demon";
    preset.name = "High Detail Demon Flow";
    preset.description = "High density spikes with chaotic micro detail for extreme levels";
    preset.styleBias = StyleBias::DemonExtreme;
    preset.symmetryPreference = 0.3f;
    preset.chaosTolerance = 0.8f;
    preset.clusteringThreshold = 0.2f;
    preset.microDetailLevel = 0.9f;
    preset.edgeEmphasis = true;
    preset.flowFollowing = true;
    preset.symmetryMirroring = false;
    preset.antiClutterMode = false;
    
    preset.densityCurve.baseDensity = 0.8f;
    preset.densityCurve.edgeMultiplier = 2.0f;
    preset.densityCurve.cornerMultiplier = 2.5f;
    preset.densityCurve.centerMultiplier = 0.9f;
    preset.densityCurve.flowMultiplier = 1.8f;
    
    preset.styleWeights["chaos"] = 1.0f;
    preset.styleWeights["density"] = 0.9f;
    preset.styleWeights["detail"] = 0.8f;
    
    preset.decorationPool = {
        {85, 1.0f, "accent", false, false},
        {86, 0.9f, "accent", false, false},
        {87, 0.8f, "detail", false, true},
        {180, 0.7f, "edge", false, false},
        {181, 0.6f, "detail", false, true},
        {182, 0.5f, "detail", false, true}
    };
    
    return preset;
}

AIPreset AIPresetManager::createCleanArchitecturalPreset() {
    AIPreset preset;
    preset.id = "builtin_clean_architectural";
    preset.name = "Clean Architectural Mode";
    preset.description = "Grid-based placement with strict spacing and architectural precision";
    preset.styleBias = StyleBias::Architectural;
    preset.symmetryPreference = 0.8f;
    preset.chaosTolerance = 0.2f;
    preset.clusteringThreshold = 0.6f;
    preset.microDetailLevel = 0.3f;
    preset.edgeEmphasis = true;
    preset.flowFollowing = false;
    preset.symmetryMirroring = true;
    preset.antiClutterMode = true;
    
    preset.densityCurve.baseDensity = 0.4f;
    preset.densityCurve.edgeMultiplier = 1.4f;
    preset.densityCurve.cornerMultiplier = 1.6f;
    preset.densityCurve.centerMultiplier = 0.6f;
    preset.densityCurve.flowMultiplier = 1.1f;
    
    preset.styleWeights["grid"] = 1.0f;
    preset.styleWeights["precision"] = 0.9f;
    preset.styleWeights["structure"] = 0.8f;
    
    preset.decorationPool = {
        {233, 0.9f, "structure", false, false},
        {234, 0.8f, "frame", false, false},
        {235, 0.7f, "detail", false, true},
        {245, 0.6f, "accent", false, false},
        {246, 0.5f, "detail", false, true}
    };
    
    return preset;
}

AIPreset AIPresetManager::createRetroGridSymmetryPreset() {
    AIPreset preset;
    preset.id = "builtin_retro_grid_symmetry";
    preset.name = "Retro Grid Symmetry";
    preset.description = "Pixel-style grid placement with strict spacing and retro aesthetics";
    preset.styleBias = StyleBias::RetroPixel;
    preset.symmetryPreference = 0.95f;
    preset.chaosTolerance = 0.05f;
    preset.clusteringThreshold = 0.8f;
    preset.microDetailLevel = 0.25f;
    preset.edgeEmphasis = true;
    preset.flowFollowing = false;
    preset.symmetryMirroring = true;
    preset.antiClutterMode = true;
    
    preset.densityCurve.baseDensity = 0.35f;
    preset.densityCurve.edgeMultiplier = 1.2f;
    preset.densityCurve.cornerMultiplier = 1.4f;
    preset.densityCurve.centerMultiplier = 0.4f;
    preset.densityCurve.flowMultiplier = 1.0f;
    
    preset.styleWeights["grid"] = 1.0f;
    preset.styleWeights["pixel"] = 0.9f;
    preset.styleWeights["retro"] = 0.8f;
    
    preset.decorationPool = {
        {50, 0.7f, "pixel", false, false},
        {51, 0.6f, "pixel", false, false},
        {52, 0.4f, "detail", false, true},
        {53, 0.5f, "accent", false, false},
        {54, 0.4f, "detail", false, true},
        {157, 0.5f, "edge", false, false}
    };
    
    return preset;
}

std::string AIPresetManager::getPresetFilePath(const std::string& presetId) {
    if (!isSafePresetId(presetId)) {
        return {};
    }
    return (std::filesystem::path(m_presetDirectory) / (presetId + ".json")).string();
}

bool AIPresetManager::loadPresetFromFile(const std::string& filePath) {
    try {
        std::ifstream file(filePath);
        if (!file) {
            return false;
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        auto parsed = matjson::parse(buffer.str());
        if (!parsed) {
            log::warn("Invalid preset JSON in {}: {}", filePath, parsed.unwrapErr());
            return false;
        }
        AIPreset preset = AIPreset::fromJson(parsed.unwrap());
        if (!isSafePresetId(preset.id)) {
            log::warn("Rejected preset with unsafe id from {}", filePath);
            return false;
        }
        addPreset(preset);
        return true;
    } catch (const std::exception& e) {
        log::warn("Failed to load preset from {}: {}", filePath, e.what());
        return false;
    }
}

bool AIPresetManager::savePresetToFile(const AIPreset& preset) {
    try {
        std::string filePath = getPresetFilePath(preset.id);
        if (filePath.empty()) {
            log::warn("Rejected preset with unsafe id: {}", preset.id);
            return false;
        }
        auto json = preset.toJson();
        std::ofstream file(filePath);
        file << json.dump(4);
        file.close();
        return true;
    } catch (const std::exception& e) {
        log::warn("Failed to save preset to {}: {}", getPresetFilePath(preset.id), e.what());
        return false;
    }
}

}
