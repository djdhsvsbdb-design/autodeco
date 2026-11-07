#pragma once

#include <Geode/Geode.hpp>
#include "AIPresetData.hpp"
#include <unordered_map>
#include <string>

namespace AutoDecoratorAI {

class AIPresetManager {
public:
    static AIPresetManager* get();
    
    void initialize();
    void loadBuiltInPresets();
    void loadCustomPresets();
    void saveCustomPresets();
    
    std::vector<AIPreset> getAllPresets() const;
    AIPreset* getPresetById(const std::string& id);
    AIPreset* getPresetByName(const std::string& name);
    
    void addPreset(const AIPreset& preset);
    void removePreset(const std::string& id);
    void updatePreset(const AIPreset& preset);
    
    AIPreset createCustomPreset(const std::string& name, const std::string& description);
    
private:
    AIPresetManager();
    ~AIPresetManager();
    
    static AIPresetManager* s_instance;
    
    std::unordered_map<std::string, AIPreset> m_presets;
    std::string m_presetDirectory;
    
    AIPreset createModernRedlinePreset();
    AIPreset createMinimalStructuralPreset();
    AIPreset createHighDetailDemonPreset();
    AIPreset createCleanArchitecturalPreset();
    AIPreset createRetroGridSymmetryPreset();
    
    std::string getPresetFilePath(const std::string& presetId);
    bool loadPresetFromFile(const std::string& filePath);
    bool savePresetToFile(const AIPreset& preset);
};

}
