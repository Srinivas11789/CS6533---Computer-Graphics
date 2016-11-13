attribute vec4 position;
attribute vec4 normal;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;
varying vec3 varyingNormal;
void main() {
varyingNormal = normalize((normalMatrix * normal).xyz);
gl_Position = projectionMatrix * modelViewMatrix * position;
}