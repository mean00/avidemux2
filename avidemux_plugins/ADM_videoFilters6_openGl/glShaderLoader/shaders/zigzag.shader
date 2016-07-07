uniform sampler2DRect myTextureY; // tex unit 0
uniform sampler2DRect myTextureU; // tex unit 1
uniform sampler2DRect myTextureV; // tex unit 2
uniform float pts; // tex unit 2
const vec2 half_vec=vec2(0.5,0.5);

void main(void) {
vec2 p=gl_TexCoord[0].xy;
p.x = p.x + sin(p.y*.5+p.x*6.+pts/100000.*2.)*3;
vec2 half=half_vec*p;
vec4 texY = texture2DRect(myTextureY,p);
vec4 texU = texture2DRect(myTextureU,half);
vec4 texV = texture2DRect(myTextureV,half);
gl_FragColor = vec4(texY.r, texU.r, texV.r, 1.0);
}

