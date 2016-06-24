    #extension GL_ARB_texture_rectangle: enable
    uniform sampler2DRect myTextureY; // tex unit 0
    uniform sampler2DRect myTextureU; // tex unit 1
    uniform sampler2DRect myTextureV; // tex unit 2
    uniform float myWidth;
    uniform float myHeight;
    uniform float teta;

    void main(void) {
      float mmin=255,mmax=0,r;
      float nx = (float)(gl_TexCoord[0].x);
      float ny = (float)(gl_TexCoord[0].y);
      vec4 texvalV = vec4(0,0,0,0);
      vec4 texvalU = vec4(0,0,0,0);
      vec4 texvalY = vec4(0,0,0,0);
      int x=0,y=0;
      for(y=-1;y<2;y++)
       for(x=-1;x<2;x++)
       { 
        r=texture2DRect(myTextureY, vec2(x+nx,y+ny));
        texvalV += texture2DRect(myTextureV, vec2(x+nx/2.,y+ny/2.));
        texvalU += texture2DRect(myTextureU, vec2(x+nx/2.,y+ny/2.));
        texvalY += r;
      }
      float t=texvalY.r/9.;
      r=t;
      gl_FragColor = vec4(r, texvalU.r/9.0, texvalV.r/9.0, 1.0);
    }

