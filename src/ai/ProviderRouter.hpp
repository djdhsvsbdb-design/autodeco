#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>
namespace AutoDecoratorAI::AI {
struct ProviderConfig {
    std::string id, name, endpoint, model, apiKey;
    bool enabled = true;
};
enum class ProviderFailureKind {
    RateLimited, QuotaExceeded, TemporaryServerError, NetworkError,
    InvalidCredentials, ModelUnavailable, InvalidRequest, Unknown
};
struct ProviderAttempt {
    ProviderConfig provider;
    std::size_t attemptNumber = 0;
};
class ProviderRouter {
public:
    void setProviders(std::vector<ProviderConfig> providers);
    void beginRequest();
    std::optional<ProviderAttempt> current() const;
    std::optional<ProviderAttempt> advanceAfterFailure(ProviderFailureKind failure, int httpStatus = 0);
    void markSuccess();
    bool hasRemainingProviders() const;
    std::size_t attemptedCount() const;
    static ProviderFailureKind classifyFailure(int httpStatus, bool networkError);
private:
    std::vector<ProviderConfig> m_providers;
    std::unordered_set<std::string> m_attempted;
    std::optional<std::size_t> m_currentIndex;
};
} // namespace AutoDecoratorAI::AI
