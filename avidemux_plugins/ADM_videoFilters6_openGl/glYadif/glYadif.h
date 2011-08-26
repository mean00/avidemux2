//
static const char *myShaderY =
	"#extension GL_ARB_texture_rectangle: enable\n"
    "#extension GL_EXT_gpu_shader4 : enable\n"
	"uniform sampler2DRect myTexturePrev;\n" // tex unit 0
    "uniform sampler2DRect myTextureCur;\n" // tex unit 1
    "uniform sampler2DRect myTextureNext;\n" // tex unit 2
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"
    "uniform int   myParity;\n"

	"void main(void) {\n"
    "  float nx = gl_TexCoord[0].x;\n"
	"  float ny = gl_TexCoord[0].y;\n"
    "  int lineno=ny;\n"
    "  int odd=lineno&1;\n"
    "  if(odd)\n"
    "  {  \n"
    "           gl_FragColor = texture2DRect(myTextureCur,vec2(nx,ny));\n"
    "  }  \n"
    "  else \n"
    " {\n"
    "           gl_FragColor = vec4(0,0,0,0);\n"
    " }\n"
	
	"}\n";

