#include <algorithm>
#include "AIDecorateLayer.hpp"
#include "../presets/AIPresetManager.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

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
        // Keep the pointer owned by the manager, not by the temporary UI copy.
        m_currentPreset = presetManager->getPresetById(m_availablePresets[0].id);
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
    auto buttonSprite = ButtonSprite::create("Alt3rDECO");
    auto button = CCMenuItemSpriteExtra::create(
        buttonSprite, this, menu_selector(AIDecorateLayer::onToggle)
    );
    button->setScale(0.72f);
    m_toggleButton = button;
    auto menu = CCMenu::create();
    menu->addChild(button);
    menu->setPosition(winSize.width / 2, winSize.height - 24.f);
    this->addChild(menu, 50);
}

void AIDecorateLayer::createOverlayPanel() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    constexpr float panelWidth = 420.f;
    constexpr float panelHeight = 300.f;
    // Soft white halo behind the dark editor panel.
    auto glow = CCScale9Sprite::create("square02_001.png");
    glow->setContentSize(CCSize(panelWidth + 10.f, panelHeight + 10.f));
    glow->setPosition(winSize.width / 2.f, winSize.height / 2.f);
    glow->setColor(ccc3(255, 255, 255));
    glow->setOpacity(42);
    this->addChild(glow, 9);

    auto panel = CCScale9Sprite::create("square02_001.png");
    panel->setContentSize(CCSize(panelWidth, panelHeight));
    panel->setPosition(winSize.width / 2.f, winSize.height / 2.f);
    panel->setColor(ccc3(8, 8, 12));
    panel->setOpacity(248);
    const float fitScale = std::min(
        1.f,
        std::min((winSize.width - 20.f) / panelWidth,
                 (winSize.height - 20.f) / panelHeight)
    );
    panel->setScale(std::max(0.62f, fitScale));
    m_overlayPanel = panel;
    this->addChild(panel, 10);
    m_mainMenu = CCMenu::create();
    m_mainMenu->setPosition(0, 0);
    panel->addChild(m_mainMenu, 5);
}

void AIDecorateLayer::createPresetSelector() {
    auto panelSize = m_overlayPanel->getContentSize();
    auto heading = CCLabelBMFont::create("PRESETS", "bigFont.fnt");
    heading->setPosition(105.f, panelSize.height - 28.f);
    heading->setScale(0.42f);
    heading->setColor(ccc3(255, 255, 255));
    m_overlayPanel->addChild(heading);
    m_presetLabel = CCLabelBMFont::create(
        m_currentPreset ? m_currentPreset->name.c_str() : "None", "goldFont.fnt"
    );
    m_presetLabel->setPosition(105.f, panelSize.height - 48.f);
    m_presetLabel->setScale(0.38f);
    m_presetLabel->setColor(ccc3(255, 255, 255));
    m_overlayPanel->addChild(m_presetLabel);
    m_presetMenu = CCMenu::create();
    m_presetMenu->setPosition(105.f, panelSize.height - 76.f);
    m_overlayPanel->addChild(m_presetMenu, 6);
    updatePresetDropdown();
}

void AIDecorateLayer::updatePresetDropdown() {
    m_presetMenu->removeAllChildren();
    for (size_t i = 0; i < m_availablePresets.size(); ++i) {
        const auto& preset = m_availablePresets[i];
        auto label = CCLabelBMFont::create(preset.name.c_str(), "chatFont.fnt");
        label->setScale(0.34f);
        label->setColor(ccc3(255, 255, 255));
        auto button = CCMenuItemLabel::create(
            label, this, menu_selector(AIDecorateLayer::onPresetChanged)
        );
        button->setUserObject(CCString::create(preset.id.c_str()));
        const int column = static_cast<int>(i % 2);
        const int row = static_cast<int>(i / 2);
        button->setPosition(column == 0 ? -48.f : 48.f, -static_cast<float>(row) * 21.f);
        m_presetMenu->addChild(button);
    }
}

void AIDecorateLayer::createSliders() {
    const float labelX = 245.f;
    const float sliderX = 340.f;
    const float startY = 246.f;
    const char* names[] = {"Density", "Chaos", "Symmetry", "Detail"};
    Slider** sliders[] = {&m_densitySlider, &m_chaosSlider, &m_symmetrySlider, &m_detailSlider};
    const float defaults[] = {0.5f, 0.3f, 0.7f, 0.4f};
    for (int i = 0; i < 4; ++i) {
        const float y = startY - static_cast<float>(i) * 34.f;
        auto label = CCLabelBMFont::create(names[i], "bigFont.fnt");
        label->setPosition(labelX, y);
        label->setScale(0.34f);
        label->setColor(ccc3(255, 255, 255));
        m_overlayPanel->addChild(label);
        SEL_MenuHandler callback =
            i == 0 ? menu_selector(AIDecorateLayer::onDensityChanged) :
            i == 1 ? menu_selector(AIDecorateLayer::onChaosChanged) :
            i == 2 ? menu_selector(AIDecorateLayer::onSymmetryChanged) :
                     menu_selector(AIDecorateLayer::onDetailChanged);
        *sliders[i] = Slider::create(this, callback, defaults[i]);
        (*sliders[i])->setPosition(sliderX, y);
        (*sliders[i])->setScale(0.72f);
        m_mainMenu->addChild(*sliders[i]);
    }
}

void AIDecorateLayer::createToggles() {
    struct ToggleInfo {
        const char* label;
        CCMenuItemToggler** target;
        SEL_MenuHandler callback;
        bool initial;
        float x;
        float y;
    };
    const bool edge = m_currentPreset ? m_currentPreset->edgeEmphasis : true;
    const bool flow = m_currentPreset ? m_currentPreset->flowFollowing : true;
    const bool symmetry = m_currentPreset ? m_currentPreset->symmetryMirroring : true;
    const bool clutter = m_currentPreset ? m_currentPreset->antiClutterMode : false;
    ToggleInfo items[] = {
        {"Edge", &m_edgeEmphasisToggle, menu_selector(AIDecorateLayer::onEdgeEmphasisChanged), edge, 55.f, 112.f},
        {"Flow", &m_flowFollowingToggle, menu_selector(AIDecorateLayer::onFlowFollowingChanged), flow, 160.f, 112.f},
        {"Mirror", &m_symmetryMirroringToggle, menu_selector(AIDecorateLayer::onSymmetryMirroringChanged), symmetry, 55.f, 76.f},
        {"Anti-Clutter", &m_antiClutterToggle, menu_selector(AIDecorateLayer::onAntiClutterChanged), clutter, 160.f, 76.f}
    };
    for (const auto& item : items) {
        auto onSprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto offSprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        *item.target = CCMenuItemToggler::create(onSprite, offSprite, this, item.callback);
        (*item.target)->setPosition(item.x, item.y);
        (*item.target)->setScale(0.78f);
        (*item.target)->toggle(item.initial);
        m_mainMenu->addChild(*item.target);
        auto label = CCLabelBMFont::create(item.label, "bigFont.fnt");
        label->setPosition(item.x + 17.f, item.y);
        label->setScale(0.32f);
        label->setColor(ccc3(255, 255, 255));
        label->setAnchorPoint(CCPoint(0.f, 0.5f));
        m_overlayPanel->addChild(label);
    }
}

void AIDecorateLayer::createActionButtons() {
    const float x = 320.f;
    const float ys[] = {112.f, 70.f, 28.f};
    const char* titles[] = {"Analyze", "Apply Deco", "Save Preset"};
    SEL_MenuHandler callbacks[] = {
        menu_selector(AIDecorateLayer::onAnalyze),
        menu_selector(AIDecorateLayer::onApply),
        menu_selector(AIDecorateLayer::onSavePreset)
    };
    CCMenuItemSpriteExtra** targets[] = {
        &m_analyzeButton, &m_applyButton, &m_savePresetButton
    };
    for (int i = 0; i < 3; ++i) {
        auto sprite = ButtonSprite::create(titles[i]);
        *targets[i] = CCMenuItemSpriteExtra::create(sprite, this, callbacks[i]);
        (*targets[i])->setScale(0.62f);
        (*targets[i])->setPosition(x, ys[i]);
        m_mainMenu->addChild(*targets[i]);
    }
}

void AIDecorateLayer::createProgressDisplay() {
    m_progressLabel = CCLabelBMFont::create("", "bigFont.fnt");
    m_progressLabel->setPosition(105.f, 24.f);
    m_progressLabel->setScale(0.32f);
    m_progressLabel->setColor(ccc3(255, 255, 255));
    m_progressLabel->setVisible(false);
    m_overlayPanel->addChild(m_progressLabel);
    auto progressSprite = CCSprite::createWithSpriteFrameName("barbg.png");
    m_progressBar = CCProgressTimer::create(progressSprite);
    m_progressBar->setType(kCCProgressTimerTypeBar);
    m_progressBar->setBarChangeRate(CCSize(1.f, 0.f));
    m_progressBar->setMidpoint(CCPoint(0.f, 0.f));
    m_progressBar->setPosition(105.f, 9.f);
    m_progressBar->setScaleX(0.55f);
    m_progressBar->setScaleY(0.4f);
    m_progressBar->setPercentage(0.f);
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
    // TextInput is a node, not a popup. First click reveals the field; the
    // second click on the same Save button confirms the name.
    if (!m_presetNameInput) {
        m_presetNameInput = TextInput::create(220.f, "Enter preset name");
        if (!m_presetNameInput) {
            FLAlertLayer::create("Error", "Could not create the preset name field.", "OK")->show();
            return;
        }
        m_presetNameInput->setLabel("Preset Name");
        m_presetNameInput->setCommonFilter(geode::CommonFilter::Name);
        m_presetNameInput->setMaxCharCount(40);
        m_presetNameInput->setPosition(
            CCPoint(m_overlayPanel->getContentSize().width / 2.f, 20.f)
        );
        m_overlayPanel->addChild(m_presetNameInput, 20);
        m_presetNameInput->focus();
        FLAlertLayer::create(
            "Save Preset",
            "Enter a name, then tap Save AI Preset again.",
            "OK"
        )->show();
        return;
    }

    const std::string name = m_presetNameInput->getString();
    if (name.empty()) {
        FLAlertLayer::create("Preset Name", "Enter a name before saving.", "OK")->show();
        m_presetNameInput->focus();
        return;
    }

    auto presetManager = AIPresetManager::get();
    auto newPreset = presetManager->createCustomPreset(name, "Custom user preset");

    if (m_currentPreset) {
        newPreset.styleBias = m_currentPreset->styleBias;
        newPreset.decorationPool = m_currentPreset->decorationPool;
        newPreset.styleWeights = m_currentPreset->styleWeights;
        newPreset.densityCurve = m_currentPreset->densityCurve;
    }

    newPreset.edgeEmphasis = m_edgeEmphasisToggle->isToggled();
    newPreset.flowFollowing = m_flowFollowingToggle->isToggled();
    newPreset.symmetryMirroring = m_symmetryMirroringToggle->isToggled();
    newPreset.antiClutterMode = m_antiClutterToggle->isToggled();
    newPreset.chaosTolerance = m_chaosSlider->getValue();
    newPreset.symmetryPreference = m_symmetrySlider->getValue();
    newPreset.microDetailLevel = m_detailSlider->getValue();

    const std::string selectedPresetId = m_currentPreset ? m_currentPreset->id : newPreset.id;
    presetManager->addPreset(newPreset);
    presetManager->saveCustomPresets();

    m_availablePresets = presetManager->getAllPresets();
    m_currentPreset = presetManager->getPresetById(selectedPresetId);
    updatePresetDropdown();

    m_presetNameInput->removeFromParent();
    m_presetNameInput = nullptr;

    FLAlertLayer::create("Success", "Preset saved successfully!", "OK")->show();
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
    
    // EditorUI owns the current selection; wrap its CCArray with Geode's
    // typed range adapter before copying references.
    auto selectedArray = m_editorUI->getSelectedObjects();
    if (!selectedArray) {
        return selectedObjects;
    }

    for (auto obj : CCArrayExt<GameObject*>(selectedArray)) {
        if (obj) {
            selectedObjects.emplace_back(obj);
        }
    }
    
    return selectedObjects;
}

}
