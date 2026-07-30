#pragma once

#include "Window.hpp"
#include "OpenGL.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"

#include <core/Log.hpp>

#include <inputs/Keyboard.hpp>
#include <inputs/Mouse.hpp>

#include <ui/Text.hpp>

#include <engine_export.h>

#include <memory>

class ENGINE_EXPORT IGame {
public:
    virtual ~IGame();
    virtual auto update(float dt) -> void;
    virtual auto on_deltamouse(float dx, float dy) -> void;

    ::Scene Scene;
};

class ENGINE_EXPORT APP
{
public:
    APP();
    auto run() -> void ;
    auto fps() const -> float;
    auto stable_fps() const -> float;
    auto set_game(IGame* g) -> void;

    static auto frame(void* ctx) -> void;

private:
    auto event_dispatch() -> void;
    auto process_events() -> void;
    auto input_update() -> void;
    auto render() -> void;
    auto render_flush() -> void;
    auto game_update() -> void;
    auto swap_buffers() -> void;
    auto draw_metrics_stats() -> void;
    auto draw_cpu_timelapsed() -> void;
    auto draw_gpu_timelapsed() -> void;
    auto debug_overlay() -> void;

private:
    bool m_Running;
    float m_dt;

public:
    ::CWindow Window;
    ::Keyboard Keyboard;
    ::Mouse Mouse;
    ::Text UiText;

private:
    OpenGL m_GApi;
    std::unique_ptr<IRenderer> Renderer;

    IGame* Game;
};