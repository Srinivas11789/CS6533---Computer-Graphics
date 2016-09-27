varying vec4 varyingColor;
varying vec2 varyingTexCoord;
uniform sampler2D texture;
uniform vec2 time;
uniform vec2 varyColor;
void main() {
vec2 texCoord = vec2(varyingTexCoord.x + time.x, varyingTexCoord.y + time.y);
gl_FragColor = texture2D(texture, texCoord);
}
