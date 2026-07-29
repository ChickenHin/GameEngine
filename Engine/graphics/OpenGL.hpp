#pragma once
#include <string_view>
#include <utility>

#include "gl.inl"
#include <engine_export.h>

class CWindow;

class ENGINE_EXPORT OpenGL
{
    public:
        explicit OpenGL(const CWindow& window);
        OpenGL(const OpenGL& other) = delete;
        auto operator = (const OpenGL& other) -> OpenGL& = delete;
        ~OpenGL();

    public:
        auto window() const -> const CWindow&;
        auto config() const -> GL_CFG ;
        auto context() const -> GL_CTX ;
        auto version() const -> std::pair<int32_t, int32_t>;
        auto is_current() const -> bool;
        auto make_current()  -> bool;
    
        static auto find_config([[maybe_unused]] const CWindow& window) -> GL_CFG;

        constexpr static enum class API { CORE, ES } api =
        #if defined(CORE_GL)
            API::CORE;
        #elif defined(ES_GL)
            API::ES;
        #endif

        constexpr static int32_t MIN_REQUIRED_MAJOR_VERSION = 3;
        constexpr static int32_t MIN_REQUIRED_MINOR_VERSION = api == API::CORE ? 3 : 0;

        constexpr static bool   DEBUG = std::string_view{EG_BUILD_CONFIG} == "Debug" ? true : false;
        constexpr static size_t MSAA  = 2;
        constexpr static float ANISOTROPY = 2.0f;

    private:
        auto create_context() -> GL_CTX;
        auto enable_debug() const -> void;
        auto resolve_function(const char* name) -> void*;
        auto load_functions() -> void;
        auto init_max_members() -> void;
        auto check_extensions() -> void;

    private:
        const CWindow& m_Window;
        GL_CFG m_Config;
        GL_CTX m_Context;
        int32_t m_Major;
        int32_t m_Minor;

        inline static int32_t s_MAX_FRAGMENT_TEXTURE_UNITS{};
        inline static int32_t s_MAX_VERTEX_TEXTURE_UNITS{};
        inline static int32_t s_MAX_COMBINED_TEXTURE_UNITS{};
        inline static int32_t s_MAX_MSAA{};
        inline static float   s_MAX_ANISOTROPY{};
        inline static int32_t s_MAX_TEXTURE_SIZE{};
        inline static int32_t s_MAX_3D_TEXTURE_SIZE{};
        inline static int32_t s_MAX_CUBE_MAP_TEXTURE_SIZE{};
        inline static int32_t s_MAX_ARRAY_TEXTURE_LAYERS{};
        inline static int32_t s_MAX_VERTEX_UNIFORM_COMPONENTS{};
        inline static int32_t s_MAX_FRAGMENT_UNIFORM_COMPONENTS{};
        inline static int32_t s_MAX_UNIFORM_BLOCK_SIZE{};
        inline static int32_t s_MAX_UNIFORM_BUFFER_BINDINGS{};
        inline static int32_t s_MAX_COMBINED_UNIFORM_BLOCKS{};
        inline static int32_t s_MAX_RENDERBUFFER_SIZE{};
        inline static int32_t s_MAX_COLOR_ATTACHMENTS{};

    public:
        inline static const int32_t& MAX_FRAGMENT_TEXTURE_UNITS = s_MAX_FRAGMENT_TEXTURE_UNITS;
        inline static const int32_t& MAX_VERTEX_TEXTURE_UNITS = s_MAX_VERTEX_TEXTURE_UNITS;
        inline static const int32_t& MAX_COMBINED_TEXTURE_UNITS = s_MAX_COMBINED_TEXTURE_UNITS;
        inline static const int32_t& MAX_MSAA = s_MAX_MSAA;
        inline static const float&   MAX_ANISOTROPY = s_MAX_ANISOTROPY;
        inline static const int32_t& MAX_TEXTURE_SIZE = s_MAX_TEXTURE_SIZE;
        inline static const int32_t& MAX_3D_TEXTURE_SIZE = s_MAX_3D_TEXTURE_SIZE;
        inline static const int32_t& MAX_CUBE_MAP_TEXTURE_SIZE = s_MAX_CUBE_MAP_TEXTURE_SIZE;
        inline static const int32_t& MAX_ARRAY_TEXTURE_LAYERS = s_MAX_ARRAY_TEXTURE_LAYERS;
        inline static const int32_t& MAX_VERTEX_UNIFORM_COMPONENTS = s_MAX_VERTEX_UNIFORM_COMPONENTS;
        inline static const int32_t& MAX_FRAGMENT_UNIFORM_COMPONENTS = s_MAX_FRAGMENT_UNIFORM_COMPONENTS;
        inline static const int32_t& MAX_UNIFORM_BLOCK_SIZE = s_MAX_UNIFORM_BLOCK_SIZE;
        inline static const int32_t& MAX_UNIFORM_BUFFER_BINDINGS = s_MAX_UNIFORM_BUFFER_BINDINGS;
        inline static const int32_t& MAX_COMBINED_UNIFORM_BLOCKS = s_MAX_COMBINED_UNIFORM_BLOCKS;
        inline static const int32_t& MAX_RENDERBUFFER_SIZE = s_MAX_RENDERBUFFER_SIZE;
        inline static const int32_t& MAX_COLOR_ATTACHMENTS = s_MAX_COLOR_ATTACHMENTS;

    public:
        inline static bool is_GL_KHR_debug{false};
        inline static bool is_GL_ARB_debug_output{false};
        inline static bool is_GL_EXT_disjoint_timer_query{false};
        inline static bool is_GL_EXT_texture_filter_anisotropic{false};
        inline static bool is_GL_ARB_texture_filter_anisotropic{false};
};