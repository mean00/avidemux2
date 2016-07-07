//http://www.geeks3d.com/20091116/shader-library-2d-shockwave-post-processing-filter-glsl/
#extension GL_ARB_texture_rectangle: enable
uniform sampler2DRect myTextureY; // tex unit 0
uniform sampler2DRect myTextureU; // tex unit 1
uniform sampler2DRect myTextureV; // tex unit 2
uniform vec2  myResolution;
uniform float pts; // tex unit 2
const vec2 half_vec=vec2(0.5,0.5);
void main() 
{ 
  vec3 shockParams=vec3( 10.0, 0.8, myResolution.x/50); // 1- pow base, 2 - pow exponent 3- Size of the wave

  vec2 pos=gl_TexCoord[0].xy;
  vec2 texCoord = pos;

  float distance = distance(pos, myResolution*half_vec);

  float time=pts/3000.;

 if ( (distance <= (time + shockParams.z)) && 
       (distance >= (time - shockParams.z)) ) 
  {
    float diff = (distance - time); 
    float powDiff = 1.0 - pow(abs(diff*shockParams.x), 
                                shockParams.y); 
    float diffTime = diff  * powDiff; 
    vec2 diffUV = normalize(pos - myResolution*half_vec); 
    texCoord = pos + (diffUV * diffTime);
  } 

  vec3 cY = texture2DRect(myTextureY, texCoord);
  vec3 cU = texture2DRect(myTextureU, texCoord*half_vec);
  vec3 cV = texture2DRect(myTextureV, texCoord*half_vec);

  gl_FragColor = vec4(cY.r,cU.r,cV.r,1.0);
}
