#version 450

const uint TYPE_POINT = 1;
const uint TYPE_SPOT = 2;
const uint TYPE_SUN = 3;

uniform mat4 view;
uniform mat4 projector;
uniform mat4 invView;

uniform vec3 lightPosition;
uniform vec3 lightEnergy;
uniform vec3 lightDirection;
uniform float lightAngle;
uniform float lightRadius;
uniform float lightRange;
uniform float lightFade;
uniform uint lightType;

uniform float width;
uniform float height;

uniform bool debug_draw;

in vec2 texCoord;

layout (binding = 0) uniform sampler2D shadowMap;

layout (binding = 1) uniform sampler2D gPosition;
layout (binding = 2) uniform sampler2D gNormal;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;

layout (location = 0) out vec4 fragColor;

void shade_light_point(
    inout vec3 diffuse_light,
    inout vec3 specular_light,
    vec3 normal,
    float dnv,
    vec3 energy,
    vec3 ray,
    float lightDist,
    float radius,
    vec3 viewVec,
    float roughness,
    float halflambert) {

    vec3 light = max(energy / (lightDist * lightDist), vec3(0));

    float fade = pow(clamp((lightRange - lightDist) / (lightRange * lightFade), 0, 1), 2);

    light *= fade;

    float lambert = dot(normal, -ray);

    vec3 halfway = normalize(-ray - viewVec);

    float phong;

    float exponent = 1 + 1 / roughness;
    phong = max(0, lambert) * pow(max(0, dot(normal, halfway)), exponent * 2) * exponent / 2;
    // phong = max(0, lambert) * pow(max(0, dot(reflect(-ray, normal), -viewVec)), exponent) * exponent / 4;

    float hl = halflambert * 0.5;
    // lambert = lambert * (1 - hl) + hl;
    diffuse_light = light * max(0, lambert);
    specular_light = light * phong;
}

void shade_light_spot(
    inout vec3 diffuse_light,
    inout vec3 specular_light,
    vec3 normal,
    float dnv,
    vec3 energy,
    vec3 projectedCoords,
    vec3 ray,
    float lightDist,
    float radius,
    vec3 viewVec,
    float roughness,
    float halflambert) {

    vec3 light = max(energy / (lightDist * lightDist), vec3(0));

    vec3 lightDirection_override = normalize(lightDirection);
    // lightDirection_override = normalize((inverse(projector) * vec4(0, 0, 1, 1)).xyz);
    // vec3 lightDirection_override = normalize(- lightPosition);

    float off_angle = acos(dot(ray, lightDirection_override));

    float blend = max(0, 1 - (off_angle / lightAngle));

    light *= pow(blend, 2.0 * lightRadius);

    float fade = pow(clamp((lightRange - lightDist) / (lightRange * lightFade), 0, 1), 2);

    light *= fade;

    float depth = texture(shadowMap, projectedCoords.xy).x;
    float depth_delta = depth - projectedCoords.z;

    depth_delta = clamp(depth_delta * 10000 + 0.8 / projectedCoords.z, 0, 1) ;

    light *= depth_delta;

    float lambert = dot(normal, -ray);

    vec3 halfway = normalize(-ray - viewVec);

    float phong;

    float exponent = 1 + 1 / roughness;
    phong = max(0, lambert) * pow(max(0, dot(normal, halfway)), exponent * 2) * exponent / 2;
    // phong = max(0, lambert) * pow(max(0, dot(reflect(-ray, normal), -viewVec)), exponent) * exponent / 4;

    float hl = halflambert * 0.5;
    // lambert = lambert * (1 - hl) + hl;
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

    vec4 projected = projector * vec4(position.xyz + normal * 0.001, 1);
    projected /= projected.w;
    projected.xyz = projected.xyz * 0.5 + 0.5;

    vec3 cameraPos = invView[3].xyz;// / invView[3].w;
    vec3 viewVec = position.xyz - cameraPos;
    float viewDistance = length(viewVec);
    viewVec /= viewDistance;
    float dnv = -dot(normal, viewVec);
    vec3 ray = position.xyz - lightPosition;

    float lightDist = length(ray);
    ray /= lightDist;

    vec3 diffuse_light = vec3(0);

    vec3 specular_light = vec3(0);


    if (lightType == TYPE_SPOT) {
        shade_light_spot(diffuse_light,
            specular_light,
            normal,
            dnv,
            lightEnergy,
            projected.xyz,
            ray,
            lightDist,
            lightRadius,
            viewVec,
            roughness * roughness,
            halflambert);
    } else if (lightType == TYPE_POINT) {
        shade_light_point(diffuse_light,
            specular_light,
            normal,
            dnv,
            lightEnergy,
            ray,
            lightDist,
            lightRadius,
            viewVec,
            roughness * roughness,
            halflambert);
    }

    vec3 surface = vec3(0);
    surface += diffuse_light * albedo;
    surface += specular_light * specular;

    bool debug_draw_override = debug_draw;
    // debug_draw_override = true;

    surface = mix(surface, lightEnergy * 0.001, float(debug_draw_override) * 0.1);
    surface += lightEnergy * 0.001 * float(debug_draw_override);

    fragColor = vec4(surface, 0);
}
