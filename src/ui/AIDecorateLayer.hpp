#pragma once

#include <Geode/Geode.hpp>
#include "../ai/AIService.hpp"
#include "../ai/DecorationPlan.hpp"
#include <string>
#include <vector>

using namespace geode::prelude;

namespace AutoDecoratorAI {

class AIDecorateLayer : public cocos2d::CCLayer {
public:
    static AIDecorateLayer* create(EditorUI* editorUI);

private:
    bool init(EditorUI* editorUI);
    void createToolbarButton();
    void createOverlayPanel();
    void showOverlay();
    void hideOverlay();

    void onToggle(CCObject* sender);
    void onGenerate(CCObject* sender);
    void onApply(CCObject* sender);

    std::vector<geode::Ref<GameObject>> getSelectedObjects();
    void setStatus(const std::string& status);

    EditorUI* m_editorUI = nullptr;
    CCNode* m_overlayPanel = nullptr;
    CCMenu* m_mainMenu = nullptr;
    CCLabelBMFont* m_promptLabel = nullptr;
    TextInput* m_promptInput = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;
    CCMenuItemSpriteExtra* m_applyButton = nullptr;

    std::string m_prompt;
    bool m_isVisible = false;
    bool m_isGenerating = false;
    bool m_hasValidPlan = false;
    AI::DecorationPlan m_lastPlan;
    std::vector<int> m_allowedObjectIds;
    CCPoint m_areaMin = CCPointZero;
    CCPoint m_areaMax = CCPointZero;
};

} // namespace AutoDecoratorAI
