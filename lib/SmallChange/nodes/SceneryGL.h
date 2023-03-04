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
  /* local block info */
  float blocksize;
  double vspacing[2];
  double voffset[2];
  float * elevdata;
  signed char * normaldata;

  /* global info */
  double bbmin[3];
  double bbmax[3];
  int dotex;
  int renderpass;

  /* culling */
  float * clipplanes;
  int numclipplanes;
  float raypos[3], raydir[3];

  /* elevation texture */
  float etexscale;
  float etexoffset;

  /* temporary */
  unsigned int activescenerytexid; /* FIXME: it is ridiculous design
                                      to keep this public. AFAICS,
                                      this is done only for the
                                      purpose of resetting its value
                                      between renderings. 20040602
                                      mortene. */

  struct RenderStateP * pimpl;
};

/* ********************************************************************** */
/* GL setup */

#ifndef APIENTRY
/* for non-win32 builds */
#define APIENTRY
#endif /* !APIENTRY */

typedef void (APIENTRY * sc_msghandler_fp)(const char * msg);
void sc_probe_gl(const unsigned int ctxid, sc_msghandler_fp msghandler); /* automatic setup of the below features */

/* used to set the current OpenGL context id from client code,
   necessary for e.g. making sure texture allocation and destruction
   is done in the correct context  */
void sc_set_current_context_id(RenderState * state, unsigned int context);
void sc_unset_current_context(RenderState * state);

/* ********************************************************************** */

/* don't use the following methods unless completely necessary */

void sc_set_use_bytenormals(unsigned int ctxid, int enable); /* for buggy GL drivers (3Dlabs) */
int sc_get_use_bytenormals(unsigned int ctxid); /* for buggy GL drivers (3Dlabs) */
void sc_set_have_clamp_to_edge(unsigned int ctxid, int enable); /* GL 1.x feature */
int sc_get_have_clamp_to_edge(unsigned int ctxid); /* GL 1.x feature */
void sc_set_use_occlusion_test(unsigned int ctxid, int enable);
int sc_get_use_occlusion_test(unsigned int ctxid);

/* ask about features */
int sc_found_multitexturing(unsigned int ctxid);
int sc_found_vertexarrays(unsigned int ctxid);
int sc_suggest_vertexarrays(unsigned int ctxid);
int sc_suggest_bytenormals(unsigned int ctxid);

/* ********************************************************************** */

void sc_renderstate_construct(RenderState * state);
void sc_renderstate_destruct(RenderState * state);

/* ********************************************************************** */
/* texture management */

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

#if 0 /* FIXME: These used to be public, but it doesn't seem like they
         have to be? I've marked them as "static" inside
         SceneryGL.cpp. 20040602 mortene. */
void sc_raypick_pre_cb(void * closure, ss_render_block_cb_info * info);
void sc_raypick_post_cb(void * closure, ss_render_block_cb_info * info);

void sc_raypick_cb(void * closure, const int x, const int y, const int len,
                   const unsigned int bitmask);
void sc_undefraypick_cb(void * closure, const int x, const int y, const int len,
                        const unsigned int bitmask_org);
#endif

/* ********************************************************************** */
/* culling callbacks */

int sc_plane_culling_pre_cb(void * closure, const double * bmin, const double * bmax);
void sc_plane_culling_post_cb(void * closure);

#if 0 /* FIXME: These used to be public, but it doesn't seem like they
         have to be? I've marked them as "static" inside
         SceneryGL.cpp. 20040602 mortene. */
int sc_ray_culling_pre_cb(void * closure, const double * bmin, const double * bmax);
void sc_ray_culling_post_cb(void * closure);
#endif

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
