//http://glslsandbox.com/e#33682.0
#extension GL_ARB_texture_rectangle: enable
uniform sampler2DRect myTextureY; // tex unit 0
uniform sampler2DRect myTextureU; // tex unit 1
uniform sampler2DRect myTextureV; // tex unit 2
uniform vec2  myResolution;
uniform float pts; // tex unit 2
const vec2 half_vec=vec2(0.5,0.5);
// Lightning
// By: Brandon Fogerty
// bfogerty at gmail dot com 
// xdpixel.com

float Hash( vec2 p)
{
     vec3 p2 = vec3(p.xy,1.0);
    return fract(sin(dot(p2,vec3(37.1,61.7, 12.4)))*3758.5453123);
}

float noise(in vec2 p)
{
    vec2 i = floor(p);
     vec2 f = fract(p);
     f *= f*(3.0-2.0*f);
    return mix(mix(Hash(i + vec2(0.,0.)), Hash(i + vec2(1.,0.)),f.x),
               mix(Hash(i + vec2(0.,1.)), Hash(i + vec2(1.,1.)),f.x),
               f.y);
}

float fbm(vec2 p)
{
     float v = 0.0;
     v += noise(p*1.0) * .75;
     v += noise(p*3.)  * .50;
     v += noise(p*9.)  * .250;
     v += noise(p*27.)  * .125;
     return v;
}


vec4 lightning( void ) 
{

	vec2 uv = ( gl_TexCoord[0].xy / myResolution.xy ) * 2.0 - 1.0;
	uv.x *= myResolution.x/myResolution.y; 
        uv.y -= 1.05;

	//vec2 tmp_uv;
	//tmp_uv.x = uv.y;
	//tmp_uv.y = uv.x;
	//uv = tmp_uv;
	float timeVal = pts /100000.;

	vec3 finalColor = vec3( 0.0 );
	for( int i=0; i < 10; ++i )
	{
		float indexAsFloat = float(i);
		float amp = 10.0 + (indexAsFloat*500.0);
		float period = 2.0 + (indexAsFloat+2.0);
		float thickness = mix( 0.9, 1.0, noise(uv*indexAsFloat) );
		float t = abs( 1.0 / (sin(uv.y + fbm( uv + timeVal * period )) * amp) * thickness );
		
		
		finalColor +=  t * vec3( 0.3, 0.3, .5 );
	}
	
	return vec4( finalColor, 1.0 );

}

void main(void)
{
        vec2 pos = gl_TexCoord[0].xy ;
	vec4 color = lightning();
        vec4 luma=color+texture2DRect(myTextureY,pos);
        vec4 chromaU=texture2DRect(myTextureU,pos*half_vec);
        vec4 chromaV=texture2DRect(myTextureV,pos*half_vec);
        gl_FragColor = vec4(luma.r,chromaU.r,chromaV.r,1.0);
}


