#version 330

in vec3 vertexColorToFS;
in vec2 vertexTexcoordToFS;

uniform sampler2D myTexture;

out vec4 pixelColor;

void main() {
    pixelColor = texture(myTexture, vertexTexcoordToFS);
    if (pixelColor.r >= 0.9 && pixelColor.g >= 0.9 && pixelColor.b >= 0.9) discard;
}
