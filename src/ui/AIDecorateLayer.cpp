#include "AIDecorateLayer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <unordered_set>
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
    constexpr float height = 320.0f;

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
    generate->setPosition(width / 2, 100.0f);
    m_mainMenu->addChild(generate);

    m_applyButton = CCMenuItemSpriteExtra::create(
        ButtonSprite::create("Apply Decoration"),
        this, menu_selector(AIDecorateLayer::onApply)
    );
    m_applyButton->setPosition(width / 2, 60.0f);
    m_applyButton->setEnabled(false);
    m_mainMenu->addChild(m_applyButton);

    m_statusLabel = CCLabelBMFont::create("Ready", "bigFont.fnt");
    m_statusLabel->setPosition(width / 2, 30.0f);
    m_statusLabel->setScale(0.42f);
    panel->addChild(m_statusLabel);

    auto note = CCLabelBMFont::create(
        "Review the preview before applying. One editor paste = one Undo step.",
        "chatFont.fnt"
    );
    note->setPosition(width / 2, 12.0f);
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
        FLAlertLayer::create("Nothing Selected", "Select the level area you want the AI to decorate.", "OK")->show();
        return;
    }

    // Build a runtime whitelist from the actual Geometry Dash object toolbox.
    // Only objects classified by the game as Decoration can be generated.
    m_allowedObjectIds.clear();
    std::ostringstream catalogue;
    auto toolbox = ObjectToolbox::sharedState();
    auto keys = toolbox ? toolbox->allKeys() : nullptr;
    std::unordered_set<int> seenIds;
    if (keys) {
        for (int i = 0; i < keys->count() && m_allowedObjectIds.size() < 80; ++i) {
            auto key = static_cast<CCString*>(keys->objectAtIndex(i));
            if (!key) continue;
            const char* idText = toolbox->frameToKey(key->getCString());
            if (!idText) continue;
            const int objectId = std::atoi(idText);
            if (objectId <= 0 || !seenIds.insert(objectId).second) continue;
            auto prototype = GameObject::createWithKey(objectId);
            if (!prototype || prototype->getType() != GameObjectType::Decoration) continue;
            m_allowedObjectIds.push_back(objectId);
            catalogue << objectId << ":" << key->getCString() << "; ";
        }
    }
    if (m_allowedObjectIds.empty()) {
        FLAlertLayer::create("Decoration Catalogue Unavailable",
            "Could not find safe decorative objects in the Geometry Dash object toolbox.",
            "OK")->show();
        return;
    }

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    for (const auto& objectRef : selected) {
        auto object = objectRef.data();
        if (!object) continue;
        const auto rect = object->getObjectRect();
        minX = std::min(minX, rect.getMinX());
        minY = std::min(minY, rect.getMinY());
        maxX = std::max(maxX, rect.getMaxX());
        maxY = std::max(maxY, rect.getMaxY());
    }
    if (minX == std::numeric_limits<float>::max()) {
        FLAlertLayer::create("Selection Error", "Could not calculate the selected area bounds.", "OK")->show();
        return;
    }
    if (maxX - minX < 60.f) { minX -= 30.f; maxX += 30.f; }
    if (maxY - minY < 60.f) { minY -= 30.f; maxY += 30.f; }
    m_areaMin = CCPoint(minX, minY);
    m_areaMax = CCPoint(maxX, maxY);

    auto mod = Mod::get();
    std::vector<AI::ProviderConfig> providers;
    auto addProvider = [&providers, mod](
        const std::string& id, const std::string& name,
        const std::string& endpointKey, const std::string& modelKey, const std::string& keyKey
    ) {
        auto endpoint = mod->getSettingValue<std::string>(endpointKey);
        auto model = mod->getSettingValue<std::string>(modelKey);
        auto apiKey = mod->getSettingValue<std::string>(keyKey);
        const bool secureEndpoint = endpoint.rfind("https://", 0) == 0;
        const bool localEndpoint =
            endpoint.rfind("http://localhost", 0) == 0 ||
            endpoint.rfind("http://127.0.0.1", 0) == 0;
        if (!endpoint.empty() && !model.empty() && !apiKey.empty() &&
            (secureEndpoint || localEndpoint)) {
            providers.push_back(AI::ProviderConfig{id, name, endpoint, model, apiKey, true});
        }
    };

    addProvider("primary", "Primary provider", "provider-1-endpoint", "provider-1-model", "provider-1-key");
    addProvider("backup", "Backup provider", "provider-2-endpoint", "provider-2-model", "provider-2-key");
    addProvider("fallback", "Fallback provider", "provider-3-endpoint", "provider-3-model", "provider-3-key");

    if (providers.empty()) {
        FLAlertLayer::create(
            "AI Provider Setup",
            "Configure at least one provider with an endpoint, model, and API key. "
            "Use HTTPS endpoints (HTTP is allowed only for localhost).",
            "OK"
        )->show();
        return;
    }

    AI::AIService::get()->setProviders(std::move(providers));
    m_isGenerating = true;
    setStatus("Contacting AI providers...");

    m_hasValidPlan = false;
    m_applyButton->setEnabled(false);
    const std::string requestPrompt =
        m_prompt + "\nSelected area bounds: " + std::to_string(maxX - minX) + " by " +
        std::to_string(maxY - minY) + " editor units.\n"
        "Verified decorative object catalogue (ID:frame): " + catalogue.str() + "\n"
        "Return ONLY JSON with style, mood, palette, layers, notes, placements. "
        "Each layer has name, purpose, elements, placement, color. "
        "Each placement has object_id, x, y, scale, rotation, color. "
        "Use 1-8 layers and 1-120 placements. object_id MUST be one of the catalogue IDs. "
        "x and y are normalized 0..1 within the selected area; scale is 0.25..3; rotation is -360..360; "
        "color must be #RRGGBB. Keep gameplay paths clear and distribute decoration naturally.";

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
            AI::DecorationPlan plan;
            std::string validationError;
            if (!AI::DecorationPlanParser::parse(response, self->m_allowedObjectIds, plan, validationError)) {
                self->m_hasValidPlan = false;
                self->setStatus("AI returned an invalid plan");
                FLAlertLayer::create("Invalid AI Plan", validationError, "Close")->show();
                return;
            }

            self->m_lastPlan = std::move(plan);
            self->m_hasValidPlan = true;
            if (self->m_applyButton) self->m_applyButton->setEnabled(true);
            self->setStatus("Plan validated - preview ready");
            auto summary = self->m_lastPlan.makeSummary();
            FLAlertLayer::create("AI Decoration Preview", summary, "Close")->show();
        }
    );
}


void AIDecorateLayer::onApply(CCObject*) {
    if (!m_hasValidPlan || !m_editorUI || m_lastPlan.placements.empty()) {
        FLAlertLayer::create("No Valid Plan", "Generate and review a valid AI plan first.", "OK")->show();
        return;
    }

    std::string serialized;
    std::size_t accepted = 0;
    for (const auto& placement : m_lastPlan.placements) {
        if (std::find(m_allowedObjectIds.begin(), m_allowedObjectIds.end(), placement.objectId) ==
            m_allowedObjectIds.end()) continue;

        auto object = GameObject::createWithKey(placement.objectId);
        if (!object || object->getType() != GameObjectType::Decoration) continue;

        const float x = m_areaMin.x + placement.x * (m_areaMax.x - m_areaMin.x);
        const float y = m_areaMin.y + placement.y * (m_areaMax.y - m_areaMin.y);
        object->setPosition(CCPoint(x, y));
        object->setScale(std::clamp(placement.scale, 0.25f, 3.0f));
        object->setRotation(std::clamp(placement.rotation, -360.0f, 360.0f));

        const auto rgb = static_cast<unsigned long>(std::stoul(placement.color.substr(1), nullptr, 16));
        object->setObjectColor(ccColor3B{
            static_cast<GLubyte>((rgb >> 16) & 0xFF),
            static_cast<GLubyte>((rgb >> 8) & 0xFF),
            static_cast<GLubyte>(rgb & 0xFF)
        });

        const auto saveString = object->getSaveString();
        serialized += saveString.c_str();
        serialized += ";";
        ++accepted;
    }

    if (accepted == 0 || serialized.empty()) {
        FLAlertLayer::create("Apply Failed", "No valid decorative objects could be created from this plan.", "OK")->show();
        return;
    }

    // Use Geometry Dash's native paste path so the operation is grouped as a normal editor action.
    auto pasted = m_editorUI->pasteObjects(gd::string(serialized.c_str()));
    if (!pasted || pasted->count() == 0) {
        FLAlertLayer::create("Apply Failed", "The editor could not paste the generated decoration.", "OK")->show();
        return;
    }

    m_editorUI->updateButtons();
    m_editorUI->updateObjectInfoLabel();
    m_hasValidPlan = false;
    if (m_applyButton) m_applyButton->setEnabled(false);
    setStatus("Decoration applied - use Undo to revert");
    FLAlertLayer::create("Decoration Applied",
        "Added " + std::to_string(pasted->count()) +
        " decorative objects. Use the editor's Undo command to revert the batch.",
        "Great")->show();
}

void AIDecorateLayer::setStatus(const std::string& status) {
    if (m_statusLabel) m_statusLabel->setString(status.c_str());
}

std::vector<geode::Ref<GameObject>> AIDecorateLayer::getSelectedObjects() {
    std::vector<geode::Ref<GameObject>> objects;
    if (!m_editorUI) return objects;

    auto selected = m_editorUI->getSelectedObjects();
    if (!selected) return objects;

    for (auto object : geode::cocos::CCArrayExt<GameObject, false>(selected)) {
        if (object) objects.emplace_back(object);
    }
    return objects;
}

} // namespace AutoDecoratorAI
