#include <chrono>
#include <variant>
#include <thread>
#include <cmath>

#include "APP.hpp"
#include "Window.hpp"
#include "OpenGL.hpp"
#include "OpenGLRenderer.hpp"
#include "gl.hpp"

#include <core/SysInfo.hpp>
#include <core/Log.hpp>
#include <core/Event.hpp>

#include <inputs/Keyboard.hpp>
#include <inputs/Mouse.hpp>

constexpr auto WINDOW_WIDTH = 1180;
constexpr auto WINDOW_HEIGHT = 640;

IGame::~IGame() = default;
auto IGame::update(float dt) -> void { logg::trace("update(delta: {})", dt); }
auto IGame::on_deltamouse(float dx, float dy) -> void { logg::trace("on_deltamouse(dx: {}, dy:{})", dx, dy); }

APP::APP()
    : m_Running(true)
    , m_dt(0.0f)
    , Window(WINDOW_WIDTH, WINDOW_HEIGHT, "")
    , Keyboard()
    , Mouse()
    , UiText()
    , m_GApi(Window)
    , Renderer(std::make_unique<OpenGLRenderer>(m_GApi, UiText))
    , Game()
{
    Window.show();
    Window.set_vsync(false);
}

auto APP::set_game(IGame* g) -> void
{
    Game = g;
}

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

auto APP::frame(void* ctx) -> void
{
    auto app = static_cast<APP*>(ctx);

    // --- frame time ---
    static auto lastTime = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    app->m_dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    {
        auto start = std::chrono::steady_clock::now();
        app->Window.poll_events();
        app->event_dispatch();
        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["event_dispatch"] = std::chrono::duration<float, std::milli>(end - start).count();
    }

    {
        auto start = std::chrono::steady_clock::now();

        // TODO move to the visit
        // Fullscreen 
        if(app->Keyboard.is_pressed(Key::F11)){
            app->Window.toggle_fullscreen();
        }

        // Lock Mouse
        if(app->Keyboard.is_pressed(Key::L) ){
            static bool on = false;
            if(!on){
                app->Mouse.lock(app->Window);
                on = true;
            }else{
                app->Mouse.unlock();
                on = false;
            }
        }

        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["custom events"] = std::chrono::duration<float, std::milli>(end - start).count();
    }

    {
        auto start = std::chrono::steady_clock::now();
        app->Renderer->render(app->Game->Scene);
        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["render"] = std::chrono::duration<float, std::milli>(end - start).count();
    }

    {
        auto start = std::chrono::steady_clock::now();
        app->debug_overlay();
        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["debug_overlay"] = std::chrono::duration<float, std::milli>(end - start).count();
    }

    {
        auto start = std::chrono::steady_clock::now();
        gl::Flush();
        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["glFlush"] = std::chrono::duration<float, std::milli>(end - start).count();
    }

    {
        auto start = std::chrono::steady_clock::now();
        app->Game->update(std::min(app->m_dt, 0.1f));
        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["game_update"] = std::chrono::duration<float, std::milli>(end - start).count();
    }

    {
        auto start = std::chrono::steady_clock::now();
        app->Window.swap_buffers();
        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["swap_buffers"] = std::chrono::duration<float, std::milli>(end - start).count();
    }

    {
        auto start = std::chrono::steady_clock::now();
        app->input_update();
        auto end = std::chrono::steady_clock::now();
        app->m_Cpu_time_elaped["input_update"] = std::chrono::duration<float, std::milli>(end - start).count();
    }
}

auto APP::event_dispatch() -> void
{
    Event event;
    while (EventQ::self().pull(event)) {
        std::visit( overloaded {
            [this](const CWindow::QuitEvent&) {

                auto ret = Window.message_box("Exit", "Are You Sure !");
                if(ret) {
                    m_Running = false;
                }
            },
            [this](const CWindow::ResizeEvent& e) {
                Renderer->set_viewport(0, 0, e.width, e.height);
            },
            [this](const CWindow::LoseFocusEvent&) {
		        Keyboard.clear_state();
		        Mouse.clear_state();
		        EventQ::self().clear();
            },
            [this](const Keyboard::KeyDownEvent& e) {
				Keyboard.on_key_down(e.key);
            },
            [this](const Keyboard::KeyUpEvent& e) {
				Keyboard.on_key_up(e.key);
            },
            [this](const Mouse::ButtonDownEvent& e) {
                Mouse.button_down(e.btn);
            },
            [this](const Mouse::ButtonUpEvent& e) {
                Mouse.button_up(e.btn);
            },
            [this](const Mouse::EnterEvent&) {
                Mouse.mouse_entered();
            },
            [this](const Mouse::LeaveEvent&) {
                Mouse.mouse_leaved();
            },
            [this](const Mouse::MoveEvent& e) {
                Mouse.mouse_moved(e.x, e.y);
            },
            [this](const Mouse::MovementEvent& e) {
                Mouse.rawdelta(e.dx, e.dy);
                auto [dx, dy] = Mouse.get_rawdelta();
                Game->on_deltamouse(dx, dy);                
            },
            [](const auto& e) {
                logg::warn("Unhandeled Event: {}", typeid(e).name()); 
            }
        }, event
        );
    }
}

auto APP::draw_metrics_stats() -> void
{
    UiText.draw(std::format("Fps         : {:.0f}", stable_fps()));
    UiText.draw(std::format("Res         : {}x{}", Window.dims().first, Window.dims().second));
    UiText.draw(std::format("Memory      : {}/{} MB", os::memory_usage(), os::memory_peak()));
    UiText.draw(std::format("Threads     : {}", std::thread::hardware_concurrency()));
    UiText.draw(std::format("Unique Mesh : {}", Renderer->stats().unique_mesh));
    UiText.draw(std::format("Draw Call   : {}", Renderer->stats().draw_call));
    UiText.draw(std::format("Vertices    : {}", Renderer->stats().vertices));
    UiText.draw(std::format("Indices     : {} ({} tri)\n", Renderer->stats().indices, Renderer->stats().indices/3));
}

auto APP::draw_cpu_timelapsed() -> void
{
    static float accumulated = 1.0f;
    accumulated += m_dt;

    static std::string s_elp_time;
    static std::string elp_time;

    if (accumulated >= 1.0f) {
        float totale_elp_time{};
        elp_time += "CPU Time Elapsed:\n";
        for(auto& [n, v] : m_Cpu_time_elaped){
            elp_time += std::format("\t{} : {:.4f} ms\n", n, v);
            totale_elp_time += v;
        }
        elp_time += std::format("\ttotale: [{:.4f} ms] ({})\n", totale_elp_time, std::round(1000.0f/totale_elp_time));
        s_elp_time = std::move(elp_time);
        accumulated = 0.0f;
    }

    if (!s_elp_time.empty())
        UiText.draw(s_elp_time);
}

auto APP::draw_gpu_timelapsed() -> void
{
    std::string elp_time = "GPU Time Elapsed:\n";
    float totale_elp_time{};
    for(auto& [n, v] : Renderer->gpu_time_elapsed()){
        float ms = static_cast<float>(v) / 1'000'000.0f;
        elp_time += std::format("\t{} : {:.4f} ms\n", n, ms);
        totale_elp_time += ms;
    }
    elp_time += std::format("\ttotale: [{:.4f} ms] ({})\n", totale_elp_time, std::round(1000.0f/totale_elp_time));
    UiText.draw(elp_time);
}

auto APP::debug_overlay() -> void
{
    static bool on = false;
    if(Keyboard.is_pressed(Key::H) ){
        on = !on ? true : false;
    }

    if(on) {
        draw_metrics_stats();
        draw_cpu_timelapsed();
        draw_gpu_timelapsed();
    }
}

auto APP::input_update() -> void
{
    Keyboard.save_prev_state();
    Mouse.save_prev_state();
}

auto APP::fps() const -> float
{
    return 1.0f/m_dt;
}

auto APP::stable_fps() const -> float
{
    static float fps = 0;
    static float accumulated = 0.0f;

    accumulated += m_dt;

    if (accumulated >= 1.0f) {
        fps = 1.0f/m_dt;
        accumulated = 0.0f;
    }

    return fps;
}