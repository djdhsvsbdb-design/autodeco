#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include "ui/AIDecorateLayer.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    log::info("AI Decorator 3.0 loaded.");
}

class $modify(AIDecoratorEditorUI, EditorUI) {
    struct Fields {
        AutoDecoratorAI::AIDecorateLayer* aiDecorateLayer = nullptr;
    };

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;

        m_fields->aiDecorateLayer = AutoDecoratorAI::AIDecorateLayer::create(this);
        if (m_fields->aiDecorateLayer) {
            this->addChild(m_fields->aiDecorateLayer, 1000);
        }
        return true;
    }
};
