attribute vec4 position;
attribute vec2 texCoord;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

varying vec2 varyingTexCoord;

void main() {
varyingTexCoord = texCoord;
gl_Position = projectionMatrix * modelViewMatrix * position;
}