attribute vec4 position2;
attribute vec2 texCoord2;
varying vec2 texture2;
void main()
{
gl_Position = position2;
texture2 = texCoord2;
}