#version 450

layout(location = 0) in vec4 vertexPosition;
layout(location = 1) in vec2 vertexCoord;

out vec2 texCoord;

void main() {
    gl_Position = vertexPosition;
    // texCoord = vertexPosition.xy * 0.5 + 0.5;
    texCoord = vertexCoord;
    // gl_Position = pos;
}
