#version 450

in vec2 texCoord;

uniform mat4 view;
uniform mat4 invView;

uniform vec3 fog;
uniform float fog_far;
uniform vec3 ambientColor;

struct Light {
    vec4 position;
    vec3 color;
    float radius;
};

const int lightCount = 15;

layout (std140, binding = 1) uniform LightBlock {
    Light lights[lightCount];
};

uniform int activeLights;

uniform bool halflambert;
uniform bool has_texture;

layout (binding = 0) uniform sampler2D gPosition;
layout (binding = 1) uniform sampler2D gNormal;
layout (binding = 2) uniform sampler2D gAlbedo;
layout (binding = 3) uniform sampler2D gEmit;

out vec4 fragColor;
// NOLINTBEGIN

vec3 filmicToneMapping(vec3 color) {
    color = max(vec3(0.), color - vec3(0.004));
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

void shade_light(
    inout vec3 diffuse_light,
    inout vec3 specular_light,
    vec3 normal,
    float dnv,
    vec3 lightColor,
    vec3 lightVec,
    float lightRadius,
    vec3 viewVec,
    float exponent) {
    float lightDist = length(lightVec);
    lightVec /= lightDist;

    vec3 light = max(lightColor / (lightDist * lightDist), vec3(0));

    float lambert = dot(normal, lightVec);

    // float halfLambert = lambert * 0.5 + 0.5;

    vec3 halfway = normalize(lightVec - viewVec);

    float phong;

    phong = max(0, lambert) * pow(max(0, dot(normal, halfway)), exponent * 2) * exponent / 2;
    // phong = max(0, lambert) * pow(max(0, dot(reflect(-lightVec, normal), -viewVec)), exponent) * exponent / 4;

    float hl = float(halflambert) * 0.5;
    lambert = lambert * (1 - hl) + hl;
    diffuse_light += light * max(0, lambert);
    specular_light += light * phong;
}

void main() {
    vec4 position = texture(gPosition, texCoord);
    vec4 packed_normal = texture(gNormal, texCoord);
    vec4 packed_albedo = texture(gAlbedo, texCoord);
    vec3 albedo = packed_albedo.rgb;
    float exponent = 1 / packed_albedo.a;
    vec3 normal = packed_normal.rgb;
    vec3 specular_tint = packed_normal.aaa;
    vec4 packed_emit = texture(gEmit, texCoord);
    vec3 emit = packed_emit.rgb;
    uint emit_flags = floatBitsToUint(packed_emit.a);
    float emit_ambient = float((emit_flags & 1) >> 1);
    float emit_illum = 1 - emit_ambient;

    vec3 cameraPos = invView[3].xyz;// / invView[3].w;
    vec3 viewVec = position.xyz - cameraPos;
    float viewDistance = length(viewVec);
    viewVec /= viewDistance;
    float dnv = -dot(normal, viewVec);

    float fresnel = 1.125 - 0.45 / (max(0, dnv) + 0.4);

    vec3 diffuse_light = ambientColor + emit_ambient * emit;

    vec3 specular_light = ambientColor + emit_ambient * emit;

    for (int i = 0; i < lightCount; ++i) {
        vec3 lightVec = lights[i].position.xyz - position.xyz;

        shade_light(diffuse_light, specular_light, normal, dnv, float(i < activeLights) * lights[i].color, lightVec, 0, viewVec, exponent);
    }

    vec3 surface = diffuse_light * albedo * fresnel + (1 - fresnel) * specular_light * specular_tint;

    surface += emit_illum * emit;

    surface = mix(surface, fog, clamp(pow(position.w / fog_far, 0.5), 0, 1));
    surface = filmicToneMapping(surface);

    fragColor = vec4(surface, 1.0);
}
