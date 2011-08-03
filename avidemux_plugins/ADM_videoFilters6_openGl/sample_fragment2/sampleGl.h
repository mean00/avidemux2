// Invert x & y
static const char *myShaderY =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTextureY;\n" // tex unit 0
    "uniform sampler2DRect myTextureU;\n" // tex unit 1
    "uniform sampler2DRect myTextureV;\n" // tex unit 2
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"

	"void main(void) {\n"
    "  float nx = gl_TexCoord[0].x;\n"
	"  float ny = gl_TexCoord[0].y;\n"
    "  vec4 texvalV = texture2DRect(myTextureV, vec2(nx,ny));\n"
    "  vec4 texvalU = texture2DRect(myTextureU, vec2(nx,ny));\n"
	"  vec4 texvalY = texture2DRect(myTextureY, vec2(nx,ny));\n"
	"  gl_FragColor = vec4(texvalY.r, texvalU.g, texvalV.b, 1.0);\n"
	"}\n";

