#version 450

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 vertexTexCoord;

uniform mat4 view;
uniform mat4 model;
uniform mat3 model_normal;

uniform bool has_texture;

out vec3 color;
out vec4 position;
out vec4 v_position;
out vec3 normal;
out vec2 texCoord;

void main()
{
	vec4 pos = vertexPosition;
	pos = model * pos;
	position = pos;
	pos = view * pos;
	texCoord = has_texture ? vertexTexCoord : vec2(0.);

	color = vertexColor;
	normal = normalize(model_normal * vertexNormal);

	// pos = pos / pos.w;

	// normal = vec3(max(0, dot(L, N)));
	gl_Position = pos;
	v_position = pos;
}
