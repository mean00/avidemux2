// Invert x & y
static const char *myShaderY =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTexture;\n"
    "uniform float teta;\n"
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"

	"void main(void) {\n"
    "  float angle=teta;\n" //"0.2;"
	"  float nx = gl_TexCoord[0].x;\n"
	"  float ny = gl_TexCoord[0].y;\n"
    "  float s= sin(angle);\n"
    "  float c= cos(angle);\n"
    "  nx=nx;\n"
    "  ny=ny;\n"
    "  float my= nx*s+ny*c+0.5;\n"
    "  float mx= nx*c-ny*s+0.5;\n"
	"  float t =  texture2DRect(myTexture, vec2(mx, my)).r;\n"
	"  gl_FragColor = vec4(t, t, t, 1.0);\n"
	//"  gl_FragColor = vec4(t, 1.0-t, 2*t, 1.0);\n"
	"}\n";

