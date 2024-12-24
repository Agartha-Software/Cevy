#version 450

in vec3 color;
in vec4 position;
in vec4 v_position;
in vec3 normal;
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

// uniform Lights {
// 	vec4 position;
// 	vec3 color;
// 	float radius;
// } lights[lightCount];

// uniform Light lights[lightCount];

layout (std140, binding = 1) uniform LightBlock {
	Light lights[lightCount];
};


uniform int activeLights;

uniform vec3 albedo;
uniform vec3 specular_tint;
uniform float phong_exponent;
uniform bool halflambert;
uniform bool has_texture;

uniform sampler2D diffuseTexture;

out vec4 fragColor;


vec3 filmicToneMapping(vec3 color)
{
	color = max(vec3(0.), color - vec3(0.004));
	color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}

void shade_light(inout vec3 diffuse_light, inout vec3 specular, vec3 smooth_normal, float dnv, vec3 lightColor, vec4 lightPosition, float lightRadius, vec3 viewVec) {
	vec3 lightVec = lightPosition.xyz - position.xyz;
	float lightDist = length(lightVec);
	lightVec /= lightDist;

	vec3 light = lightColor / (lightDist * lightDist);

	float lambert = dot(smooth_normal, lightVec);

	// float halfLambert = lambert * 0.5 + 0.5;

	vec3 halfway = normalize(lightVec - viewVec);

	float phong;

	phong = max(0, lambert) * pow(max(0, dot(smooth_normal, halfway)), phong_exponent * 2) * phong_exponent / 2;
	// phong = max(0, lambert) * pow(max(0, dot(reflect(-lightVec, smooth_normal), -viewVec)), phong_exponent) * phong_exponent / 4;

	float hl = float(halflambert) * 0.5;
	lambert = lambert * (1 - hl) + hl;
	diffuse_light += light * max(0, lambert);
	specular += light * phong;
}

void main()
{
	vec3 smooth_normal = normal;
	smooth_normal = normalize(normal);

	vec3 cameraPos = invView[3].xyz;// / invView[3].w;
	vec3 albedo = has_texture ? texture(diffuseTexture, texCoord).xyz * albedo : albedo;
	vec3 viewVec = position.xyz - cameraPos;
	float viewDistance = length(viewVec);
	viewVec /= viewDistance;
	float dnv = -dot(smooth_normal, viewVec);

	float fresnel = 1.125 - 0.45 / (max(0, dnv) + 0.4);

	vec3 diffuse_light = ambientColor;

	vec3 specular = ambientColor;

	for (int i = 0; i < lightCount; ++i) {
		shade_light(diffuse_light, specular, smooth_normal, dnv, float(i < activeLights) * lights[i].color, lights[i].position, 0, viewVec);
	}

	vec3 surface = fresnel * diffuse_light * albedo * color + (1 - fresnel) * specular * specular_tint;

	surface = mix(surface, fog, clamp(pow(v_position.z / fog_far, 0.5), 0, 1));
	surface = filmicToneMapping(surface);

	fragColor = vec4(surface, 1.0);
}
