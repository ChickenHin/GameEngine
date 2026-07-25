#pragma once
#include <memory>
#include <string>
#include <format>

#include <emath/vec3.hpp>
#include <emath/mat4.hpp>

#include "Transform.hpp"
#include <engine_export.h>

class ENGINE_EXPORT GameObject {
public:
    friend struct std::formatter<GameObject>;
    GameObject(emath::vec3 position, std::shared_ptr<class Material> matt, std::shared_ptr<class Mesh> mesh) noexcept;
    GameObject(Transform transform, std::shared_ptr<class Material> matt, std::shared_ptr<class Mesh> mesh) noexcept;
    ~GameObject();

    GameObject(const GameObject&) = default;
    auto operator=(const GameObject&) -> GameObject& = default;

    GameObject(GameObject&& other) noexcept;
    auto operator=(GameObject&& other) noexcept -> GameObject&;

    auto set_position(const emath::vec3 &pos)                   -> void ;
    auto set_scale(const emath::vec3 &Scale)                    -> void ;
    auto rotate(float angle, emath::vec3 axis)                 -> void ;
    auto transform() const                                   -> Transform ;
    auto model() const                                       -> emath::mat4 ;
    auto mesh() const                                        -> std::shared_ptr<class Mesh> ;
    auto material() const                                    -> std::shared_ptr<class Material> ;

private:
    emath::mat4 m_model;
    std::shared_ptr<class Material> m_Material;
    std::shared_ptr<class Mesh> m_Mesh;
};
