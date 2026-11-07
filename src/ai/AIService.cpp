#include "AIService.hpp"
#include "DecorationPlan.hpp"
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

void AIService::generateDecorationPlan(const std::string& userPrompt, std::vector<int> allowedObjectIds, Completion completion) {
    m_userPrompt = userPrompt;
    m_allowedObjectIds = std::move(allowedObjectIds);
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
    body["max_tokens"] = 3500;
    body["messages"] = matjson::Value::array();

    matjson::Value systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] =
        "You are an expert Geometry Dash level decorator. Return a valid, concise JSON design plan only. "
        "Use this schema: {\\"style\\":string,\\"palette\\":[string],\\"mood\\":string,"
        "\\"layers\\":[{\\"name\\":string,\\"purpose\\":string,\\"elements\\":[string],"
        "\\"placement\\":string,\\"color\\":string}],\\"notes\\":[string],"
        "\\"placements\\":[{\\"object_id\\":integer,\\"x\\":number,\\"y\\":number,"
        "\\"scale\\":number,\\"rotation\\":number,\\"color\\":\\"#RRGGBB\\"}]}. "
        "Only use object IDs explicitly present in the user's verified decoration catalogue. "
        "Coordinates x/y must be normalized from 0 to 1 within the selected area. "
        "Scale must be 0.25 to 3, rotation -360 to 360, and colors must be #RRGGBB. "
        "Return 1-120 placements. Preserve gameplay clearance. Never claim objects have already been placed.";

    matjson::Value userMessage;
    userMessage["role"] = "user";
    userMessage["content"] =
        "Create a practical decoration plan for the selected Geometry Dash level area. "
        "The selected-object count and the user's visual brief are included below. "
        "Avoid covering gameplay-critical paths. User brief: " + m_userPrompt;

    body["messages"].push_back(systemMessage);
    body["messages"].push_back(userMessage);

    geode::utils::web::WebRequest request;
    request.header("Content-Type", "application/json");
    request.header("Authorization", "Bearer " + provider.apiKey);
    request.bodyJSON(body);
    request.timeout(std::chrono::seconds(45));

    m_requestTask.spawn(
        request.post(provider.endpoint),
        [this](geode::utils::web::WebResponse response) {
            this->handleResponse(std::move(response));
        }
    );
}

void AIService::handleResponse(web::WebResponse response) {
    const int status = response.code();
    const std::string responseBody = response.string().unwrapOr("");

    if (response.ok()) {
        // OpenAI-compatible endpoints wrap generated text in choices[0].message.content.
        // Some compatible services return the plan JSON directly, so support both formats.
        std::string generatedText;
        auto envelopeResult = matjson::Value::parse(responseBody);
        if (envelopeResult) {
            auto envelope = envelopeResult.unwrapOr(matjson::Value{});
            if (envelope.isObject() && envelope["choices"].isArray() &&
                envelope["choices"].size() > 0) {
                const auto& choice = envelope["choices"][0];
                generatedText = choice["message"]["content"].asString().unwrapOr("");
                if (generatedText.empty()) {
                    generatedText = choice["text"].asString().unwrapOr("");
                }
            }
            else if (envelope.isObject() && envelope.contains("style") &&
                     envelope.contains("layers")) {
                generatedText = responseBody;
            }
        }

        if (!generatedText.empty()) {
            DecorationPlan validatedPlan;
            std::string validationError;
            if (!DecorationPlanParser::parse(generatedText, m_allowedObjectIds, validatedPlan, validationError)) {
                log::warn("AI provider returned an invalid decoration plan: {}", validationError);
                auto next = m_router.advanceAfterFailure(ProviderFailureKind::Unknown, status);
                if (next) {
                    sendCurrentRequest();
                    return;
                }
                if (m_completion) {
                    m_completion(false, {}, "All configured providers returned invalid plans. Last validation error: " +
                        validationError);
                }
                return;
            }

            m_router.markSuccess();
            if (m_completion) {
                m_completion(true, std::move(generatedText), "AI provider returned a validated decoration plan.");
            }
            return;
        }

        log::warn("AI provider returned HTTP success but no usable plan content.");
        auto next = m_router.advanceAfterFailure(ProviderFailureKind::Unknown, status);
        if (next) {
            sendCurrentRequest();
            return;
        }
        if (m_completion) {
            m_completion(false, {}, "The AI providers responded, but none returned a usable JSON plan.");
        }
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
