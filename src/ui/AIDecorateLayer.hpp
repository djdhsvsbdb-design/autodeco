#pragma once

#include <Geode/Geode.hpp>
#include "../presets/AIPresetData.hpp"
#include "../engine/AIDecorationEngine.hpp"
#include <vector>

using namespace geode::prelude;

namespace AutoDecoratorAI {

class AIDecorateLayer : public cocos2d::CCLayer {
public:
    static AIDecorateLayer* create(EditorUI* editorUI);
    
    void updatePresetDropdown();
    void updateProgressDisplay(const std::string& status, float progress);
    
private:
    bool init(EditorUI* editorUI);
    
    EditorUI* m_editorUI;
    AIDecorationEngine* m_decorationEngine;
    
    // UI Elements
    CCMenuItemSpriteExtra* m_toggleButton;
    CCMenu* m_mainMenu;
    CCNode* m_overlayPanel;
    
    CCMenuItemToggler* m_edgeEmphasisToggle;
    CCMenuItemToggler* m_flowFollowingToggle;
    CCMenuItemToggler* m_symmetryMirroringToggle;
    CCMenuItemToggler* m_antiClutterToggle;
    
    Slider* m_densitySlider;
    Slider* m_chaosSlider;
    Slider* m_symmetrySlider;
    Slider* m_detailSlider;
    
    CCLabelBMFont* m_presetLabel;
    CCMenu* m_presetMenu;
    CCMenuItemSpriteExtra* m_analyzeButton;
    CCMenuItemSpriteExtra* m_applyButton;
    CCMenuItemSpriteExtra* m_savePresetButton;
    
    CCLabelBMFont* m_progressLabel;
    CCProgressTimer* m_progressBar;
    geode::TextInput* m_presetNameInput = nullptr;
    
    // State
    bool m_isVisible;
    AIPreset* m_currentPreset;
    std::vector<AIPreset> m_availablePresets;
    std::vector<DecorationPlacement> m_previewPlacements;
    std::vector<cocos2d::CCPoint> m_previewSelectionPositions;
    bool m_hasPreview = false;
    
    // Callbacks
    void onToggle(CCObject* sender);
    void onPresetChanged(CCObject* sender);
    void onAnalyze(CCObject* sender);
    void onApply(CCObject* sender);
    void onSavePreset(CCObject* sender);
    void onDensityChanged(CCObject* sender);
    void onChaosChanged(CCObject* sender);
    void onSymmetryChanged(CCObject* sender);
    void onDetailChanged(CCObject* sender);
    void onEdgeEmphasisChanged(CCObject* sender);
    void onFlowFollowingChanged(CCObject* sender);
    void onSymmetryMirroringChanged(CCObject* sender);
    void onAntiClutterChanged(CCObject* sender);
    
    void createOverlayPanel();
    void createToolbarButton();
    void createPresetSelector();
    void createSliders();
    void createToggles();
    void createActionButtons();
    void createProgressDisplay();
    
    void showOverlay();
    void hideOverlay();
    
    std::vector<geode::Ref<GameObject>> getSelectedObjects();
};

}
