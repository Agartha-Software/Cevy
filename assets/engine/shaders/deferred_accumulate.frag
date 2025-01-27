#version 450

uniform mat4 view;
uniform mat4 invView;
uniform vec3 lightPosition;
uniform vec3 lightEnergy;
uniform float lightRadius;
uniform float lightRange;

uniform float width;
uniform float height;

uniform bool debug_draw;

in vec2 texCoord;

layout (binding = 1) uniform sampler2D gPosition;
layout (binding = 2) uniform sampler2D gNormal;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;

out vec4 fragColor;

void shade_light(
    out vec3 diffuse_light,
    out vec3 specular_light,
    vec3 normal,
    float dnv,
    vec3 energy,
    vec3 ray,
    float radius,
    vec3 viewVec,
    float roughness,
    float halflambert) {
    float lightDist = length(ray);
    ray /= lightDist;

    vec3 light = max(energy / (lightDist * lightDist), vec3(0));

    float lambert = dot(normal, -ray);

    // float halfLambert = lambert * 0.5 + 0.5;

    vec3 halfway = normalize(-ray - viewVec);

    float phong;

    float exponent = 1 + 1 / roughness;
    phong = max(0, lambert) * pow(max(0, dot(normal, halfway)), exponent * 2) * exponent / 2;
    // phong = max(0, lambert) * pow(max(0, dot(reflect(-ray, normal), -viewVec)), exponent) * exponent / 4;

    float hl = halflambert * 0.5;
    lambert = lambert * (1 - hl) + hl;
    diffuse_light = light * max(0, lambert);
    specular_light = light * phong;
}

void main() {
    vec2 screenCoord;
    // screenCoord = texCoord.xy;
    screenCoord = gl_FragCoord.xy / vec2(width, height);
    vec4 position = texture(gPosition, screenCoord);
    vec4 packed_normal = texture(gNormal, screenCoord);
    vec4 packed_albedo = texture(gAlbedo, screenCoord);
    vec4 packed_specular = texture(gSpecular, screenCoord);
    vec3 albedo = packed_albedo.rgb;
    vec3 normal = packed_normal.rgb;
    vec3 specular = packed_specular.rgb;
    float roughness = packed_specular.a;
    uint flags = uint(packed_albedo.a * 255);
    // float emit_ambient = float((flags & 2) >> 1);
    // float emit_illum = 1 - emit_ambient;
    float halflambert = float((flags & 4) >> 2);

    vec3 cameraPos = invView[3].xyz;// / invView[3].w;
    vec3 viewVec = position.xyz - cameraPos;
    float viewDistance = length(viewVec);
    viewVec /= viewDistance;
    float dnv = -dot(normal, viewVec);
    float fresnel = 1.125 - 0.45 / (max(0, dnv) + 0.4);
    vec3 ray = position.xyz - lightPosition;

    vec3 diffuse_light;

    vec3 specular_light;

    shade_light(diffuse_light,
        specular_light,
        normal,
        dnv,
        lightEnergy,
        ray,
        lightRadius,
        viewVec,
        roughness,
        halflambert);

    vec3 surface = diffuse_light * albedo * fresnel;
    surface += (1 - fresnel) * specular_light * specular;

    bool debug_draw_override = debug_draw;

    surface = mix(surface, lightEnergy * 0.01, float(debug_draw_override));

    fragColor = vec4(surface, 1);
}
