#include "AIDecorateLayer.hpp"
#include "../presets/AIPresetManager.hpp"
#include <Geode/Geode.hpp>

using namespace geode::preload;

namespace AutoDecoratorAI {

AIDecorateLayer* AIDecorateLayer::create(EditorUI* editorUI) {
    auto ret = new AIDecorateLayer();
    if (ret && ret->init(editorUI)) {
        ret->autorelease();
        return ret;
    }
    CC_SAFE_DELETE(ret);
    return nullptr;
}

bool AIDecorateLayer::init(EditorUI* editorUI) {
    if (!CCLayer::init()) {
        return false;
    }
    
    m_editorUI = editorUI;
    m_decorationEngine = AIDecorationEngine::get();
    m_isVisible = false;
    m_currentPreset = nullptr;
    
    // Initialize decoration engine
    m_decorationEngine->initialize();
    
    // Set progress callback
    m_decorationEngine->setProgressCallback([this](const std::string& status, float progress) {
        this->updateProgressDisplay(status, progress);
    });
    
    // Load available presets
    auto presetManager = AIPresetManager::get();
    m_availablePresets = presetManager->getAllPresets();
    
    if (!m_availablePresets.empty()) {
        m_currentPreset = &m_availablePresets[0];
    }
    
    // Create UI elements
    createToolbarButton();
    createOverlayPanel();
    createPresetSelector();
    createSliders();
    createToggles();
    createActionButtons();
    createProgressDisplay();
    
    // Initially hide overlay
    hideOverlay();
    
    return true;
}

void AIDecorateLayer::createToolbarButton() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    // Create toggle button
    auto buttonSprite = ButtonSprite::create("Alt3rDECO");
    auto button = CCMenuItemSpriteExtra::create(
        buttonSprite,
        this,
        menu_selector(AIDecorateLayer::onToggle)
    );
    
    m_toggleButton = button;
    
    // Position in toolbar area
    auto menu = CCMenu::create();
    menu->addChild(button);
    menu->setPosition(winSize.width / 2, winSize.height - 30);
    this->addChild(menu);
}

void AIDecorateLayer::createOverlayPanel() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    
    // Create main panel
    auto panel = CCScale9Sprite::create("square02_001.png");
    panel->setContentSize(CCSize(400, 500));
    panel->setPosition(winSize.width / 2, winSize.height / 2);
    panel->setOpacity(200);
    
    m_overlayPanel = panel;
    this->addChild(panel);
    
    // Create main menu for panel content
    m_mainMenu = CCMenu::create();
    m_mainMenu->setPosition(0, 0);
    panel->addChild(m_mainMenu);
}

void AIDecorateLayer::createPresetSelector() {
    auto panelSize = m_overlayPanel->getContentSize();
    
    // Preset label
    auto presetLabel = CCLabelBMFont::create("Preset:", "bigFont.fnt");
    presetLabel->setPosition(CCSize(panelSize.width / 2, panelSize.height - 40));
    presetLabel->setScale(0.6f);
    m_overlayPanel->addChild(presetLabel);
    
    // Current preset display
    m_presetLabel = CCLabelBMFont::create(
        m_currentPreset ? m_currentPreset->name.c_str() : "None",
        "goldFont.fnt"
    );
    m_presetLabel->setPosition(CCSize(panelSize.width / 2, panelSize.height - 70));
    m_presetLabel->setScale(0.5f);
    m_overlayPanel->addChild(m_presetLabel);
    
    // Preset selector menu
    m_presetMenu = CCMenu::create();
    m_presetMenu->setPosition(CCSize(panelSize.width / 2, panelSize.height - 100));
    m_overlayPanel->addChild(m_presetMenu);
    
    updatePresetDropdown();
}

void AIDecorateLayer::updatePresetDropdown() {
    m_presetMenu->removeAllChildren();
    
    float yOffset = 0;
    for (const auto& preset : m_availablePresets) {
        auto label = CCLabelBMFont::create(preset.name.c_str(), "chatFont.fnt");
        label->setScale(0.4f);
        
        auto button = CCMenuItemLabel::create(
            label,
            this,
            menu_selector(AIDecorateLayer::onPresetChanged)
        );
        button->setUserObject(CCString::create(preset.id.c_str()));
        button->setPosition(0, yOffset);
        
        m_presetMenu->addChild(button);
        yOffset -= 25;
    }
}

void AIDecorateLayer::createSliders() {
    auto panelSize = m_overlayPanel->getContentSize();
    float startY = panelSize.height - 200;
    
    // Density slider
    auto densityLabel = CCLabelBMFont::create("Density:", "bigFont.fnt");
    densityLabel->setPosition(CCSize(80, startY));
    densityLabel->setScale(0.4f);
    m_overlayPanel->addChild(densityLabel);
    
    m_densitySlider = Slider::create(this, menu_selector(AIDecorateLayer::onDensityChanged), 0.5f);
    m_densitySlider->setPosition(CCSize(panelSize.width / 2 + 50, startY));
    m_densitySlider->setScale(0.8f);
    m_mainMenu->addChild(m_densitySlider);
    
    // Chaos slider
    auto chaosLabel = CCLabelBMFont::create("Chaos:", "bigFont.fnt");
    chaosLabel->setPosition(CCSize(80, startY - 40));
    chaosLabel->setScale(0.4f);
    m_overlayPanel->addChild(chaosLabel);
    
    m_chaosSlider = Slider::create(this, menu_selector(AIDecorateLayer::onChaosChanged), 0.3f);
    m_chaosSlider->setPosition(CCSize(panelSize.width / 2 + 50, startY - 40));
    m_chaosSlider->setScale(0.8f);
    m_mainMenu->addChild(m_chaosSlider);
    
    // Symmetry slider
    auto symmetryLabel = CCLabelBMFont::create("Symmetry:", "bigFont.fnt");
    symmetryLabel->setPosition(CCSize(80, startY - 80));
    symmetryLabel->setScale(0.4f);
    m_overlayPanel->addChild(symmetryLabel);
    
    m_symmetrySlider = Slider::create(this, menu_selector(AIDecorateLayer::onSymmetryChanged), 0.7f);
    m_symmetrySlider->setPosition(CCSize(panelSize.width / 2 + 50, startY - 80));
    m_symmetrySlider->setScale(0.8f);
    m_mainMenu->addChild(m_symmetrySlider);
    
    // Detail slider
    auto detailLabel = CCLabelBMFont::create("Detail:", "bigFont.fnt");
    detailLabel->setPosition(CCSize(80, startY - 120));
    detailLabel->setScale(0.4f);
    m_overlayPanel->addChild(detailLabel);
    
    m_detailSlider = Slider::create(this, menu_selector(AIDecorateLayer::onDetailChanged), 0.4f);
    m_detailSlider->setPosition(CCSize(panelSize.width / 2 + 50, startY - 120));
    m_detailSlider->setScale(0.8f);
    m_mainMenu->addChild(m_detailSlider);
}

void AIDecorateLayer::createToggles() {
    auto panelSize = m_overlayPanel->getContentSize();
    float startY = panelSize.height - 280;
    
    // Edge Emphasis toggle
    auto edgeSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto edgeUncheckSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    m_edgeEmphasisToggle = CCMenuItemToggler::create(
        edgeSprite,
        edgeUncheckSprite,
        this,
        menu_selector(AIDecorateLayer::onEdgeEmphasisChanged)
    );
    m_edgeEmphasisToggle->setPosition(CCSize(100, startY));
    m_edgeEmphasisToggle->toggle(m_currentPreset ? m_currentPreset->edgeEmphasis : true);
    m_mainMenu->addChild(m_edgeEmphasisToggle);
    
    auto edgeLabel = CCLabelBMFont::create("Edge Emphasis", "bigFont.fnt");
    edgeLabel->setPosition(CCSize(180, startY));
    edgeLabel->setScale(0.35f);
    edgeLabel->setAnchorPoint(CCSize(0, 0.5f));
    m_overlayPanel->addChild(edgeLabel);
    
    // Flow Following toggle
    auto flowSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto flowUncheckSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    m_flowFollowingToggle = CCMenuItemToggler::create(
        flowSprite,
        flowUncheckSprite,
        this,
        menu_selector(AIDecorateLayer::onFlowFollowingChanged)
    );
    m_flowFollowingToggle->setPosition(CCSize(100, startY - 35));
    m_flowFollowingToggle->toggle(m_currentPreset ? m_currentPreset->flowFollowing : true);
    m_mainMenu->addChild(m_flowFollowingToggle);
    
    auto flowLabel = CCLabelBMFont::create("Flow Following", "bigFont.fnt");
    flowLabel->setPosition(CCSize(180, startY - 35));
    flowLabel->setScale(0.35f);
    flowLabel->setAnchorPoint(CCSize(0, 0.5f));
    m_overlayPanel->addChild(flowLabel);
    
    // Symmetry Mirroring toggle
    auto symSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto symUncheckSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    m_symmetryMirroringToggle = CCMenuItemToggler::create(
        symSprite,
        symUncheckSprite,
        this,
        menu_selector(AIDecorateLayer::onSymmetryMirroringChanged)
    );
    m_symmetryMirroringToggle->setPosition(CCSize(100, startY - 70));
    m_symmetryMirroringToggle->toggle(m_currentPreset ? m_currentPreset->symmetryMirroring : true);
    m_mainMenu->addChild(m_symmetryMirroringToggle);
    
    auto symLabel = CCLabelBMFont::create("Symmetry Mirroring", "bigFont.fnt");
    symLabel->setPosition(CCSize(180, startY - 70));
    symLabel->setScale(0.35f);
    symLabel->setAnchorPoint(CCSize(0, 0.5f));
    m_overlayPanel->addChild(symLabel);
    
    // Anti-Clutter toggle
    auto clutterSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    auto clutterUncheckSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    m_antiClutterToggle = CCMenuItemToggler::create(
        clutterSprite,
        clutterUncheckSprite,
        this,
        menu_selector(AIDecorateLayer::onAntiClutterChanged)
    );
    m_antiClutterToggle->setPosition(CCSize(100, startY - 105));
    m_antiClutterToggle->toggle(m_currentPreset ? m_currentPreset->antiClutterMode : false);
    m_mainMenu->addChild(m_antiClutterToggle);
    
    auto clutterLabel = CCLabelBMFont::create("Anti-Clutter", "bigFont.fnt");
    clutterLabel->setPosition(CCSize(180, startY - 105));
    clutterLabel->setScale(0.35f);
    clutterLabel->setAnchorPoint(CCSize(0, 0.5f));
    m_overlayPanel->addChild(clutterLabel);
}

void AIDecorateLayer::createActionButtons() {
    auto panelSize = m_overlayPanel->getContentSize();
    float startY = 80;
    
    // Analyze button
    auto analyzeSprite = ButtonSprite::create("Analyze Selection");
    m_analyzeButton = CCMenuItemSpriteExtra::create(
        analyzeSprite,
        this,
        menu_selector(AIDecorateLayer::onAnalyze)
    );
    m_analyzeButton->setPosition(CCSize(panelSize.width / 2, startY + 60));
    m_mainMenu->addChild(m_analyzeButton);
    
    // Apply button
    auto applySprite = ButtonSprite::create("Apply AI Decoration");
    m_applyButton = CCMenuItemSpriteExtra::create(
        applySprite,
        this,
        menu_selector(AIDecorateLayer::onApply)
    );
    m_applyButton->setPosition(CCSize(panelSize.width / 2, startY + 20));
    m_mainMenu->addChild(m_applyButton);
    
    // Save Preset button
    auto saveSprite = ButtonSprite::create("Save AI Preset");
    m_savePresetButton = CCMenuItemSpriteExtra::create(
        saveSprite,
        this,
        menu_selector(AIDecorateLayer::onSavePreset)
    );
    m_savePresetButton->setPosition(CCSize(panelSize.width / 2, startY - 20));
    m_mainMenu->addChild(m_savePresetButton);
}

void AIDecorateLayer::createProgressDisplay() {
    auto panelSize = m_overlayPanel->getContentSize();
    
    // Progress label
    m_progressLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_progressLabel->setPosition(CCSize(panelSize.width / 2, 30));
    m_progressLabel->setScale(0.4f);
    m_progressLabel->setVisible(false);
    m_overlayPanel->addChild(m_progressLabel);
    
    // Progress bar
    auto progressSprite = CCSprite::createWithSpriteFrameName("barbg.png");
    m_progressBar = CCProgressTimer::create(progressSprite);
    m_progressBar->setType(kCCProgressTimerTypeBar);
    m_progressBar->setBarChangeRate(CCSize(1, 0));
    m_progressBar->setMidpoint(CCSize(0, 0));
    m_progressBar->setPosition(CCSize(panelSize.width / 2, 15));
    m_progressBar->setScaleX(0.8f);
    m_progressBar->setScaleY(0.5f);
    m_progressBar->setPercentage(0);
    m_progressBar->setVisible(false);
    m_overlayPanel->addChild(m_progressBar);
}

void AIDecorateLayer::updateProgressDisplay(const std::string& status, float progress) {
    if (m_progressLabel) {
        m_progressLabel->setString(status.c_str());
        m_progressLabel->setVisible(true);
    }
    
    if (m_progressBar) {
        m_progressBar->setPercentage(progress * 100);
        m_progressBar->setVisible(true);
    }
}

void AIDecorateLayer::onToggle(CCObject* sender) {
    if (m_isVisible) {
        hideOverlay();
    } else {
        showOverlay();
    }
    m_isVisible = !m_isVisible;
}

void AIDecorateLayer::showOverlay() {
    m_overlayPanel->setVisible(true);
    m_overlayPanel->runAction(CCFadeIn::create(0.2f));
}

void AIDecorateLayer::hideOverlay() {
    m_overlayPanel->setVisible(false);
}

void AIDecorateLayer::onPresetChanged(CCObject* sender) {
    auto button = static_cast<CCMenuItemLabel*>(sender);
    auto presetId = static_cast<CCString*>(button->getUserObject())->getCString();
    
    auto presetManager = AIPresetManager::get();
    m_currentPreset = presetManager->getPresetById(presetId);
    
    if (m_currentPreset) {
        m_presetLabel->setString(m_currentPreset->name.c_str());
        
        // Update toggles to match preset
        m_edgeEmphasisToggle->toggle(m_currentPreset->edgeEmphasis);
        m_flowFollowingToggle->toggle(m_currentPreset->flowFollowing);
        m_symmetryMirroringToggle->toggle(m_currentPreset->symmetryMirroring);
        m_antiClutterToggle->toggle(m_currentPreset->antiClutterMode);
        
        // Update sliders
        m_chaosSlider->setValue(m_currentPreset->chaosTolerance);
        m_symmetrySlider->setValue(m_currentPreset->symmetryPreference);
        m_detailSlider->setValue(m_currentPreset->microDetailLevel);
    }
}

void AIDecorateLayer::onAnalyze(CCObject* sender) {
    auto selectedObjects = getSelectedObjects();
    
    if (selectedObjects.empty()) {
        FLAlertLayer::create("No Selection", "Please select some objects in the editor first.", "OK")->show();
        return;
    }
    
    // Perform analysis
    auto analyzer = GeometryAnalyzer::get();
    auto clusters = analyzer->analyzeSelection(selectedObjects);
    
    auto patternDetector = PatternDetector::get();
    auto patterns = patternDetector->detectPatterns(selectedObjects);
    
    // Show results
    std::string result = "Analysis Complete:\n\n";
    result += "Clusters found: " + std::to_string(clusters.size()) + "\n";
    result += "Symmetry: ";
    
    switch (patterns.symmetryType) {
        case SymmetryType::Horizontal: result += "Horizontal"; break;
        case SymmetryType::Vertical: result += "Vertical"; break;
        case SymmetryType::Radial: result += "Radial"; break;
        case SymmetryType::Bilateral: result += "Bilateral"; break;
        default: result += "None"; break;
    }
    
    result += " (" + std::to_string(static_cast<int>(patterns.symmetryStrength * 100)) + "%)\n";
    result += "Repetition: " + std::string(patterns.hasRepetition ? "Yes" : "No") + "\n";
    result += "Density Imbalance: " + std::to_string(static_cast<int>(patterns.densityImbalance * 100)) + "%\n";
    
    FLAlertLayer::create("Geometry Analysis", result, "OK")->show();
}

void AIDecorateLayer::onApply(CCObject* sender) {
    auto selectedObjects = getSelectedObjects();
    
    if (selectedObjects.empty()) {
        FLAlertLayer::create("No Selection", "Please select some objects in the editor first.", "OK")->show();
        return;
    }
    
    if (!m_currentPreset) {
        FLAlertLayer::create("No Preset", "Please select an AI preset first.", "OK")->show();
        return;
    }
    
    // Update preset with current slider values
    AIPreset modifiedPreset = *m_currentPreset;
    modifiedPreset.edgeEmphasis = m_edgeEmphasisToggle->isToggled();
    modifiedPreset.flowFollowing = m_flowFollowingToggle->isToggled();
    modifiedPreset.symmetryMirroring = m_symmetryMirroringToggle->isToggled();
    modifiedPreset.antiClutterMode = m_antiClutterToggle->isToggled();
    
    // Generate decorations
    auto placements = m_decorationEngine->generateDecorations(
        selectedObjects,
        modifiedPreset,
        m_densitySlider->getValue(),
        m_chaosSlider->getValue(),
        m_symmetrySlider->getValue(),
        m_detailSlider->getValue()
    );
    
    if (placements.empty()) {
        FLAlertLayer::create("No Decorations", "No decorations were generated. Try adjusting the parameters.", "OK")->show();
        return;
    }
    
    // Apply to level
    m_decorationEngine->applyDecorationsToLevel(placements, m_editorUI);
    
    // Hide progress display
    m_progressLabel->setVisible(false);
    m_progressBar->setVisible(false);
    
    FLAlertLayer::create("Success", "Applied " + std::to_string(placements.size()) + " decorations!", "OK")->show();
}

void AIDecorateLayer::onSavePreset(CCObject* sender) {
    auto input = TextInput::create("Preset Name", "Enter preset name");
    input->setCallback([this](const std::string& name) {
        if (name.empty()) return;
        
        auto presetManager = AIPresetManager::get();
        auto newPreset = presetManager->createCustomPreset(name, "Custom user preset");
        
        // Copy current settings
        if (m_currentPreset) {
            newPreset.styleBias = m_currentPreset->styleBias;
            newPreset.decorationPool = m_currentPreset->decorationPool;
            newPreset.styleWeights = m_currentPreset->styleWeights;
        }
        
        newPreset.edgeEmphasis = m_edgeEmphasisToggle->isToggled();
        newPreset.flowFollowing = m_flowFollowingToggle->isToggled();
        newPreset.symmetryMirroring = m_symmetryMirroringToggle->isToggled();
        newPreset.antiClutterMode = m_antiClutterToggle->isToggled();
        newPreset.chaosTolerance = m_chaosSlider->getValue();
        newPreset.symmetryPreference = m_symmetrySlider->getValue();
        newPreset.microDetailLevel = m_detailSlider->getValue();
        
        presetManager->addPreset(newPreset);
        presetManager->saveCustomPresets();
        
        // Refresh preset list
        m_availablePresets = presetManager->getAllPresets();
        updatePresetDropdown();
        
        FLAlertLayer::create("Success", "Preset saved successfully!", "OK")->show();
    });
    
    input->show();
}

void AIDecorateLayer::onDensityChanged(CCObject* sender) {
    // Density is handled as override during generation
}

void AIDecorateLayer::onChaosChanged(CCObject* sender) {
    // Chaos is handled as override during generation
}

void AIDecorateLayer::onSymmetryChanged(CCObject* sender) {
    // Symmetry is handled as override during generation
}

void AIDecorateLayer::onDetailChanged(CCObject* sender) {
    // Detail is handled as override during generation
}

void AIDecorateLayer::onEdgeEmphasisChanged(CCObject* sender) {
    // Toggle state is read during generation
}

void AIDecorateLayer::onFlowFollowingChanged(CCObject* sender) {
    // Toggle state is read during generation
}

void AIDecorateLayer::onSymmetryMirroringChanged(CCObject* sender) {
    // Toggle state is read during generation
}

void AIDecorateLayer::onAntiClutterChanged(CCObject* sender) {
    // Toggle state is read during generation
}

std::vector<geode::Ref<GameObject>> AIDecorateLayer::getSelectedObjects() {
    std::vector<geode::Ref<GameObject>> selectedObjects;
    
    if (!m_editorUI) {
        return selectedObjects;
    }
    
    auto editorLayer = m_editorUI->m_editorLayer;
    if (!editorLayer) {
        return selectedObjects;
    }
    
    // Get selected objects from editor
    auto selected = editorLayer->getSelectedObjects();
    for (auto obj : selected) {
        selectedObjects.push_back(obj);
    }
    
    return selectedObjects;
}

}
