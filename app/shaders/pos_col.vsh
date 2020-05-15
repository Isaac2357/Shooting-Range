#version 330

in vec3 vertexPosition;
in vec3 vertexColor;
in vec2 vertexTexcoord;
out vec3 vertexColorToFS;
out vec2 vertexTexcoordToFS;

void main() {
    gl_Position = vec4(vertexPosition, 1);
    vertexColorToFS = vertexColor;
    vertexTexcoordToFS = vertexTexcoord;
}
