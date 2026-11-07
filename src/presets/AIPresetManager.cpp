#include "AIPresetManager.hpp"
#include <Geode/Geode.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cctype>

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
    addPreset(createAdaptiveNeonPreset());
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

AIPreset AIPresetManager::createAdaptiveNeonPreset() {
    AIPreset preset;
    preset.id = "builtin_adaptive_neon";
    preset.name = "Adaptive Neon Intelligence";
    preset.description = "High contrast outlines with layered lighting objects and glow effects";
    preset.styleBias = StyleBias::NeonGlow;
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
    
    preset.styleWeights["glow"] = 1.0f;
    preset.styleWeights["contrast"] = 0.9f;
    preset.styleWeights["layering"] = 0.8f;
    
    // Neon decoration objects
    preset.decorationPool = {
        {749, 1.0f, "glow", true, false},   // Glow orb
        {750, 0.9f, "glow", true, false},   // Small glow
        {751, 0.8f, "outline", false, false}, // Neon line
        {752, 0.7f, "detail", false, true},  // Micro particle
        {753, 0.6f, "accent", true, false}  // Neon accent
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
        {100, 0.8f, "structure", false, false}, // Basic block
        {101, 0.7f, "accent", false, false},    // Small accent
        {102, 0.6f, "detail", false, true}     // Minimal detail
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
        {45, 1.0f, "spike", false, false},    // Spike
        {46, 0.9f, "spike", false, false},    // Double spike
        {47, 0.8f, "hazard", false, false},   // Hazard
        {48, 0.7f, "detail", false, true},    // Micro detail
        {49, 0.6f, "accent", false, false}    // Chaotic accent
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
        {200, 0.9f, "structure", false, false}, // Architectural block
        {201, 0.8f, "frame", false, false},     // Frame element
        {202, 0.7f, "detail", false, true},    // Architectural detail
        {203, 0.6f, "accent", false, false}     // Clean accent
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
        {300, 0.9f, "pixel", false, false},   // Pixel block
        {301, 0.8f, "pixel", false, false},   // Small pixel
        {302, 0.7f, "detail", false, true},    // Pixel detail
        {303, 0.6f, "accent", false, false}    // Retro accent
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
        auto json = matjson::parse(buffer.str());
        AIPreset preset = AIPreset::fromJson(json);
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
