uniform sampler2D screenFramebuffer2;
varying vec2 texture2;
void main()
{
vec4 texColor = texture2D( screenFramebuffer2, texture2);
float brightness = (texColor.x+texColor.y+texColor.z)/3.0;
//gl_FragColor = vec4(1.0-(brightness * texture2D( screenFramebuffer2, texture2)).xyz, 1.0);
gl_FragColor = vec4(brightness*(1.0-texture2D( screenFramebuffer2, texture2)).xyz, 1.0);
//gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}