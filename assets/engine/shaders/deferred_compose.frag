#version 450

uniform mat4 view;
uniform mat4 invView;

uniform vec3 ambientColor;
uniform vec3 fog;
uniform float fog_far;

uniform float width;
uniform float height;

in vec2 texCoord;

layout (binding = 0) uniform sampler2D renderBuffer;
layout (binding = 1) uniform sampler2D gPosition;
layout (binding = 2) uniform sampler2D gNormal;
layout (binding = 3) uniform sampler2D gAlbedo;
layout (binding = 4) uniform sampler2D gSpecular;

layout (location = 0) out vec4 fragColor;

uniform float exposure;

vec3 filmicToneMapping(vec3 color) {
    color = max(vec3(0.), color - vec3(0.004));
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
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
    float emit_ambient = float((flags & 2) >> 1);
    float emit_illum = 1 - emit_ambient;
    float halflambert = float((flags & 4) >> 2);

    vec3 cameraPos = invView[3].xyz;// / invView[3].w;
    vec3 viewVec = position.xyz - cameraPos;
    float viewDistance = length(viewVec);
    viewVec /= viewDistance;
    float dnv = -dot(normal, viewVec);

    float fresnel = 1.125 - 0.45 / (max(0, dnv) + 0.4);

    vec3 surface = max(vec3(0, 0, 0), texture(renderBuffer, texCoord).rgb);

    // surface += ambientColor * (albedo * fresnel + (1 - fresnel) * specular);

    // vec3 bg = (view * vec4(0, 0, 1, 0)).xyz;
    surface = mix(surface, fog, clamp(pow(position.w / fog_far, 0.5), 0.8, 1));

    surface = filmicToneMapping(surface);

    fragColor = vec4(surface, 1.0);
    // fragColor = vec4(albedo, 1.0);
    vec4 nm = vec4(normal, 0);
    nm = invView * nm;
    nm = nm * vec4(1, 1, -1, 1);
    nm = nm * 0.5 + 0.5;

    // fragColor = vec4(nm.xyz, 1.0);
    fragColor = vec4(nm.xyz, 1.0);
    // fragColor = vec4(roughness, roughness, roughness, 1.0);
    // fragColor = vec4(specular * (1 + 1 / roughness), 1.0);
}
