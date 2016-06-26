#extension GL_ARB_texture_rectangle: enable
uniform sampler2DRect myTextureY; // tex unit 0
uniform sampler2DRect myTextureU; // tex unit 1
uniform sampler2DRect myTextureV; // tex unit 2
uniform float pts; // tex unit 2
const vec2 half_vec=vec2(0.5,0.5);

#define T texture2DRect(myTextureY,.5+(p.xy*=.992))
void main() 
{
  vec3 p = gl_TexCoord[0].xy-0.5;
  vec3 o = T.rbb;
  for (float i=0.;i<100.;i++) 
    p.z += pow(max(0.,.5-length(T.rg)),2.)*exp(-i*.08);
  gl_FragColor=vec4(o*o+p.z,1);
}
