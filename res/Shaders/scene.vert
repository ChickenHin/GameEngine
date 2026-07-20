layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;
layout (location = 3) in mat4 aModel;

out vec3 Normal;
out vec3 WorldPos;
out vec2 Uv;
flat out int InstanceID;

layout(std140) uniform Camera
{
    mat4 Projection;
    mat4 View;
    vec3 Position;
} Cam;


void main() {
    InstanceID = gl_InstanceID;

    vec4 worldPos_ = aModel * vec4(aPosition, 1.0);
    WorldPos = worldPos_.xyz;

    mat3 normalMatrix = mat3(transpose(inverse(aModel)));
    Normal = normalMatrix * aNormal;

    Uv = aUv;

    gl_Position = Cam.Projection * Cam.View * worldPos_;
}