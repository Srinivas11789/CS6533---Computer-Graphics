uniform sampler2D screenFramebuffer3;
varying vec2 texture3;
void main()
{
vec4 texColor = texture2D( screenFramebuffer3, texture3);
float brightness = (texColor.x+texColor.y+texColor.z)/3.0;
gl_FragColor = vec4(brightness, brightness, brightness, 1.0);
}