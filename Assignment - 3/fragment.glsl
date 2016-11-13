varying vec3 varyingNormal;
uniform vec3 uColor;
uniform vec3 lightDirection;
void main() {
float diffuse = max(0.0, dot(varyingNormal, lightDirection));
vec3 intensity = uColor * diffuse;
gl_FragColor = vec4(intensity.xyz, 1.0);
}