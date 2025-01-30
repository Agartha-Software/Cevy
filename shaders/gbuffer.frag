#version 450
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gEmit;

in vec3 color;
in vec4 position;
in vec4 v_position;
in vec3 normal;
in vec2 texCoord;

uniform vec3 albedo;
uniform vec3 specular_tint;
uniform float phong_exponent;
uniform bool halflambert;
uniform vec3 emit;
uniform bool emit_ambient;

layout (binding = 0) uniform sampler2D diffuseTexture;
// uniform sampler2D texture_specular1;

void main() {
    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(position.xyz, v_position.z);
    // also store the per-fragment normals into the gbuffer
    gNormal = vec4(normalize(normal), (specular_tint.x + specular_tint.y + specular_tint.z) / 3);
    // and the diffuse per-fragment color
    // vec3 albedo = has_texture ? texture(diffuseTexture, texCoord).xyz * albedo : albedo;

    gAlbedo = vec4(albedo /* * texture(diffuseTexture, texCoord).rgb */, 1 / phong_exponent);
    // store specular intensity in gAlbedo's alpha component
    int emit_mode = int(halflambert) | (int(emit_ambient) << 1);
    // gEmit = vec4(emit, emit_mode);
    gEmit = vec4(emit, emit_mode / 255.f);
}
