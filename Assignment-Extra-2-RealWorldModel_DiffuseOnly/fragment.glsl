varying vec2 varyingTexCoord;
uniform sampler2D diffuseTexture;
void main() {
gl_FragColor = texture2D(diffuseTexture, varyingTexCoord);
}