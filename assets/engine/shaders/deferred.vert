#version 450

uniform mat4 canvas;

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec2 vertexCoord;

out vec2 texCoord;

void main() {
    gl_Position = canvas * vertexPosition;
    texCoord = vertexCoord;
}
