attribute vec4 position3;
attribute vec2 texCoord3;
varying vec2 texture3;
void main()
{
gl_Position = position3;
texture3 = texCoord3;
}