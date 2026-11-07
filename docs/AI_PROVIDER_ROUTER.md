# AI provider routing

The AI Decorator 3.0 request router supports up to three configured OpenAI-compatible providers: primary, backup, and fallback.

## Request flow

1. The editor selection is measured to determine a bounded area.
2. The mod builds a whitelist from the installed game's ObjectToolbox and keeps only objects classified as `GameObjectType::Decoration`.
3. The prompt and verified catalogue are sent to the first enabled provider.
4. The response envelope is normalized to the generated text.
5. The full JSON plan is parsed and validated against the exact runtime object whitelist before the provider is accepted.
6. If the provider fails or returns an invalid plan, the router tries the next provider. Each provider is attempted at most once per generation request.
7. A valid plan is previewed. Nothing is inserted until the user presses **Apply Decoration**.

## Failover policy

- HTTP 429: try next provider (rate limit).
- HTTP 402: try next provider (quota exhausted / payment required).
- HTTP 401 or 403: skip provider (credentials/access issue).
- HTTP 404: skip provider (endpoint/model unavailable).
- HTTP 408 or network failure: try next provider.
- HTTP 5xx: try next provider.
- HTTP 400 or 422: stop; likely invalid request or unsupported schema.
- HTTP success with malformed envelope or invalid plan: try next provider.

Each provider is tried at most once. A valid response ends failover. The mod does not retry or duplicate an already-applied plan.

## Plan safety limits

- Maximum response size: 24 KB.
- 1-8 visual layers and 1-8 palette entries.
- 1-120 object placements.
- Object IDs must come from the live game's decorative-object whitelist.
- Coordinates are normalized to 0..1 within the selected area's bounds.
- Scale is clamped to 0.25..3.0; rotation to -360..360 degrees.
- Colors must be #RRGGBB.
- Non-decoration objects are rejected both during catalogue construction and immediately before serialization.

The generated objects are passed through the editor's native paste path. Undo behavior and placement offsets still require in-game verification on the target Geometry Dash/Geode build.

## Credentials and privacy

- Never commit API keys to Git or include them in logs.
- Store keys in local Geode mod settings only.
- Send a key only to its configured provider over HTTPS. Plain HTTP is accepted only for localhost development.
- Do not send level passwords, account credentials, or unrelated personal data.
- Send only the prompt, selected-area dimensions, and the decorative object catalogue needed to create a plan.

Local Geode settings are not a hardware-backed secret vault. Treat keys on a shared computer accordingly.
