// Invert x & y
static const char *myShaderY =
	"#extension GL_ARB_texture_rectangle: enable\n"
	"uniform sampler2DRect myTexture;\n"
    "uniform int kernelSize;\n"
    "uniform int normalization;\n"

	"void main(void) {\n"
	"  int nx = (int)gl_TexCoord[0].x;\n"
	"  int ny = (int)gl_TexCoord[0].y;\n"
	"  float t =  0;\n"
    "  float mul;\n"
    "  int x,y;\n"
    "  for(y=-kernelSize;y<=kernelSize;y++)\n"
    "  {\n"
    "   for(x=-kernelSize;x<=kernelSize;x++)\n"
    "   {\n"
    "       if(0==x && 0==y){ mul=4;}\n"
    "       else if(0==x) {mul=-1;}\n"
    "       else if(0==y) {mul=-1;}\n"
    "       else {mul=0;}\n"
    "       t +=  mul*texture2DRect(myTexture, vec2(nx+x, ny+y)).r;\n"
    "     }\n"
    "  }\n"
    "  t=t/normalization;\n"
    "  t=t+texture2DRect(myTexture, vec2(nx, ny)).r;\n"
    "  gl_FragColor = vec4(t, t, t, 1.0);\n"
	"}\n";

