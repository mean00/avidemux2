// Invert x & y
static const char *myShaderY =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTextureY;\n" // tex unit 0
    "uniform sampler2DRect myTextureU;\n" // tex unit 1
    "uniform sampler2DRect myTextureV;\n" // tex unit 2
    "const vec2 half_vec=vec2(0.5,0.5);\n"
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"
    "uniform float teta;\n"

	"void main(void) {\n"
    "  vec2 full_coord=gl_TexCoord[0];\n"
    "  vec2 half_coord=full_coord*half_vec;"
    "  vec4 texvalV = texture2DRect(myTextureV, half_coord);\n"
    "  vec4 texvalU = texture2DRect(myTextureU, half_coord);\n"
	"  vec4 texvalY = texture2DRect(myTextureY, full_coord);\n"
	"  gl_FragColor = vec4(texvalY.r, texvalU.r, texvalV.r, 1.0);\n"
	"}\n";

