uniform vec4 noisexform0;
uniform vec4 noisexform1;
uniform vec4 noisexform2;
uniform vec4 noisexform3;
uniform vec4 scaleBias;

void main(void)
{
  vec4 uvtmp;
  uvtmp.x = 0.0;
  uvtmp.y = 0.0;
  uvtmp.z = 0.0;
  uvtmp.w = 1.0;

  vec4 uvin = gl_MultiTexCoord0;
  
  uvtmp.x = dot(noisexform0, uvin);
  uvtmp.y = dot(noisexform1, uvin);

  gl_TexCoord[0] = uvtmp;

  uvtmp.x = dot(noisexform2, uvin);
  uvtmp.y = dot(noisexform3, uvin);

  gl_TexCoord[1] = uvtmp;

  gl_FrontColor = vec4(scaleBias[0], scaleBias[0], scaleBias[2], scaleBias[2]);
  gl_FrontSecondaryColor = vec4(scaleBias[1], scaleBias[1], scaleBias[2], scaleBias[2]);

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}


