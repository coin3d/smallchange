#ifndef SMALLCHANGE_SCENERYGL_H
#define SMALLCHANGE_SCENERYGL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct cc_glglue cc_glglue;

typedef struct {
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

  // elevation texture
  float etexscale;
  float etexoffset;

  // temporary
  unsigned char * texdata;
  int texw, texh, texnc;

  // debugging
  void * debuglist;
  int newtexcount;
} RenderState;

void sc_set_glglue_instance(const cc_glglue * glue);

int sc_is_texturing_enabled(void);
void sc_enable_texturing(void);
void sc_disable_texturing(void);

void sc_generate_elevation_line_texture(float distance, float offset, float thickness, int emphasis, uint8_t * buffer, float * texcoordscale, float * texcoordoffset);

void sc_display_debug_info(float * campos, short * vpsize, void * debuglist);

void sc_undefrender_cb(void * closure, const int x, const int y, const int len,
                       const unsigned int bitmask_org);
void sc_render_cb(void * closure, const int x, const int y,
                  const int len, const unsigned int bitmask);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /*! SMALLCHANGE_SCENERYGL_H */
