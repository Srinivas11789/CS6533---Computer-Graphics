uniform sampler2D screenFramebuffer2;
varying vec2 texture2;
void main()
{
gl_FragColor = vec4(1.0-texture2D( screenFramebuffer2, texture2).xyz, 1.0);
//gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}