    #extension GL_ARB_texture_rectangle: enable
    uniform sampler2DRect myTextureY; // tex unit 0
    uniform sampler2DRect myTextureU; // tex unit 1
    uniform sampler2DRect myTextureV; // tex unit 2
    const vec2 half_vec=vec2(0.5,0.5);

    void main(void) {
      vec2 full_coord=gl_TexCoord[0].xy;
      vec2 half_coord=full_coord*half_vec;
      vec4 texvalV = texture2DRect(myTextureV, half_coord);
      vec4 texvalU = texture2DRect(myTextureU, half_coord);
      vec4 texvalY = texture2DRect(myTextureY, full_coord);
      gl_FragColor = vec4(texvalY.r, texvalU.r, texvalV.r, 1.0);
        }
