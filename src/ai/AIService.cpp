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
    body["temperature"] = 0.25;
    body["max_tokens"] = 1800;
    body["messages"] = matjson::Value::array();

    matjson::Value systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] =
        "You are an expert Geometry Dash level decorator. Return a valid, concise JSON design plan only. "
        "Use this schema: {\\"style\\":string,\\"palette\\":[string],\\"mood\\":string,"
        "\\"layers\\":[{\\"name\\":string,\\"purpose\\":string,\\"elements\\":[string],"
        "\\"placement\\":string,\\"color\\":string}] ,\\"notes\\":[string]}. "
        "Do not invent Geometry Dash object IDs or claim that objects were placed in the level. "
        "Describe visual intent, relative positions, colors, layering, and how decoration follows the selected geometry.";

    matjson::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] =
        "Create a practical decoration plan for the selected Geometry Dash level area. "
        "The selected-object count and the user's visual brief are included below. "
        "Avoid covering gameplay-critical paths. User brief: " + m_userPrompt;

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
