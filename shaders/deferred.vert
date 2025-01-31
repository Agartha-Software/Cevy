#version 450

layout(location = 0) in vec4 vertexPosition;

out vec2 texCoord;

void main() {
    gl_Position = vertexPosition;
    texCoord = vertexPosition.xy * 0.5 + 0.5;
    // gl_Position = pos;
}
