#version 450
layout (location = 0) out vec4 renderBuffer;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gSpecular;
// layout (location = 5) out vec4 gEmit;

in vec3 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 view;
uniform mat4 invView;

uniform vec3 albedo;
uniform vec3 specular;
uniform float roughness;
uniform vec3 emit;
uniform bool halflambert;
uniform vec3 custom_ambient;

layout (binding = 0) uniform sampler2D diffuse_texture;
layout (binding = 1) uniform sampler2D specular_texture;
layout (binding = 2) uniform sampler2D emission_texture;
// uniform sampler2D texture_specular1;

void main() {
    gPosition = position;
    gNormal = vec4(normalize(normal), 0);
    // and the diffuse per-fragment color
    // vec3 albedo = has_texture ? texture(diffuseTexture, texCoord).xyz * albedo : albedo;

    int flags = (int(halflambert) << 2);
    gAlbedo = vec4(color * albedo /* * texture(diffuseTexture, texCoord).rgb */, flags);
    gSpecular = vec4(specular, roughness);

    vec3 cameraPos = invView[3].xyz;// / invView[3].w;
    vec3 viewVec = position.xyz - cameraPos;
    float viewDistance = length(viewVec);
    viewVec /= viewDistance;
    float dnv = -dot(normal, viewVec);

    float fresnel = 1.125 - 0.45 / (max(0, dnv) + 0.4);

    vec3 custom_light = emit;
    custom_light += custom_ambient * mix(albedo, specular, fresnel);

    renderBuffer = vec4(custom_light, 1);
}
