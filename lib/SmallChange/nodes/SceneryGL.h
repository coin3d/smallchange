#ifndef SS_SCENERYGL_H
#define SS_SCENERYGL_H

class SoState;
class SoAction;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ********************************************************************** */

typedef struct ss_render_pre_cb_info ss_render_pre_cb_info;
typedef struct RenderState RenderState;
typedef struct cc_hash cc_hash;

struct RenderState {
  // local block info
  float blocksize;
  double vspacing[2];
  double voffset[2];
  float tscale[2];
  float toffset[2];
  unsigned int texid;
  float * elevdata;
  signed char * normaldata;

  // global info
  double bbmin[3];
  double bbmax[3];
  int dotex;

  // elevation texture
  float etexscale;
  float etexoffset;

  // temporary
  unsigned int currtexid;
  unsigned char * texdata;
  int texw, texh, texnc;
  int texisenabled;

  cc_hash * texhash;
  void * reusetexlist; // SbList<TexInfo *>
  void * tmplist; // SbList<unsigned int>

  // debugging
  void * debuglist; // SbList<float>
  int newtexcount;

  // ugh
  SoState * state;
  SoAction * action;

};

/* ********************************************************************** */
/* GL setup */

void sc_set_use_byte_normals(int enable); // for buggy GL drivers

void sc_set_have_clamp_to_edge(int enable); // GL 1.x feature
void sc_set_glMultiTexCoord2f(void * fptr); // GL 1.3 feature

/* ********************************************************************** */
/* texture management */

void sc_renderstate_construct(RenderState * state);
void sc_renderstate_destruct(RenderState * state);

void sc_mark_unused_textures(RenderState * state);
void sc_delete_unused_textures(RenderState * state);
void sc_delete_all_textures(RenderState * state);

/* ********************************************************************** */
/* rendering callbacks */

int sc_render_pre_cb(void * closure, ss_render_pre_cb_info * info);

void sc_render_cb(void * closure, const int x, const int y,
                  const int len, const unsigned int bitmask);
void sc_undefrender_cb(void * closure, const int x, const int y, const int len,
                       const unsigned int bitmask_org);

/* ********************************************************************** */
/* misc utilitites */

void sc_generate_elevation_line_texture(float distance, float offset, float thickness, int emphasis, unsigned char * buffer, int components, int texturesize, float * texcoordscale, float * texcoordoffset);

void sc_display_debug_info(float * campos, short * vpsize, void * debuglist);

/* ********************************************************************** */

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /*! SS_SCENERYGL_H */
