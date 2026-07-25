#include <core/Log.hpp>
#include "GameObject.hpp"
#include "Material.hpp"
#include "Mesh.hpp"
#include <emath/emath.hpp>

#include <cmath>

GameObject::GameObject(emath::vec3 position, std::shared_ptr<Material> matt, std::shared_ptr<Mesh> mesh) noexcept
    : m_model(Transform(position))
    , m_Material(matt)
    , m_Mesh(mesh)
{}

GameObject::GameObject(Transform transform, std::shared_ptr<Material> matt, std::shared_ptr<Mesh> mesh) noexcept
    : m_model(transform)
    , m_Material(matt)
    , m_Mesh(mesh)
{}

GameObject::~GameObject()
{}

GameObject::GameObject(GameObject&& other) noexcept
    : m_model(std::move(other.m_model))
    , m_Material(other.m_Material)
    , m_Mesh(other.m_Mesh)  
{
    other.m_Material.reset();
    other.m_Mesh.reset();
}

auto GameObject::operator=(GameObject&& other) noexcept -> GameObject&
{
    if(this != &other){
        m_model = std::move(other.m_model);
        m_Material = std::move(other.m_Material);
        m_Mesh = std::move(other.m_Mesh);
    }
    return *this;
}

auto GameObject::transform() const -> Transform
{
    return Transform::from_mat4(m_model);
}

auto GameObject::model() const -> emath::mat4
{
    return m_model;
}

auto GameObject::set_position(const emath::vec3 &pos) -> void
{
    m_model = emath::translate(m_model, pos);
}

auto GameObject::set_scale(const emath::vec3 &Scale) -> void
{
    m_model = emath::scale(m_model, Scale);
}

auto GameObject::rotate(float angle, emath::vec3 axis) -> void // TODO: move this logic to emath::rotate(flaot angle, vec3 axis)
{
    axis = emath::vec3::normalize(axis);

    const float c = std::cos(angle);
    const float s = std::sin(angle);
    const float t = 1.0f - c;

    const float x = axis.x;
    const float y = axis.y;
    const float z = axis.z;

    emath::mat4 R {
        t*x*x + c,     t*x*y - s*z,   t*x*z + s*y,   0.0f,
        t*x*y + s*z,   t*y*y + c,     t*y*z - s*x,   0.0f,
        t*x*z - s*y,   t*y*z + s*x,   t*z*z + c,     0.0f,
        0.0f,          0.0f,          0.0f,          1.0f
    };

    m_model = R * m_model;
}

auto GameObject::mesh() const -> std::shared_ptr<Mesh>
{
    return m_Mesh;
}

auto GameObject::material() const -> std::shared_ptr<Material>
{
    return m_Material;
}
