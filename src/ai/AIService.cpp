#include "AIService.hpp"
#include <Geode/Geode.hpp>
#include <chrono>
#include <utility>

using namespace geode::prelude;

namespace AutoDecoratorAI::AI {

AIService* AIService::get() {
    static AIService instance;
    return &instance;
}

void AIService::setProviders(std::vector<ProviderConfig> providers) {
    m_router.setProviders(std::move(providers));
}

void AIService::generateDecorationPlan(const std::string& userPrompt, Completion completion) {
    m_userPrompt = userPrompt;
    m_completion = std::move(completion);
    m_router.beginRequest();

    if (!m_router.current()) {
        if (m_completion) m_completion(false, {}, "No enabled AI providers are configured.");
        return;
    }

    sendCurrentRequest();
}

void AIService::sendCurrentRequest() {
    auto attempt = m_router.current();
    if (!attempt) {
        if (m_completion) m_completion(false, {}, "All configured AI providers failed or are unavailable.");
        return;
    }

    const auto& provider = attempt->provider;
    matjson::Value body;
    body["model"] = provider.model;
    body["temperature"] = 0.35;
    body["messages"] = matjson::Value::array();

    matjson::Value systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] =
        "You are a Geometry Dash level decoration planner. Return a concise JSON decoration plan only. "
        "Do not invent unsupported object IDs. If object IDs are not supplied, describe decoration intent "
        "using semantic categories, positions relative to the selected geometry, colors, and layer roles.";

    matjson::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] =
        "Create a decoration plan for this Geometry Dash level selection. Prompt: " + m_userPrompt;

    body["messages"].push_back(systemMessage);
    body["messages"].push_back(userMessage);

    web::WebRequest request;
    request.header("Content-Type", "application/json");
    request.header("Authorization", "Bearer " + provider.apiKey);
    request.bodyJSON(body);
    request.timeout(std::chrono::seconds(45));

    m_requestTask.spawn(
        request.post(provider.endpoint),
        [this](web::WebResponse response) {
            this->handleResponse(std::move(response));
        }
    );
}

void AIService::handleResponse(web::WebResponse response) {
    const int status = response.code();
    const std::string responseBody = response.string().unwrapOr("");

    if (response.ok()) {
        m_router.markSuccess();
        if (m_completion) m_completion(true, responseBody, "AI provider responded successfully.");
        return;
    }

    const auto failure = ProviderRouter::classifyFailure(status, status == 0);
    auto next = m_router.advanceAfterFailure(failure, status);

    log::warn("AI provider request failed (HTTP {}). Switching provider if available.", status);

    if (next) {
        sendCurrentRequest();
        return;
    }

    if (m_completion) {
        m_completion(false, {}, "No AI provider could complete the request. Last HTTP status: " +
            std::to_string(status));
    }
}

} // namespace AutoDecoratorAI::AI
