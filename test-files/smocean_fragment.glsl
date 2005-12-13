
vec3 expand(vec3 v)
{
  return (v - 0.5) * 2.0;
}

void main(void)
{
  vec4 params = gl_SecondaryColor;
  vec4 color = gl_Color;
  vec2 bumpcoord = gl_TexCoord[0].xy;
  vec3 lightdir = gl_TexCoord[1].xyz;
  vec3 halfangle = gl_TexCoord[2].xyz;
  vec3 ray = gl_TexCoord[3].xyz;

  vec3 n = vec3(0,0,1);
  // vec3 n = expand(tex2D(normalmap, bumpcoord).xyz);
  //  n.xy *= params[0];
  n = normalize(n);
  vec3 l = normalize(lightdir);
  
  float d = dot(l, n);

  vec3 h = normalize(halfangle);
  float s = clamp(dot(h, n), 0.0, 1.0);
  vec3 speccol = vec3(pow(s, 128.0));
  vec3 r = reflect(ray, n);
  // vec3 diffuse = texCUBE(diffusemap, r);
  //  vec3 diffuse = color.xyz;
  vec3 diffuse = color.xyz;
  gl_FragColor = vec4(diffuse.xyz * d + speccol, color.a);
}

