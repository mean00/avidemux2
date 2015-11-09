// Invert x & y
static const char *myShaderY =
    "#extension GL_ARB_texture_rectangle: enable\n"
    "uniform sampler2DRect myTextureY;\n" // tex unit 0
    "uniform sampler2DRect myTextureU;\n" // tex unit 1
    "uniform sampler2DRect myTextureV;\n" // tex unit 2
    "uniform float myWidth;\n"
    "uniform float myHeight;\n"
    "uniform float teta;\n"
    "const vec2 half_vec=vec2(0.5,0.5);\n"

    "void main(void) {\n"
    "  vec2 sizexy=vec2(myWidth/2.,myHeight/2.);\n"
    "  vec2 full_coord=gl_TexCoord[0].xy-sizexy;\n"
    "  float c=cos(teta); \n"
    "  float s=sin(teta); \n"
    "  mat2 mymatrix=mat2(c,-s,s,c);"
    "  "
    "  vec2 new_coord=full_coord*mymatrix+sizexy;\n"
    "  vec2 half_coord=new_coord*half_vec;\n"
    "  vec4 texvalV = texture2DRect(myTextureV, half_coord);\n"
    "  vec4 texvalU = texture2DRect(myTextureU, half_coord);\n"
    "  vec4 texvalY = texture2DRect(myTextureY, new_coord);\n"
    "  gl_FragColor = vec4(texvalY.r, texvalU.r, texvalV.r, 1.0);\n"
    "}\n";

