#include "AIDecorateLayer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>
#include <utility>

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
    if (!CCLayer::init() || !editorUI) return false;

    m_editorUI = editorUI;
    createToolbarButton();
    createOverlayPanel();
    hideOverlay();
    return true;
}

void AIDecorateLayer::createToolbarButton() {
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    auto sprite = ButtonSprite::create("AI Decorator");
    auto button = CCMenuItemSpriteExtra::create(
        sprite, this, menu_selector(AIDecorateLayer::onToggle)
    );
    auto menu = CCMenu::create();
    menu->addChild(button);
    menu->setPosition(winSize.width / 2, winSize.height - 30.0f);
    addChild(menu);
}

void AIDecorateLayer::createOverlayPanel() {
    const auto winSize = CCDirector::sharedDirector()->getWinSize();
    constexpr float width = 420.0f;
    constexpr float height = 300.0f;

    auto panel = CCScale9Sprite::create("square02_001.png");
    panel->setContentSize(CCSize(width, height));
    panel->setPosition(winSize.width / 2, winSize.height / 2);
    const float scale = std::min(1.0f, std::min(
        (winSize.width - 24.0f) / width,
        (winSize.height - 24.0f) / height
    ));
    panel->setScale(std::max(0.5f, scale));
    panel->setOpacity(230);
    m_overlayPanel = panel;
    addChild(panel, 10);

    auto title = CCLabelBMFont::create("AI DECORATOR", "goldFont.fnt");
    title->setPosition(width / 2, height - 28.0f);
    title->setScale(0.7f);
    panel->addChild(title);

    m_mainMenu = CCMenu::create();
    m_mainMenu->setPosition(0, 0);
    panel->addChild(m_mainMenu);

    auto close = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("X"), this, menu_selector(AIDecorateLayer::onToggle)
    );
    close->setPosition(width - 32.0f, height - 28.0f);
    m_mainMenu->addChild(close);

    auto description = CCLabelBMFont::create(
        "Describe the style and details you want", "bigFont.fnt"
    );
    description->setPosition(width / 2, height - 78.0f);
    description->setScale(0.43f);
    panel->addChild(description);

    auto promptButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Write Decoration Prompt"),
        this, menu_selector(AIDecorateLayer::onSetPrompt)
    );
    promptButton->setPosition(width / 2, height - 125.0f);
    m_mainMenu->addChild(promptButton);

    m_promptLabel = CCLabelBMFont::create("No prompt set", "chatFont.fnt");
    m_promptLabel->setPosition(width / 2, height - 160.0f);
    m_promptLabel->setScale(0.42f);
    panel->addChild(m_promptLabel);

    auto generate = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Generate AI Plan"),
        this, menu_selector(AIDecorateLayer::onGenerate)
    );
    generate->setPosition(width / 2, 92.0f);
    m_mainMenu->addChild(generate);

    m_statusLabel = CCLabelBMFont::create("Ready", "bigFont.fnt");
    m_statusLabel->setPosition(width / 2, 48.0f);
    m_statusLabel->setScale(0.42f);
    panel->addChild(m_statusLabel);

    auto note = CCLabelBMFont::create(
        "AI creates a plan preview; level objects are not changed yet.",
        "chatFont.fnt"
    );
    note->setPosition(width / 2, 22.0f);
    note->setScale(0.34f);
    panel->addChild(note);
}

void AIDecorateLayer::onToggle(CCObject*) {
    if (m_isVisible) hideOverlay();
    else showOverlay();
}

void AIDecorateLayer::showOverlay() {
    if (!m_overlayPanel) return;
    m_overlayPanel->stopAllActions();
    m_overlayPanel->setVisible(true);
    m_overlayPanel->setOpacity(0);
    m_overlayPanel->runAction(CCFadeTo::create(0.18f, 230));
    m_isVisible = true;
}

void AIDecorateLayer::hideOverlay() {
    if (!m_overlayPanel) return;
    m_overlayPanel->stopAllActions();
    m_overlayPanel->setVisible(false);
    m_isVisible = false;
}

void AIDecorateLayer::onSetPrompt(CCObject*) {
    auto input = TextInput::create("AI Decoration Prompt", "Describe the decoration style...");
    input->setCallback([this](const std::string& value) {
        auto cleaned = value;
        if (cleaned.size() > 1500) cleaned.resize(1500);
        m_prompt = cleaned;
        if (m_promptLabel) {
            std::string preview = m_prompt;
            if (preview.size() > 52) preview = preview.substr(0, 49) + "...";
            m_promptLabel->setString(m_prompt.empty() ? "No prompt set" : preview.c_str());
        }
        setStatus(m_prompt.empty() ? "Enter a prompt to begin" : "Prompt saved");
    });
    input->show();
}

void AIDecorateLayer::onGenerate(CCObject*) {
    if (m_isGenerating) return;
    if (m_prompt.empty()) {
        FLAlertLayer::create("Prompt Required", "Write what kind of decoration you want first.", "OK")->show();
        return;
    }

    auto selected = getSelectedObjects();
    if (selected.empty()) {
        FLAlertLayer::create("Nothing Selected", "Select the level objects or area you want the AI to decorate.", "OK")->show();
        return;
    }

    auto mod = Mod::get();
    std::vector<AI::ProviderConfig> providers;
    auto addProvider = [&providers, mod](
        const std::string& id, const std::string& name,
        const std::string& endpointKey, const std::string& modelKey, const std::string& keyKey
    ) {
        auto endpoint = mod->getSettingValue<std::string>(endpointKey);
        auto model = mod->getSettingValue<std::string>(modelKey);
        auto apiKey = mod->getSettingValue<std::string>(keyKey);
        if (!endpoint.empty() && !model.empty() && !apiKey.empty()) {
            providers.push_back(AI::ProviderConfig{id, name, endpoint, model, apiKey, true});
        }
    };

    addProvider("primary", "Primary provider", "provider-1-endpoint", "provider-1-model", "provider-1-key");
    addProvider("backup", "Backup provider", "provider-2-endpoint", "provider-2-model", "provider-2-key");

    AI::AIService::get()->setProviders(std::move(providers));
    m_isGenerating = true;
    setStatus("Contacting AI providers...");

    const std::string requestPrompt =
        m_prompt + "\nSelected Geometry Dash objects: " + std::to_string(selected.size()) +
        ". Return a JSON design plan with semantic decoration categories and relative placement guidance. "
        "Do not claim that objects have already been added to the level.";

    geode::Ref<AIDecorateLayer> self(this);
    AI::AIService::get()->generateDecorationPlan(requestPrompt,
        [self](bool success, std::string response, std::string message) {
            if (!self) return;
            self->m_isGenerating = false;
            if (!success) {
                self->setStatus("AI request failed");
                FLAlertLayer::create("AI Error", message, "OK")->show();
                return;
            }
            self->setStatus("AI plan received");
            FLAlertLayer::create("AI Decoration Plan", response, "Close")->show();
        }
    );
}

void AIDecorateLayer::setStatus(const std::string& status) {
    if (m_statusLabel) m_statusLabel->setString(status.c_str());
}

std::vector<geode::Ref<GameObject>> AIDecorateLayer::getSelectedObjects() {
    std::vector<geode::Ref<GameObject>> objects;
    if (!m_editorUI) return objects;
    auto editorLayer = m_editorUI->getEditorLayer();
    if (!editorLayer) return objects;
    for (auto object : editorLayer->getSelectedObjects()) objects.push_back(object);
    return objects;
}

} // namespace AutoDecoratorAI
