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

uniform vec3 diffuse_const;
uniform vec3 specular_const;
uniform float roughness_const;
uniform vec3 emit_const;
uniform bool halflambert;
uniform vec3 custom_ambient;

layout (binding = 0) uniform sampler2D diffuse_texture;
layout (binding = 1) uniform sampler2D specular_texture;
layout (binding = 2) uniform sampler2D emission_texture;
// uniform sampler2D texture_specular1;

void main() {
    gPosition = position;

    /* vec3 normal_sample = texture(normal_texture, texCoord); */
    /* mat3 normal_transform = ?; */

    vec3 normal_final;
    // if (normal_is_global) {
    //     mat3 normal_transform = ?;
    normal_final = /* normal_transform * */ normalize(normal);
    // } else {
    //     normal_final = normal_sample;
    // }

    gNormal = vec4(normalize(normal), 0);

    vec3 cameraPos = invView[3].xyz;// / invView[3].w;
    vec3 viewVec = position.xyz - cameraPos;
    float viewDistance = length(viewVec);
    viewVec /= viewDistance;

    float dnv = -dot(normal, viewVec);
    float fresnel = 1.125 - 0.45 / (max(0, dnv) + 0.4);
    // and the diffuse per-fragment color
    // vec3 albedo = has_texture ? texture(diffuseTexture, texCoord).xyz * albedo : albedo;

    int flags = (int(halflambert) << 2);

    // vec4 diffuse_sample = texture(diffuse_texture, texCoord);
    // vec3 diffuse_final = (1 - fresnel) * color * diffuse_const * diffuse_sample.rgb;
    vec3 diffuse_final = (1 - fresnel) * color * diffuse_const;

    gAlbedo = vec4(diffuse_final , flags);

    // vec4 specular_sample = texture(specular_texture, texCoord);
    // vec3 specular_final = fresnel * specular_const * specular_sample.rgb;
    vec3 specular_final = fresnel * specular_const;

    // gSpecular = vec4(specular_final, roughness_const * specular_sample.a);
    gSpecular = vec4(specular_final, roughness_const);

    // vec4 emission_sample = texture(emission_texture, texCoord);
    // vec3 custom_light = emit_const + emission_sample.rgb;
    vec3 custom_light = emit_const;

    custom_light += custom_ambient * diffuse_final + specular_final;

    renderBuffer = vec4(custom_light, 1);
}
