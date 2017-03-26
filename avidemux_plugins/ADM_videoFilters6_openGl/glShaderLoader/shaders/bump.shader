#extension GL_ARB_texture_rectangle: enable
uniform sampler2DRect myTextureY; // tex unit 0
uniform sampler2DRect myTextureU; // tex unit 1
uniform sampler2DRect myTextureV; // tex unit 2
uniform float pts; // time in us
uniform vec2  myResolution; // 
const vec2 haalf_vec=vec2(0.5,0.5);
const float PI = 3.1415926535;
 
void main()
{
  float aperture = 178.0;
  float aperturehaalf = 0.5 * aperture * (PI / 180.0);
  float maxFactor = sin(aperturehaalf)*3.*cos((pts/3000000.)*PI);
  if(maxFactor<0.)
  {
               maxFactor=-1.*maxFactor; // fabs ? 
  }
  
  vec2 uv;
  vec2 xy;
  vec2 mul2=vec2(2.0,2.0);
  xy =  mul2*gl_TexCoord[0].xy;
  xy=xy/myResolution -vec2(1.,1.);
  float d = length(xy);
  if (d < (2.0-maxFactor))
  {
    d = length(xy * maxFactor);
    float z = sqrt(1.0 - d * d);
    float r = atan(d, z) / PI;
    float phi = atan(xy.y, xy.x);
    
    uv.x = r * cos(phi) + 0.5;
    uv.y = r * sin(phi) + 0.5;
    uv=uv*myResolution;
  }
  else
  {
    uv = gl_TexCoord[0].xy ;
  }
  vec4 cY = texture2DRect(myTextureY, uv);
  vec4 cU = texture2DRect(myTextureU, uv*haalf_vec);
  vec4 cV = texture2DRect(myTextureV, uv*haalf_vec);
  gl_FragColor = vec4(cY.r,cU.r,cV.r,1.0);
}

