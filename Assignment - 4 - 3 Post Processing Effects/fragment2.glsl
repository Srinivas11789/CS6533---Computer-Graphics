uniform sampler2D screenFramebuffer2;
varying vec2 texture2;
void main()
{
vec4 texColor = texture2D( screenFramebuffer2, texture2);
float brightness = (texColor.x+texColor.y+texColor.z)/3.0;
vec4 bw = vec4(brightness,brightness,brightness,1.0);
gl_FragColor = vec4(1.0-texture2D( screenFramebuffer2, texture2).xyz, 1.0) + bw;
}
