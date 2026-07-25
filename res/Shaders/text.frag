
in vec2 v_TexCoord;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec3 u_Color;

layout(std140) uniform Globle
{
    uvec2 ScreenResolution;
    uint Frame;
    uint Flags;
};

void main() {
    vec2 atlasSize = vec2(textureSize(u_Texture, 0));
    vec2 uv = v_TexCoord / atlasSize;
    float glyph = texture(u_Texture, uv).r;

    FragColor = vec4(u_Color, glyph);
}