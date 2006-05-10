
uniform sampler2D bumpmap;
uniform sampler2D envmap;

vec3 expand(vec3 v)
{
  return (v - 0.5) * 2.0;
}

void main(void)
{
  vec4 params = gl_SecondaryColor;
  vec4 color = gl_Color;
  vec2 bumpcoord = vec2(gl_TexCoord[0]);
  vec3 lightdir = vec3(gl_TexCoord[1]);
  vec3 halfangle = vec3(gl_TexCoord[2]);
  vec3 ray = vec3(gl_TexCoord[3]);

  vec3 n = expand(texture2D(bumpmap, bumpcoord).xyz);
  n.xy *= params[0];
  n.z = 0.8;
  n = normalize(n);
  vec3 l = normalize(lightdir);
  
  float d = clamp(dot(l, n), 0.0, 1.0);
  if (lightdir.z <= 0.0) d = 0.0;

  vec3 r = normalize(reflect(ray,n));
  float intensity = max(0.0, dot(r, -ray));  

  vec3 h = normalize(halfangle);
  float s = clamp(dot(h, n), 0.0, 1.0);
  if (d == 0.0) s = 0.0;
  vec3 speccol = vec3(pow(s, 128.0));
  vec3 diffuse1 = color.xyz * d;
  gl_FragColor = vec4(diffuse1 + speccol, color.a);
}

