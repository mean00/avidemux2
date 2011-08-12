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
    "gl_TexCoord[0] = a;\n"

    "vec4 shift=vec4(myWidth/2,myHeight/2,0,0);\n"


    "a=a-shift;\n"
    "mat3 rotation = mat3(\n"
    "   vec3( cos(skew),  sin(skew),  0.0),\n"
    "   vec3(-sin(skew),  cos(skew),  0.0),\n"
    "   vec3(       0.0,        0.0,  1.0)\n"
    ");\n"

    "vec4 xout=vec4(rotation * a.xyz, 1.0);\n"
    "xout=xout+shift;\n"

    "gl_Position = gl_ModelViewProjectionMatrix * xout;\n"
"\n"
"}   \n";

// EOF
