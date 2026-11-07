#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

#include "ui/AIDecorateLayer.hpp"
#include "presets/AIPresetManager.hpp"

using namespace geode::prelude;

$on_mod(Loaded) {
    AutoDecoratorAI::AIPresetManager::get()->initialize();
    log::info("Alt3rDECO loaded");
}

class $modify(EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        auto decorateLayer = AutoDecoratorAI::AIDecorateLayer::create(this);
        if (decorateLayer) {
            this->addChild(decorateLayer, 1000);
        }

        return true;
    }
};
