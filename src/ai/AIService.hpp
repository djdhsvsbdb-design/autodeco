#pragma once
#include "ProviderRouter.hpp"
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <functional>
#include <string>

namespace AutoDecoratorAI::AI {

class AIService {
public:
    using Completion = std::function<void(bool success, std::string response, std::string message)>;

    static AIService* get();

    void setProviders(std::vector<ProviderConfig> providers);
    void generateDecorationPlan(const std::string& userPrompt, Completion completion);

private:
    AIService() = default;
    void sendCurrentRequest();
    void handleResponse(web::WebResponse response);

    ProviderRouter m_router;
    TaskHolder<web::WebResponse> m_requestTask;
    std::string m_userPrompt;
    Completion m_completion;
};

} // namespace AutoDecoratorAI::AI
