#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "ui/AIDecorateLayer.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("AI Decorator loaded.");
}

class $modify(EditorUI) {
    AutoDecoratorAI::AIDecorateLayer* m_aiDecorateLayer = nullptr;

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        m_aiDecorateLayer = AutoDecoratorAI::AIDecorateLayer::create(this);
        if (m_aiDecorateLayer) this->addChild(m_aiDecorateLayer, 1000);
        return true;
    }

    void destructor() {
        if (m_aiDecorateLayer) {
            m_aiDecorateLayer->removeFromParent();
            m_aiDecorateLayer = nullptr;
        }
        EditorUI::destructor();
    }
};
