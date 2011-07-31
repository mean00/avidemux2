// Invert x & y
static const char *myShaderY =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTextureY;\n"
    "uniform sampler2DRect myTextureU;\n"
    "uniform sampler2DRect myTextureV;\n"
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"

	"void main(void) {\n"
    "  vec4 texvalV = texture2DRect(myTextureV, vec2(gl_TexCoord[2]));\n"
    "  vec4 texvalU = texture2DRect(myTextureU, vec2(gl_TexCoord[1]));\n"
	"  vec4 texvalY = texture2DRect(myTextureY, vec2(gl_TexCoord[0]));\n"
	"  gl_FragColor = vec4(texvalY.r, texvalU.r, texvalV.r, 1.0);\n"
	"}\n";

