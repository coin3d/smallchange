uniform sampler2D biasnoisemap0;
uniform sampler2D biasnoisemap1;

void main(void)
{
  vec4 w0 = texture2D(biasnoisemap0, gl_TexCoord[0].st);
  vec4 w1 = texture2D(biasnoisemap1, gl_TexCoord[1].st);
  
  // FIXME: just temporary
  gl_FragColor = vec4(w0.rgb, 0.2);
}


