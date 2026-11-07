#pragma once

#include <Geode/Geode.hpp>
#include "../ai/AIService.hpp"
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
    void onSetPrompt(CCObject* sender);
    void onGenerate(CCObject* sender);

    std::vector<geode::Ref<GameObject>> getSelectedObjects();
    void setStatus(const std::string& status);

    EditorUI* m_editorUI = nullptr;
    CCNode* m_overlayPanel = nullptr;
    CCMenu* m_mainMenu = nullptr;
    CCLabelBMFont* m_promptLabel = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;

    std::string m_prompt;
    bool m_isVisible = false;
    bool m_isGenerating = false;
};

} // namespace AutoDecoratorAI
