
void calc_sincos(const in vec2 xy,
                 const in vec4 geowaveFreq,
                 const in vec4 geowavePhase,
                 const in vec2 geowave1dir,
                 const in vec2 geowave2dir,
                 const in vec2 geowave3dir,
                 const in vec2 geowave4dir,
                 out vec4 sinvec,
                 out vec4 cosvec)
{
  float tmp;

  // FIXME: precalc geowaveFreq * wavedirvector ??
  tmp = dot(xy, geowave1dir) * geowaveFreq[0] + geowavePhase[0];
  sinvec[0] = sin(tmp);
  cosvec[0] = cos(tmp);

  tmp = dot(xy, geowave2dir) * geowaveFreq[1] + geowavePhase[1];
  sinvec[1] = sin(tmp);
  cosvec[1] = cos(tmp);

  tmp = dot(xy, geowave3dir) * geowaveFreq[2] + geowavePhase[2];
  sinvec[2] = sin(tmp);
  cosvec[2] = cos(tmp);

  tmp = dot(xy, geowave4dir) * geowaveFreq[3] + geowavePhase[3];
  sinvec[3] = sin(tmp);
  cosvec[3] = cos(tmp);
} 


vec4 calc_position(const in vec2 xy,
                   const in vec4 geowaveAmp,     
                   const in vec2 geowave1dir,
                   const in vec2 geowave2dir,
                   const in vec2 geowave3dir,
                   const in vec2 geowave4dir,
                   const in vec4 geowaveQ,
                   const in vec4 sinvec,
                   const in vec4 cosvec)
{
  vec4 ampQ = geowaveQ * geowaveAmp;

  // FIXME: precalculate these vectors outside the program and pass them as parameters
  vec4 posvecx = vec4(ampQ[0] * geowave1dir[0],
                          ampQ[1] * geowave2dir[0],
                          ampQ[2] * geowave3dir[0],
                          ampQ[3] * geowave4dir[0]);
  vec4 posvecy = vec4(ampQ[0] * geowave1dir[1],
                          ampQ[1] * geowave2dir[1],
                          ampQ[2] * geowave3dir[1],
                          ampQ[3] * geowave4dir[1]);


  vec4 p;
  p[0] = xy[0] + dot(posvecx, cosvec);
  p[1] = xy[1] + dot(posvecy, cosvec);

  p[2] = dot(geowaveAmp, sinvec);
  p[3] = 1.0;
  return p;
}


vec3 calc_normal(const in vec4 geowaveAmp,     
                   const in vec4 geowaveFreq,     
                   const in vec2 geowave1dir,
                   const in vec2 geowave2dir,
                   const in vec2 geowave3dir,
                   const in vec2 geowave4dir,
                   const in vec4 Q,
                   const in vec4 sinvec,
                   const in vec4 cosvec)
{

  vec3 n;

  // FIXME: precaluculate these vectors outside the program
  vec4 freqamp = geowaveAmp * geowaveFreq;
  vec4 dirx = vec4(geowave1dir[0], geowave2dir[0], geowave3dir[0], geowave4dir[0]);
  vec4 diry = vec4(geowave1dir[1], geowave2dir[1], geowave3dir[1], geowave4dir[1]);

  vec4 freqampx = freqamp * dirx;
  vec4 freqampy = freqamp * diry;

  n[0] = - dot(freqampx, cosvec);
  n[1] = - dot(freqampy, cosvec);
  n[2] =  1.0 - (dot(Q, freqamp * sinvec));
  return normalize(n);
}

vec3 calc_tangent(const in vec4 geowaveAmp,     
                    const in vec4 geowaveFreq,     
                    const in vec2 geowave1dir,
                    const in vec2 geowave2dir,
                    const in vec2 geowave3dir,
                    const in vec2 geowave4dir,
                    const in vec4 Q,
                    const in vec4 sinvec,
                    const in vec4 cosvec)
{
  vec4 freqamp = geowaveAmp * geowaveFreq;
  vec4 dirx = vec4(geowave1dir[0], geowave2dir[0], geowave3dir[0], geowave4dir[0]);
  vec4 diry = vec4(geowave1dir[1], geowave2dir[1], geowave3dir[1], geowave4dir[1]);

  vec3 t = vec3(-dot(Q*dirx*diry*freqamp, sinvec),
                    1.0 - dot(Q*diry*diry*freqamp, sinvec),
                    dot(diry * freqamp, cosvec));
  return normalize(t);
}

uniform vec3 eyepos;
uniform vec3 lightdir;
uniform vec3 attenuation; 
uniform vec4 geowaveAmp;
uniform vec4 geowaveFreq;
uniform vec4 geowavePhase;
uniform vec2 geowave1dir;
uniform vec2 geowave2dir;
uniform vec2 geowave3dir;
uniform vec2 geowave4dir;
uniform vec4 geowaveQ;

void main(void)
{
  vec4 position = gl_Vertex;
  float dist = distance(position.xyz, eyepos);
  float att = 1.0 / (attenuation[0] + attenuation[1] * dist + attenuation[2] * dist * dist);
  vec4 amp = geowaveAmp * att;

  mat4 modelViewProj = gl_ModelViewProjectionMatrix ;
  vec4 sinvec;
  vec4 cosvec;

  calc_sincos(position.xy,
              geowaveFreq,  
              geowavePhase,
              geowave1dir,
              geowave2dir,
              geowave3dir,
              geowave4dir,
              sinvec, cosvec);

  vec4 p = calc_position(position.xy,
                         amp,
                         geowave1dir,
                         geowave2dir,
                         geowave3dir,
                         geowave4dir,
                         geowaveQ,
                         sinvec,
                         cosvec);

  calc_sincos(p.xy,
              geowaveFreq,  
              geowavePhase,
              geowave1dir,
              geowave2dir,
              geowave3dir,
              geowave4dir,
              sinvec, cosvec);

  vec3 n = calc_normal(amp,
                       geowaveFreq,
                       geowave1dir,
                       geowave2dir,
                       geowave3dir,
                       geowave4dir,
                       geowaveQ,
                       sinvec, cosvec);
  
  vec3 t = calc_tangent(amp,
                        geowaveFreq,
                        geowave1dir,
                        geowave2dir,
                        geowave3dir,
                        geowave4dir,
                        geowaveQ,
                        sinvec, cosvec);

  vec3 binormal = normalize(cross(t, n));
  mat3 rot = mat3(binormal, t, n);
  
  // set bumpmap coordinates
  gl_TexCoord[0] = vec4(position.xy*0.04, 0.0, 1.0);  

  vec3 ldir = rot * lightdir;

  // direction to light source from vertex
  gl_TexCoord[1] = vec4(ldir, 1.0);
  vec3 eyedir = rot * normalize(eyepos - p.xyz);

  // half vector
  gl_TexCoord[2] = vec4(normalize(normalize(ldir) + normalize(eyedir)), 1.0);

  gl_Position = modelViewProj * p;    // position in clip space

  // light ray
  gl_TexCoord[3] = vec4(-eyedir, 1.0);

  // water color
  gl_FrontColor = vec4(0.2, 0.4, 0.7, 1);
  // gl_FrontColor = vec4(0.1, 0.2, 0.4, 1);

  // some parameters
  gl_FrontSecondaryColor = vec4(max(att, 0.05), 0.0, 0.0, 0.0);
}




