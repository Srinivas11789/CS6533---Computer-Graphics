varying vec2 varyingTexCoord;
uniform vec2 modelPosition1;
uniform sampler2D texture;
uniform vec2 time;
void main() {
vec2 texCoord = vec2(varyingTexCoord.x + time.x, varyingTexCoord.y + time.y) + vec2(modelPosition1.x , modelPosition1.y);
gl_FragColor = texture2D(texture, texCoord);
}
