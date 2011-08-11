//void main()
//	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

static const char *myVertex=
//"#version 130\n"
"uniform float myWidth;\n"
"uniform float myHeight;\n"
"uniform float skew;\n"
"void main(void)\n"
"{\n"
    "vec4 a = gl_Vertex;\n"
    "vec4 pos=a;\n" //"vec4(a.x,a.y,0,0);"
    "gl_TexCoord[0] = pos;\n"
    "float nx=(a.x-myWidth/2);\n"
    "float ny=(a.y-myHeight/2);\n"
    "nx=nx*skew;\n"
    "ny=ny*skew;\n"
    "a.x=nx+myWidth/2;\n"
    "a.y=ny+myHeight/2;\n"
    "gl_Position = gl_ModelViewProjectionMatrix * a;\n"
"\n"
"}   \n";

// EOF

