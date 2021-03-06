varying vec3 varyingNormal;
varying vec3 varyingPosition;
uniform vec3 uColor;

varying vec2 varyingTexCoord;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

struct Light {
vec3 lightPosition;
vec3 lightColor;
vec3 specularLightColor;
};
uniform Light lights[2];
float attenuate(float dist, float a, float b) {
return 1.0 / (1.0 + a*dist + b*dist*dist);
}
void main() {
vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
vec3 specularColor = vec3(0.0, 0.0, 0.0);
for(int i=0; i< 2; i++) {
vec3 lightDirection = -normalize(varyingPosition-lights[i].lightPosition);
float diffuse = max(0.0, dot(varyingNormal, lightDirection));
float attenuation = 1.0;//attenuate(distance(varyingPosition, lights[i].lightPosition) / 5.0, 2.7, 5.0);
diffuseColor += (lights[i].lightColor * diffuse) * attenuation;
vec3 v = normalize(-varyingPosition);
vec3 h = normalize(v + lightDirection);
float specular = pow(max(0.0, dot(h, varyingNormal)), 64.0);
specularColor += lights[i].specularLightColor * specular * attenuation;
}
vec3 intensity = (texture2D(diffuseTexture, varyingTexCoord).xyz * diffuseColor) + specularColor;
gl_FragColor = vec4(intensity.xyz, 1.0);
}