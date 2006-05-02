uniform sampler2D biasnoisemap0;
uniform sampler2D biasnoisemap1;

void main(void)
{
  vec4 scale = gl_Color;
  vec4 bias = gl_SecondaryColor;

  vec4 w0 = texture2D(biasnoisemap0, gl_TexCoord[0].st);
  vec4 w1 = texture2D(biasnoisemap1, gl_TexCoord[1].st);
  
  vec4 c = w0 + w1;
  c *= scale;
  c += bias;

  gl_FragColor = vec4(c.rgb, 0.0);

  // enable the line below to disable the noise texture
  // gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
}

