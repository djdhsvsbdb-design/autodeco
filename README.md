# AI Decorator 3.0 for Geometry Dash

A Geode mod that turns a natural-language decoration brief into a structured AI design plan.

## 3.0 work in progress

- Prompt-first editor interface; the old preset-driven Auto Deco generator is removed.
- Primary, backup, and fallback OpenAI-compatible providers with automatic failover for rate limits, quota exhaustion, network errors, unavailable models, and server errors.
- Normalizes standard chat-completions responses and direct JSON-plan responses.
- Validates AI output before preview: required fields, array types, string lengths, layer counts, palette limits, and response-size limits.
- Shows a readable preview of style, mood, palette, decoration layers, placement guidance, and notes.
- API keys remain in local Geode settings; never commit keys or paste them into public issues.

## Current limitation

The 3.0 branch does not yet apply generated decoration objects to the level. The AI plan is validated and previewed only. The next implementation stage is to translate the plan into a bounded set of verified Geometry Dash decoration objects, create them through the editor API, and register one undoable batch. This must be tested in the target Geometry Dash + Geode SDK version before being enabled.

## Provider setup

In the mod's Geode settings, configure endpoint, model, and API key for the primary provider. Configure the backup and fallback providers to enable multi-step failover. Only HTTPS endpoints are accepted, except localhost for local development. Endpoints must be OpenAI-compatible chat-completions URLs. Use HTTPS endpoints; keep keys private.

## Build

Requires Windows, CMake, Visual Studio C++ tools, Geode SDK 2.0+, and Geometry Dash 2.2081. The project must be built and tested against the installed SDK before release.

```sh
cmake -S . -B build
cmake --build build --config Release
```
