// Invert x & y
static const char *myShaderY =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTextureY;\n" // tex unit 0
    "uniform sampler2DRect myTextureU;\n" // tex unit 1
    "uniform sampler2DRect myTextureV;\n" // tex unit 2
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"
    "uniform float teta;\n"

	"void main(void) {\n"
    "  float mmin=255,mmax=0,r;\n"
    "  float nx = gl_TexCoord[0].x;\n"
	"  float ny = gl_TexCoord[0].y;\n"
    "  vec4 texvalV = vec4(0,0,0,0);\n"
    "  vec4 texvalU = vec4(0,0,0,0);\n"
	"  vec4 texvalY = vec4(0,0,0,0);\n"
    "  int x=0,y=0;\n"
    "  for(y=-1;y<2;y++)\n"
    "   for(x=-1;x<2;x++)\n"
    "  { \n"
    "   r=texture2DRect(myTextureY, vec2(x+nx,y+ny));\n"
    "   texvalV += texture2DRect(myTextureV, vec2(x+nx/2,y+ny/2));\n"
    "   texvalU += texture2DRect(myTextureU, vec2(x+nx/2,y+ny/2));\n"
	"   texvalY += r;\n"
    "  }"
    "  float t=texvalY.r/9;\n"
    "  r=t;"
	"  gl_FragColor = vec4(r, texvalU.r/9, texvalV.r/9, 1.0);\n"
	"}\n";

