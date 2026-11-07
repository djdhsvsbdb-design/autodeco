# AI Decorator for Geometry Dash

A Geode mod that turns a natural-language decoration brief into an AI-generated design plan.

## Current workflow

1. Select the level objects/area in the Geometry Dash editor.
2. Open **AI Decorator**.
3. Enter a prompt describing the style, palette, mood, and details.
4. Choose one or two OpenAI-compatible providers in the mod's Geode settings.
5. Press **Generate AI Plan**. If the primary provider hits a quota, rate limit, network error, or supported server failure, the router tries the backup provider.
6. Review the returned plan.

## Provider setup

In Geode's settings for this mod, configure the endpoint, model, and API key for the primary provider. Optionally configure a backup provider. Endpoints must be OpenAI-compatible chat-completions URLs. Leave a provider field empty to disable that provider.

API keys are local mod settings and must never be committed to the repository.

## Important limitation

This version requests and previews an AI decoration plan. It does **not yet place generated objects into the level**. Safe application requires a strict plan parser, a verified Geometry Dash object-ID catalogue, bounds validation, and an undo-safe editor transaction. The old preset-driven procedural generator has been removed from the active build rather than being presented as AI.

## Build

Requires Windows, CMake, Visual Studio C++ tools, Geode SDK 2.0+, and Geometry Dash 2.2081.

```sh
cmake -S . -B build
cmake --build build --config Release
```
