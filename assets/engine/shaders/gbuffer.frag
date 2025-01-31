#version 450
layout (location = 0) out vec4 renderBuffer;
layout (location = 1) out vec4 gPosition;
layout (location = 2) out vec4 gNormal;
layout (location = 3) out vec4 gAlbedo;
layout (location = 4) out vec4 gSpecular;
// layout (location = 5) out vec4 gEmit;

const struct {
    int Normal_None;
    int Normal_Tangeant;
    int Normal_Model;
} NormalMode = {
    0, 1, 3
};

in vec3 color;
in vec4 position;
in vec3 normal;
in vec2 texCoord;

uniform mat4 view;
uniform mat4 invView;
uniform mat4 model;
uniform mat3 model_normal;

uniform vec3 diffuse_const;
uniform vec3 specular_const;
uniform float roughness_const;
uniform vec3 emit_const;
uniform bool halflambert;
uniform vec3 custom_ambient;

uniform int normal_mode;

layout (binding = 0) uniform sampler2D diffuse_texture;
layout (binding = 1) uniform sampler2D specular_texture;
layout (binding = 2) uniform sampler2D emission_texture;
layout (binding = 3) uniform sampler2D normal_texture;
// uniform sampler2D texture_specular1;

void main() {
    gPosition = position;

    vec3 normal_sample = texture(normal_texture, texCoord).rgb * 2 - 1;
    vec3 normal_final;

    if (normal_mode == NormalMode.Normal_None) {
        normal_final = normalize(normal);
        // } else if (normal_mode == NormalMode.Normal_Model) {
        //     normal_final = model_normal * normalize(normal);
    } else {
        vec3 Q1  = dFdx(vec3(position));
        vec3 Q2  = dFdy(vec3(position));
        vec2 st1 = dFdx(vec2(texCoord));
        vec2 st2 = dFdy(vec2(texCoord));

        vec3 N  = normalize(normal);
        vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
        vec3 B  = cross(N, T);
        mat3 TBN = mat3(T, B, N);

        // normal_final = normalize(transpose(TBN) * (normal_sample * 2 - 1));
        // normal_final = normalize(cross(T, -B));
        normal_final = normalize(TBN * normal_sample);
    }

    gNormal = vec4(normal_final, 0);

    vec3 cameraPos = invView[3].xyz;// / invView[3].w;
    vec3 viewVec = position.xyz - cameraPos;
    float viewDistance = length(viewVec);
    viewVec /= viewDistance;

    float dnv = -dot(normal, viewVec);
    float fresnel = 1.125 - 0.45 / (max(0, dnv) + 0.4);
    // and the diffuse per-fragment color
    // vec3 albedo = has_texture ? texture(diffuseTexture, texCoord).xyz * albedo : albedo;

    int flags = (int(halflambert) << 2);

    vec4 diffuse_sample = texture(diffuse_texture, texCoord);// + vec4(0.5, 0.5, 0, 0);
    vec3 diffuse_final = fresnel * color * diffuse_const * diffuse_sample.rgb;
    // vec3 diffuse_final = fresnel * color * diffuse_const;

    // gAlbedo = vec4(diffuse_final , flags);
    gAlbedo = vec4(diffuse_sample.rgb , flags);

    vec4 specular_sample = texture(specular_texture, texCoord);
    vec3 specular_final = (1 - fresnel) * specular_const * specular_sample.rgb;
    // vec3 specular_final = (1 - fresnel) * specular_const;

    gSpecular = vec4(specular_final, roughness_const * specular_sample.a);
    // gSpecular = vec4(specular_final, roughness_const);

    vec4 emission_sample = texture(emission_texture, texCoord);
    vec3 custom_light = emit_const + emission_sample.rgb;
    // vec3 custom_light = emit_const;

    custom_light += custom_ambient * (diffuse_final + specular_final);

    renderBuffer = vec4(custom_light, 1);
}
