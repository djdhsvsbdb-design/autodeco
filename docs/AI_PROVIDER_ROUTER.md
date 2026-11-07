# AI provider routing

This is the first provider-selection/failover layer for Ail Deco.

## Request flow
1. The prompt planner receives the user's decoration prompt and a compact description of selected level geometry.
2. The router selects the first enabled provider with an endpoint and model configured.
3. A provider-specific HTTP adapter sends the request using that provider's documented API format.
4. On success, the response is parsed and validated before objects are created in the editor.
5. On provider-specific failures, the router advances to the next configured provider. Each provider is tried at most once per generation request.

## Failover policy
- HTTP 429: try next provider (rate limit).
- HTTP 402: try next provider (quota exhausted / payment required).
- HTTP 401 or 403: skip provider (credentials/access issue).
- HTTP 404: skip provider (endpoint/model unavailable).
- HTTP 408 or network failure: try next provider.
- HTTP 5xx: try next provider.
- HTTP 400 or 422: stop; likely invalid request or unsupported schema.

A successful response ends failover. The mod must not duplicate an already-applied decoration plan.

## Credentials and privacy
- Never commit API keys to Git or include them in logs.
- Store keys in local user configuration only; send a key only to its selected provider.
- Do not send level passwords, account credentials, or unrelated personal data.
- Send only the prompt and minimum geometry data needed to create a decoration plan.
- The user must be able to disable a provider and remove its local key.

Local Geode settings are not a hardware-backed secret vault. Treat keys on a shared computer accordingly.

## Implementation boundary
ProviderRouter is only the provider selection/failover policy. HTTP adapters, provider-specific request/response parsers, prompt-to-decoration schema, and editor application transaction still need to be connected and tested before end-to-end functionality can be claimed.
