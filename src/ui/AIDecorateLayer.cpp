#include "AIDecorateLayer.hpp"
#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>
#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

using namespace geode::prelude;

namespace AutoDecoratorAI {
namespace {

bool isLoopbackHttpEndpoint(const std::string& endpoint) {
    constexpr std::string_view prefix = "http://";
    if (endpoint.rfind(prefix, 0) != 0) return false;

    const auto authorityStart = prefix.size();
    const auto authorityEnd = endpoint.find_first_of("/?#", authorityStart);
    const auto authority = endpoint.substr(
        authorityStart,
        authorityEnd == std::string::npos ? std::string::npos : authorityEnd - authorityStart
    );
    if (authority.empty() || authority.find('@') != std::string::npos ||
        authority.find('\\') != std::string::npos) return false;

    std::string host;
    if (authority.front() == '[') {
        const auto close = authority.find(']');
        if (close == std::string::npos) return false;
        host = authority.substr(1, close - 1);
        if (close + 1 < authority.size() && authority[close + 1] != ':') return false;
    }
    else {
        const auto colon = authority.find(':');
        host = authority.substr(0, colon);
    }

    std::transform(host.begin(), host.end(), host.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return host == "localhost" || host == "127.0.0.1" || host == "::1";
}

struct CatalogueEntry {
    int objectId;
    std::string frame;
};

} // namespace


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

    m_promptInput = TextInput::create(width - 60.0f, "Describe the decoration style...", "bigFont.fnt");
    m_promptInput->setMaxCharCount(1500);
    m_promptInput->setTextAlign(TextInputAlign::Left);
    m_promptInput->setPosition(width / 2, height - 122.0f);
    m_promptInput->setScale(0.82f);
    m_promptInput->setCallback([this](const std::string& value) {
        m_prompt = value;
        m_hasValidPlan = false;
        if (m_applyButton) m_applyButton->setEnabled(false);
        std::string preview = m_prompt;
        if (preview.empty()) preview = "No prompt set";
        else if (preview.size() > 52) preview = preview.substr(0, 49) + "...";
        if (m_promptLabel) m_promptLabel->setString(preview.c_str());
        setStatus(m_prompt.empty() ? "Enter a prompt to begin" : "Prompt updated");
    });
    panel->addChild(m_promptInput);

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
        "Review the preview before applying. Verify Undo before saving.",
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
    m_isVisible = true;
}

void AIDecorateLayer::hideOverlay() {
    if (!m_overlayPanel) return;
    if (m_promptInput) m_promptInput->defocus();
    m_overlayPanel->stopAllActions();
    m_overlayPanel->setVisible(false);
    m_isVisible = false;
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

    // ObjectToolbox::allKeys() is not implemented in the public 2.2081
    // decompilation. The populated m_allKeys map is the runtime source of truth.
    // Read it directly and retain only objects the game classifies as Decoration.
    constexpr std::size_t maxCatalogueEntries = 160;
    m_allowedObjectIds.clear();
    std::vector<CatalogueEntry> candidates;
    auto toolbox = ObjectToolbox::sharedState();
    if (toolbox) {
        candidates.reserve(toolbox->m_allKeys.size());
        for (auto const& [objectId, frameName] : toolbox->m_allKeys) {
            if (objectId <= 0 || frameName.empty()) continue;
            auto prototype = GameObject::createWithKey(objectId);
            if (!prototype || prototype->m_objectType != GameObjectType::Decoration) continue;
            candidates.push_back({objectId, frameName.c_str()});
        }
    }

    // Keep a deterministic, evenly-spread sample so newer IDs are represented
    // without sending an enormous catalogue in every AI request.
    std::ostringstream catalogue;
    const std::size_t selectedCount = std::min(maxCatalogueEntries, candidates.size());
    for (std::size_t i = 0; i < selectedCount; ++i) {
        const std::size_t index = selectedCount == candidates.size()
            ? i
            : (selectedCount == 1 ? 0 : i * (candidates.size() - 1) / (selectedCount - 1));
        const auto& entry = candidates[index];
        m_allowedObjectIds.push_back(entry.objectId);
        catalogue << entry.objectId << ":" << entry.frame << "; ";
    }

    if (m_allowedObjectIds.empty()) {
        FLAlertLayer::create("Decoration Catalogue Unavailable",
            "The game did not expose any usable decorative objects. Check that the editor assets finished loading, then try again.",
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
        const bool localEndpoint = isLoopbackHttpEndpoint(endpoint);
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
    AI::AIService::get()->generateDecorationPlan(requestPrompt, m_allowedObjectIds,
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
        FLAlertLayer::create("Nothing to Apply", "Generate and review a valid decoration plan first.", "OK")->show();
        return;
    }

    std::string serializedObjects;
    for (const auto& placement : m_lastPlan.placements) {
        const float x = m_areaMin.x + placement.x * (m_areaMax.x - m_areaMin.x);
        const float y = m_areaMin.y + placement.y * (m_areaMax.y - m_areaMin.y);
        auto object = GameObject::createWithKey(placement.objectId);
        if (!object) continue;
        object->setPosition(CCPoint(x, y));
        object->setScale(placement.scale);
        object->setRotation(placement.rotation);
        object->m_baseColor = ccColor3B(
            static_cast<GLubyte>(placement.color.r),
            static_cast<GLubyte>(placement.color.g),
            static_cast<GLubyte>(placement.color.b)
        );
        serializedObjects += object->getSaveString(m_editorUI->getGameLayer());
        serializedObjects += ";";
    }

    if (serializedObjects.empty()) {
        FLAlertLayer::create("Apply Failed", "The generated plan did not contain any usable objects.", "OK")->show();
        return;
    }

    m_editorUI->pasteObjects(serializedObjects, true, false);
    m_hasValidPlan = false;
    m_applyButton->setEnabled(false);
    setStatus("Decoration pasted into editor");
    FLAlertLayer::create("Decoration Applied", "The generated decoration was pasted into the level. Use Undo if anything looks wrong.", "OK")->show();
}
