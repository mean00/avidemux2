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
    "  float mx = gl_TexCoord[0].x;\n"
	"  float my = gl_TexCoord[0].y;\n"
    "  vec4 texvalV = texture2DRect(myTextureV, vec2(mx/2,my/2));\n"
    "  vec4 texvalU = texture2DRect(myTextureU, vec2(mx/2,my/2));\n"
	"  vec4 texvalY = texture2DRect(myTextureY, vec2(mx,my));\n"
	"  gl_FragColor = vec4(texvalY.r, texvalU.r, texvalV.r, 1.0);\n"
	"}\n";

