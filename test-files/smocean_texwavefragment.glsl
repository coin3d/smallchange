uniform sampler2D cosmap0;
uniform sampler2D cosmap1;
uniform sampler2D cosmap2;
uniform sampler2D cosmap3;
uniform vec4 wavecoef0;
uniform vec4 wavecoef1;
uniform vec4 wavecoef2;
uniform vec4 wavecoef3;
uniform vec4 rescale;

void main(void)
{
  vec4 w0 = texture2D(cosmap0, gl_TexCoord[0].st);
  vec4 w1 = texture2D(cosmap1, gl_TexCoord[1].st);
  vec4 w2 = texture2D(cosmap2, gl_TexCoord[2].st);
  vec4 w3 = texture2D(cosmap3, gl_TexCoord[3].st);

  vec4 tmp = 
    w0 * wavecoef0 + 
    w1 * wavecoef1 + 
    w2 * wavecoef2 + 
    w3 * wavecoef3;

  tmp *= rescale;
  tmp += rescale;
  
  gl_FragColor = tmp;
}


