#ifndef SS_SCENERYGL_H
#define SS_SCENERYGL_H

class SoState;
class SoAction;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ********************************************************************** */

typedef struct ss_render_block_cb_info ss_render_block_cb_info;
typedef struct RenderState RenderState;

/*
 * The below RenderState structure is arbitrary - one we have chosen
 * to use, common for all the callback implementations in
 * SceneryGL.cpp.  There are no ties between this structure and the
 * Scenery library.
 */

struct RenderState {
  // local block info
  float blocksize;
  double vspacing[2];
  double voffset[2];
  float tscale[2];
  float toffset[2];
  float invtsizescale[2]; // (1.0f / blocksize) * tscale[n]
  unsigned int texid;
  float * elevdata;
  signed char * normaldata;

  // global info
  double bbmin[3];
  double bbmax[3];
  int dotex;
  int renderpass;

  // culling
  float * clipplanes;
  int numclipplanes;
  float raypos[3], raydir[3];

  int intersected;
  float intersection[3];

  // elevation texture
  float etexscale;
  float etexoffset;

  // internal vertex array state data
  int vertexcount;

  // post-block loop
  void * varray;
  void * narray;
  void * t1array;
  void * t2array;
  void * idxarray;
  void * lenarray;

  // interleaved
  float vertices[10*3];
  unsigned char normals[10*3];
  float texture1[10*2];
  float texture2[10*2];

  // temporary
  unsigned int currtexid;
  unsigned char * texdata;
  int texw, texh, texnc;
  int texisenabled;

  void * texhash;      // SbHash<TexInfo *, unsigned int>
  void * reusetexlist; // SbList<TexInfo *>
  void * tmplist;      // SbList<unsigned int>
  
  // debugging
  void * debuglist;    // SbList<float>
  int newtexcount;

};

/* ********************************************************************** */
/* GL setup */

void sc_set_use_bytenormals(int enable); // for buggy GL drivers (3Dlabs)
void sc_set_have_clamp_to_edge(int enable); // GL 1.x feature

void sc_probe_gl(int verbose); // automatic setup of the below features

/* don't use the following methods unless completely necessary */

void sc_set_glPolygonOffset(void * fptr);

/* texture objects */
void sc_set_glGenTextures(void * fptr);
void sc_set_glBindTexture(void * fptr);
void sc_set_glTexImage2D(void * fptr);
void sc_set_glDeleteTextures(void * fptr);

/* multi-texturing */
void sc_set_glMultiTexCoord2f(void * fptr);      /* GL 1.3 feature */
void sc_set_glClientActiveTexture(void * fptr);  /* GL 1.3 feature? */

/* vertex arrays */
void sc_set_glEnableClientState(void * fptr);    /* GL 1.1 feature */
void sc_set_glDisableClientState(void * fptr);   /* GL 1.1 feature */
void sc_set_glVertexPointer(void * fptr);        /* GL 1.1 feature */
void sc_set_glNormalPointer(void * fptr);        /* GL 1.1 feature */
void sc_set_glTexCoordPointer(void * fptr);      /* GL 1.1 feature */
void sc_set_glDrawArrays(void * fptr);           /* GL 1.1 feature */
void sc_set_glDrawElements(void * fptr);         /* GL 1.1 feature */
void sc_set_glDrawRangeElements(void * fptr);    /* GL 1.2 function */

/* ask about features */
int sc_found_multitexturing(void);
int sc_found_vertexarrays(void);
int sc_suggest_vertexarrays(void);
int sc_suggest_bytenormals(void);

/* ********************************************************************** */

void sc_renderstate_construct(RenderState * state);
void sc_renderstate_destruct(RenderState * state);

/* ********************************************************************** */
/* texture management */

typedef void * sc_texture_construct_f(unsigned char * data, int texw, int texh, int nc, int wraps, int wrapt, float q, int hey);
typedef void sc_texture_activate_f(RenderState * state, void * handle);
typedef void sc_texture_release_f(void * handle);

void sc_set_texture_functions(sc_texture_construct_f * construct, sc_texture_activate_f * activate, sc_texture_release_f * release);

void * sc_default_texture_construct(unsigned char * data, int texw, int texh, int nc, int wraps, int wrapt, float q, int hey);
void sc_default_texture_activate(RenderState * state, void * handle);
void sc_default_texture_release(void * handle);

void sc_mark_unused_textures(RenderState * state);
void sc_delete_unused_textures(RenderState * state);
void sc_delete_all_textures(RenderState * state);

/* ********************************************************************** */
/* rendering callbacks */

/* direct rendering with triangle fans */
void sc_render_pre_cb(void * closure, ss_render_block_cb_info * info);
void sc_render_post_cb(void * closure, ss_render_block_cb_info * info);

void sc_render_cb(void * closure, const int x, const int y, const int len,
                  const unsigned int bitmask);
void sc_undefrender_cb(void * closure, const int x, const int y, const int len,
                       const unsigned int bitmask_org);

/* delayed rendering with vertex arrays */
void sc_va_render_pre_cb(void * closure, ss_render_block_cb_info * info);
void sc_va_render_post_cb(void * closure, ss_render_block_cb_info * info);

void sc_va_render_cb(void * closure, const int x, const int y, const int len,
                  const unsigned int bitmask);
void sc_va_undefrender_cb(void * closure, const int x, const int y, const int len,
                       const unsigned int bitmask_org);

/* ********************************************************************** */
/* raypick callbacks */

void sc_raypick_pre_cb(void * closure, ss_render_block_cb_info * info);

void sc_raypick_cb(void * closure, const int x, const int y, const int len,
                   const unsigned int bitmask);
void sc_undefraypick_cb(void * closure, const int x, const int y, const int len,
                        const unsigned int bitmask_org);

/* ********************************************************************** */
/* culling callbacks */

int sc_plane_culling_pre_cb(void * closure, const double * bmin, const double * bmax);
void sc_plane_culling_post_cb(void * closure);

int sc_ray_culling_pre_cb(void * closure, const double * bmin, const double * bmax);
void sc_ray_culling_post_cb(void * closure);

/* ********************************************************************** */
/* misc utilitites */

void sc_generate_elevation_line_texture(float distance, float offset, float thickness, int emphasis, unsigned char * buffer, int components, int texturesize, float * texcoordscale, float * texcoordoffset);

void sc_init_debug_info(RenderState * state);
void sc_display_debug_info(RenderState * state, float * campos, short * vpsize);

/* ********************************************************************** */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /*! SS_SCENERYGL_H */
