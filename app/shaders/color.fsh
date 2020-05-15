#version 330

in vec3 vertexColorToFS;
in vec2 vertexTexcoordToFS;

uniform sampler2D myTexture;

out vec4 pixelColor;

void main() {
    pixelColor = vec4(vertexColorToFS, 1) * texture(myTexture, vertexTexcoordToFS);
}
