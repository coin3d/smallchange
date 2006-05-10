uniform vec4 trans0;
uniform vec4 trans1;
uniform vec4 trans2;
uniform vec4 trans3;

void main(void)
{
  vec4 uvtmp;
  uvtmp.x = 0.0;
  uvtmp.y = 0.0;
  uvtmp.z = 0.0;
  uvtmp.w = 1.0;
  // vec4 uvtmp(vec3(0.0), 1.0);
  vec4 uvin = gl_MultiTexCoord0;

  uvtmp.x = dot(uvin, trans0);
  gl_TexCoord[0] = uvtmp;
  uvtmp.x = dot(uvin, trans1);
  gl_TexCoord[1] = uvtmp;
  uvtmp.x = dot(uvin, trans2);
  gl_TexCoord[2] = uvtmp;
  uvtmp.x = dot(uvin, trans3);
  gl_TexCoord[3] = uvtmp;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;


}
