#include "ProviderRouter.hpp"
#include <algorithm>
#include <utility>
namespace AutoDecoratorAI::AI {
void ProviderRouter::setProviders(std::vector<ProviderConfig> providers) {
    m_providers = std::move(providers);
    m_attempted.clear();
    m_currentIndex.reset();
}
void ProviderRouter::beginRequest() {
    m_attempted.clear();
    m_currentIndex.reset();
    for (std::size_t i = 0; i < m_providers.size(); ++i) {
        const auto& p = m_providers[i];
        if (p.enabled && !p.endpoint.empty() && !p.model.empty()) {
            m_currentIndex = i;
            m_attempted.insert(p.id);
            return;
        }
    }
}
std::optional<ProviderAttempt> ProviderRouter::current() const {
    if (!m_currentIndex || *m_currentIndex >= m_providers.size()) return std::nullopt;
    return ProviderAttempt{m_providers[*m_currentIndex], m_attempted.size()};
}
std::optional<ProviderAttempt> ProviderRouter::advanceAfterFailure(
    ProviderFailureKind failure, int httpStatus
) {
    if (failure == ProviderFailureKind::InvalidRequest || httpStatus == 400 || httpStatus == 422) {
        m_currentIndex.reset();
        return std::nullopt;
    }
    if (!m_currentIndex) return std::nullopt;
    for (std::size_t i = *m_currentIndex + 1; i < m_providers.size(); ++i) {
        const auto& p = m_providers[i];
        if (!p.enabled || p.endpoint.empty() || p.model.empty() || m_attempted.contains(p.id)) continue;
        m_currentIndex = i;
        m_attempted.insert(p.id);
        return current();
    }
    m_currentIndex.reset();
    return std::nullopt;
}
void ProviderRouter::markSuccess() { m_currentIndex.reset(); }
bool ProviderRouter::hasRemainingProviders() const {
    for (const auto& p : m_providers) {
        if (p.enabled && !p.endpoint.empty() && !p.model.empty() && !m_attempted.contains(p.id)) return true;
    }
    return false;
}
std::size_t ProviderRouter::attemptedCount() const { return m_attempted.size(); }
ProviderFailureKind ProviderRouter::classifyFailure(int status, bool networkError) {
    if (networkError || status == 408) return ProviderFailureKind::NetworkError;
    if (status == 401 || status == 403) return ProviderFailureKind::InvalidCredentials;
    if (status == 402) return ProviderFailureKind::QuotaExceeded;
    if (status == 429) return ProviderFailureKind::RateLimited;
    if (status == 404) return ProviderFailureKind::ModelUnavailable;
    if (status >= 500 && status <= 599) return ProviderFailureKind::TemporaryServerError;
    if (status == 400 || status == 422) return ProviderFailureKind::InvalidRequest;
    return ProviderFailureKind::Unknown;
}
} // namespace AutoDecoratorAI::AI
