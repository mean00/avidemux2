// Invert x & y
static const char *myShader =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTex;\n"

	"void main(void) {\n"
	"  float nx = gl_TexCoord[0].x;\n"
	"  float ny = gl_TexCoord[0].y;\n"
	"  float t =  texture2DRect(myTex, vec2(ny, nx)).r;\n"
       // no op
	"  gl_FragColor = vec4(t, t, t, 1.0);\n"
	//"  gl_FragColor = vec4(t, 1.0-t, 2*t, 1.0);\n"
	"}\n";

