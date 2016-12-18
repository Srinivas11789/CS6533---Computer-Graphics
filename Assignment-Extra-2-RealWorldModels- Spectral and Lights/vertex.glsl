attribute vec4 position;
attribute vec2 texCoord;

attribute vec4 normal;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;

varying vec2 varyingTexCoord;
varying vec3 varyingNormal;
varying vec3 varyingPosition;

void main() {
varyingTexCoord = texCoord;
varyingNormal = normalize((normalMatrix * normal).xyz);
vec4 p = modelViewMatrix * position;
varyingPosition = p.xyz;
gl_Position = projectionMatrix * p;
}