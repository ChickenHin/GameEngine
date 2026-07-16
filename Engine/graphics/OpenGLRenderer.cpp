#include "OpenGLRenderer.hpp"

#include "Window.hpp"
#include "Camera.hpp"
#include "OpenGL.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include "ShaderProgram.hpp"
#include "emath/vec3.hpp"
#include "emath/uvec2.hpp"
#include "gl.hpp"

#include <core/Log.hpp>
#include <core/Exception.hpp>

#include <ui/Text.hpp>

#include <algorithm>
#include <ranges>
#include <chrono>
#include <cstring>
#include <cmath>
#include <cstddef>
#include <numeric>

namespace UBO {

    struct Camera
    {
        constexpr static int32_t BINDING_POINT = 0;

        alignas(16) emath::mat4 Projection;
        alignas(16) emath::mat4 View;
        emath::vec3 Position;
    };

    struct SunLight
    {
        constexpr static int32_t BINDING_POINT = 1;

        alignas(16) emath::vec3 Direction;
        alignas(16) emath::vec3 Color;
        alignas(16) emath::vec3 Ambient;
    };

    struct Globle
    {
        constexpr static int32_t BINDING_POINT = 2;

        alignas(8) emath::uvec2 ScreenResolution;
        alignas(4) uint32_t Frame;
        alignas(4) uint32_t Flags;
    };

}

OpenGLRenderer::OpenGLRenderer(const OpenGL& ctx, Text& text)
    : m_GApi(ctx)
    , m_DrawMode(DrawMode::Triangles)
    , m_Depth {
        std::make_shared<ShaderProgram>("res/shaders/depth.vert", "res/shaders/depth.frag", "Depth pre-pass")
    }
    , m_Scene {
        std::make_shared<ShaderProgram>("res/shaders/scene.vert", "res/shaders/scene.frag", "Scene"), 0
    }
    , m_SkyBox {
        std::make_shared<ShaderProgram>("res/shaders/skybox.vert", "res/shaders/skybox.frag", "SkyBox"),
        Texture::texture_cubemap("res/textures/forest.jpg"), 0
    }
    , m_Text {
        text,
        std::make_shared<ShaderProgram>("res/shaders/text.vert", "res/shaders/text.frag", "Text"),
        0, 0, 0, 0
    }
    , m_Stats()
    , m_Frame(0)
    , m_Gpu_time_elaped {
        {m_Scene.Program->name(), 0},
        {m_SkyBox.Program->name(), 0},
        {m_Text.Program->name(), 0}
    }
{
    gl::GenQueries(1, &m_Scene.time_elapsed);
    gl::GenQueries(1, &m_SkyBox.time_elapsed);
    gl::GenQueries(1, &m_Text.time_elapsed);

    set_depth(true);
    set_stencil(true);
    set_blend(true);
    set_face_cull(true);

    // Initialize buffers
    prepare_text_buffers();

    gl::GenTextures(1, &m_Text.Atlas);
    gl::BindTexture(GL_TEXTURE_2D, m_Text.Atlas);
    gl::label_texture(m_Text.Atlas, "Text Atlas");

    auto [w, h] = m_Text.Text.atlas_dims();
    std::vector<uint8_t> bitmap = m_Text.Text.bitmap(w,h);

    gl::TexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());

    // Set texture parameters
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Prepeare Camera UBO --------------------------------------------------------------------------
    ShaderProgram::create_ubo("Camera", sizeof(UBO::Camera));
    m_Depth.Program->attach_ubo("Camera");
    m_Scene.Program->attach_ubo("Camera");
    m_SkyBox.Program->attach_ubo("Camera");

    // Prepeare Sun UBO --------------------------------------------------------------------------
    ShaderProgram::create_ubo("SunLight", sizeof(UBO::SunLight));
    m_Scene.Program->attach_ubo("SunLight");
    
    // Prepeare Globle UBO --------------------------------------------------------------------------
    ShaderProgram::create_ubo("Globle", sizeof(UBO::Globle));
    m_Text.Program->attach_ubo("Globle");

    {
        // set texture uints
        m_Scene.Program->use();
        BATCH_SIZE = gl::get_intv(GL_MAX_TEXTURE_IMAGE_UNITS);
        static const std::vector<int32_t> units = []{
            std::vector<int32_t> a(BATCH_SIZE);
            std::iota(a.begin(), a.end(), 0);
            return a;
        }();
        m_Scene.Program->set_uniform("uDiffuseMaps[0]", units.data(), units.size());
    }
}

auto OpenGLRenderer::render(const Scene& scene) const -> void
{
    {
        auto& cam = scene.main_camera();

        // Uploading Camera UBO
        UBO::Camera data {
            .Projection = cam.projection(),
            .View = cam.view(),
            .Position = cam.position()
        };

        ShaderProgram::set_ubo("Camera", sizeof(UBO::Camera), &data);
    }

    {
        static auto start = std::chrono::steady_clock::now();

        float t = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - start
        ).count();

        float angle  = t * 0.5f;
        float height = std::sin(angle);
        height = std::max(height, 0.1f);

        // Uploading Sun UBO
        UBO::SunLight data {
            .Direction = emath::vec3(
                std::cos(angle),
                -height,
                std::sin(angle)
            ),
            .Color = emath::vec3(1.0f),
            .Ambient = emath::vec3(0.4f)
        };

        ShaderProgram::set_ubo("SunLight", sizeof(UBO::SunLight), &data);
    }

    {
        // Uploading Globle UBO
        UBO::Globle data {
            .ScreenResolution = emath::uvec2(m_GApi.window().dims().first, m_GApi.window().dims().second),
            .Frame = m_Frame,
            .Flags = 0
        };

        ShaderProgram::set_ubo("Globle", sizeof(UBO::Globle), &data);
    }

    gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // TODO: enable when im fragment-bound
    // gl::DepthMask(GL_TRUE);
    // gl::DepthFunc(GL_LESS);
    // depthpre_pass(scene);
    {
        gl::begin_query_time_elapsed(m_Scene.time_elapsed);
        // gl::DepthMask(GL_FALSE);
        // gl::DepthFunc(GL_LEQUAL);
        scene_pass(scene);
        gl::end_query_time_elapsed();
    }
    {
        gl::begin_query_time_elapsed(m_SkyBox.time_elapsed);
        gl::DepthMask(GL_FALSE);
        gl::DepthFunc(GL_LEQUAL);
        gl::Disable(GL_BLEND);
        skybox_pass();
        gl::end_query_time_elapsed();
    }
    {
        gl::begin_query_time_elapsed(m_Text.time_elapsed);
        gl::Enable(GL_BLEND);
        gl::DepthMask(GL_FALSE);
        gl::DepthFunc(GL_ALWAYS);
        text_pass();
        gl::end_query_time_elapsed();
    }
    gl::DepthMask(GL_TRUE);
    gl::DepthFunc(GL_LESS);

    gl::get_query_time_elapsed(m_Scene.time_elapsed, &m_Gpu_time_elaped[m_Scene.Program->name()]);
    gl::get_query_time_elapsed(m_SkyBox.time_elapsed, &m_Gpu_time_elaped[m_SkyBox.Program->name()]);
    gl::get_query_time_elapsed(m_Text.time_elapsed, &m_Gpu_time_elaped[m_Text.Program->name()]);

    m_Frame++;
}

auto OpenGLRenderer::depthpre_pass(const Scene& scene) const -> void
{
    gl::push_debug_group("depthpre_pass");

    gl::ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    m_Depth.Program->use();
    m_Stats.pipeline_switch++;

    Mesh* currentMesh = nullptr;

    for (const auto& obj : scene.entities())
    {
        auto mesh = obj.mesh().get();

        m_Depth.Program->set_uniform("Model", obj.model());

        if (currentMesh != mesh)
        {
            currentMesh = mesh;
            gl::BindVertexArray(mesh->VAO);
            m_Stats.mesh_switch++;
        }

        gl::DrawElements(GL_TRIANGLES, int32_t(mesh->indices_size()), GL_UNSIGNED_SHORT, (void*)0);
        m_Stats.draw_call++;
    }

    gl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    gl::pop_debug_group();
}

auto OpenGLRenderer::scene_pass(const Scene& scene) const -> void
{
    gl::push_debug_group("scene_pass");

    m_Stats.reset();

    auto Entities = scene.entities() | std::views::chunk_by(
        [](const GameObject& a, const GameObject& b)
        {
            return a.mesh() == b.mesh();
        }
    );

    m_Scene.Program->use();
    m_Stats.pipeline_switch++;

    Mesh* currentMesh = nullptr;
    
    std::vector<emath::mat4> modelMatrices;
    modelMatrices.reserve(BATCH_SIZE);

    for (auto group : Entities)
    {
        const auto& firstObj = *group.begin();
        Mesh* mesh = firstObj.mesh().get();

        if (currentMesh != mesh) {
            currentMesh = mesh;
            gl::BindVertexArray(currentMesh->VAO);
            m_Stats.mesh_switch++;
        }

        for (auto batch : group | std::views::chunk(BATCH_SIZE)){
            size_t instanceCount = std::ranges::distance(batch);
            if (instanceCount == 0) continue;

            for (int32_t texUnit = 0; const GameObject& obj : batch)
            {
                modelMatrices.push_back(obj.model());

                gl::ActiveTexture(GL_TEXTURE0 + texUnit);
                obj.material()->diffuse()->bind();
                texUnit++;
            }
            m_Stats.texture_switch++;

            m_Scene.Program->set_uniform("uModels[0]", modelMatrices.data(), int32_t(instanceCount));

            gl::DrawElementsInstanced(
                GL_TRIANGLES,
                int32_t(mesh->indices_size()),
                GL_UNSIGNED_SHORT,
                (void*)0,
                instanceCount
            );

            m_Stats.draw_call++;
            m_Stats.vertices += mesh->vertex_size() * instanceCount;
            m_Stats.indices  += mesh->indices_size() * instanceCount;
            modelMatrices.clear();
        }
    }

    gl::pop_debug_group();
}

auto OpenGLRenderer::skybox_pass() const -> void
{
    gl::push_debug_group("skybox_pass");


    m_SkyBox.Program->use();
    m_Stats.pipeline_switch++;

    gl::ActiveTexture(GL_TEXTURE0);
    m_SkyBox.Texture->bind();
    m_SkyBox.Program->set_uniform("uDiffuseMap", 0);
    m_Stats.texture_switch++;

    m_Stats.vertices += 3;
    // gl::BindVertexArray(VAO);
    gl::DrawArrays(GL_TRIANGLES, 0, 3);
    m_Stats.draw_call++;

    gl::pop_debug_group();
}

auto OpenGLRenderer::text_pass() const -> void {
    gl::push_debug_group("text_pass");

    auto [width, height] = m_GApi.window().dims();
    m_Text.Text.fill_text_buffer(width, height);

    m_Text.Program->use();
    m_Stats.pipeline_switch++;

    m_Text.Program->set_uniform("u_Color", Text::DEFAULT_FONT_COLOR);

    gl::ActiveTexture(GL_TEXTURE0);
    gl::BindTexture(GL_TEXTURE_2D, m_Text.Atlas);
    m_Text.Program->set_uniform("u_Texture", 0);

    gl::BindVertexArray(m_Text.VAO);
    gl::BindBuffer(GL_ARRAY_BUFFER, m_Text.VBO);
    m_Stats.texture_switch++;

    auto text_glyphs = m_Text.Text.glyphs();

    for (size_t offset = 0; offset < text_glyphs.size(); offset += TEXT_BATCH_SIZE)
    {
        auto batchCount = std::min(TEXT_BATCH_SIZE, text_glyphs.size() - offset);
        ptrdiff_t bytesToCopy = batchCount * sizeof(Text::Glyph);

        void* mappedMemory = gl::MapBufferRange(GL_ARRAY_BUFFER, 0, bytesToCopy, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        if (mappedMemory) {
            m_Stats.vertices += 4;
            std::memcpy(mappedMemory, text_glyphs.data() + offset, bytesToCopy);
            gl::UnmapBuffer(GL_ARRAY_BUFFER);
            gl::DrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<int32_t>(batchCount));
            m_Stats.draw_call++;
        }
    }

    m_Text.Text.clear_glyphs();
    m_Text.Text.clear();

    gl::pop_debug_group();
}

auto OpenGLRenderer::set_viewport(int32_t x, int32_t y, int32_t width, int32_t height) -> void
{
    gl::Viewport(x, y, width, height);
}

auto OpenGLRenderer::set_mode(DrawMode mode) -> void
{
    m_DrawMode = mode;
}

auto OpenGLRenderer::clear_screen(uint32_t buffersmask) const -> void
{
    gl::Clear((uint32_t)buffersmask);
}

auto OpenGLRenderer::stats() const -> RenderStats
{
    return m_Stats;
}

auto OpenGLRenderer::prepare_text_buffers() -> void {
    // Generate and bind VAO
    gl::GenVertexArrays(1, &m_Text.VAO);
    gl::BindVertexArray(m_Text.VAO);
    gl::label_vertex_array(m_Text.VAO, "Text VAO");

    // Dynamic instance VBO
    gl::GenBuffers(1, &m_Text.VBO);
    gl::BindBuffer(GL_ARRAY_BUFFER, m_Text.VBO);
    gl::BufferData(GL_ARRAY_BUFFER, TEXT_BATCH_SIZE * sizeof(Text::Glyph), nullptr, GL_STREAM_DRAW);
    gl::label_buffer(m_Text.VBO, "Text VBO");

    // Offset (2 * 4 byte)
    gl::EnableVertexAttribArray(0);
    gl::VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Text::Glyph), (void*)offsetof(Text::Glyph, offset));
    gl::VertexAttribDivisor(0, 1);

    // TexRect (4 * 2 byte)
    gl::EnableVertexAttribArray(1);
    gl::VertexAttribIPointer(1, 4, GL_UNSIGNED_SHORT, sizeof(Text::Glyph), (void*)offsetof(Text::Glyph, texRect));
    gl::VertexAttribDivisor(1, 1);
}

auto  OpenGLRenderer::set_depth(bool v) const -> void
{
    if (v) {
        gl::Enable(GL_DEPTH_TEST);
    } else gl::Disable(GL_DEPTH_TEST);
}

auto  OpenGLRenderer::set_stencil(bool v) const -> void
{
    if (v) {
        gl::Enable(GL_STENCIL_TEST);
        gl::StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    } else gl::Disable(GL_STENCIL_TEST);
}

auto  OpenGLRenderer::set_blend(bool v) const -> void
{
    if (v) {
        gl::Enable(GL_BLEND);
        gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else gl::Disable(GL_BLEND);

}

auto  OpenGLRenderer::set_face_cull(bool v) const -> void
{
    if (v){ 
        gl::Enable(GL_CULL_FACE);
        gl::CullFace(GL_BACK);
    }  else gl::Disable(GL_CULL_FACE);
}

auto OpenGLRenderer::batch_size() const -> int32_t
{
    return BATCH_SIZE;
}

auto OpenGLRenderer::gpu_time_elapsed() const -> std::unordered_map<std::string, uint64_t>&
{
    return m_Gpu_time_elaped;
}