#ifndef SMALLCHANGE_SCENERYGLUE_H
#define SMALLCHANGE_SCENERYGLUE_H

/**************************************************************************\
 *
 *  This file is part of the Coin 3D visualization library.
 *  Copyright (C) 1998-2003 by Systems in Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  ("GPL") version 2 as published by the Free Software Foundation.
 *  See the file LICENSE.GPL at the root directory of this source
 *  distribution for additional information about the GNU GPL.
 *
 *  For using Coin with software that can not be combined with the GNU
 *  GPL, and for taking advantage of the additional benefits of our
 *  support services, please contact Systems in Motion about acquiring
 *  a Coin Professional Edition License.
 *
 *  See <URL:http://www.coin3d.org> for  more information.
 *
 *  Systems in Motion, Teknobyen, Abels Gate 5, 7030 Trondheim, NORWAY.
 *  <URL:http://www.sim.no>.
 *
\**************************************************************************/

#ifndef SMALLCHANGE_INTERNAL
#error this is a private header file
#endif /* SMALLCHANGE_INTERNAL */

#include <Inventor/SbBasic.h> /* uint32_t */

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/

#define SS_RENDER_BIT_WEST  (0x0010)
#define SS_RENDER_BIT_SOUTH (0x0020)
#define SS_RENDER_BIT_EAST  (0x0040)
#define SS_RENDER_BIT_NORTH (0x0080)

#define SS_ELEVATION_TYPE        (1)
#define SS_TEXTURE_TYPE          (2)

/*************************************************************************/

typedef struct ss_system ss_system;
typedef struct ss_render_block_cb_info ss_render_block_cb_info;

typedef int ss_cull_pre_cb(void * closure,
                           const double * bbmin, const double * bbmax);
typedef void ss_cull_post_cb(void * closure);
typedef void ss_render_block_cb(void * closure, ss_render_block_cb_info * info);
typedef void ss_render_cb(void * closure, const int x, const int y, 
                          const int len, const unsigned int omitmask);
typedef uint32_t ss_rttexture2d_cb(void * closure, double * pos, float elevation, double * spacing);

/*************************************************************************/

int sc_scenery_available(void);

int sc_ssglue_initialize(void);
ss_system * sc_ssglue_system_open(const char * filename, int maxviews);
void sc_ssglue_system_close(ss_system * system);

ss_system * sc_ssglue_system_construct(int maxviews, double * origo,
                                       double * spacing, int * elements,
                                       float * values, float undef);
ss_system * sc_ssglue_system_construct_rotated(int maxviews, int rows, int cols,
                                               double * xyzgrid, double undef);
ss_system * sc_ssglue_system_construct_randomized(int maxviews, int points,
                                                  double * xyzvals, float reach);

void sc_ssglue_render_get_elevation_measures(ss_render_block_cb_info * info,
                                             double * offset,
                                             double * vertex_spacing,
                                             int * dimensions,
                                             float ** elevation_array,
                                             signed char ** normal_array,
                                             int ** dataset_array);

void sc_ssglue_render_get_texture_image(ss_render_block_cb_info * info,
                                        unsigned int texture_id,
                                        unsigned char ** bytes,
                                        int * width, int * height,
                                        int * components);

void sc_ssglue_render_get_texture_measures(ss_render_block_cb_info * info,
                                              unsigned int * texture_id,
                                              float * offset,
                                              float * scaling);
const signed char * sc_ssglue_render_get_undef_array(unsigned int bitmask);
 
int sc_ssglue_system_add_runtime_texture2d(ss_system * system,
                                           int dataset,
                                           ss_rttexture2d_cb * callback,
                                           void * closure);

int sc_ssglue_system_get_blocksize(ss_system * system);

int sc_ssglue_system_get_dataset_type(ss_system * system, int id);
int sc_ssglue_system_get_num_datasets(ss_system * system);
void sc_ssglue_system_get_origo_world_position(ss_system * system, double * coordinates);
void sc_ssglue_system_get_object_box(ss_system * system, double * bbmin, double * bbmax);
void sc_ssglue_system_get_elevation_data_box(ss_system * system, int id, double * bbmin, double * bbmax);
void sc_ssglue_system_refresh_runtime_texture2d(ss_system * system, int id);

int sc_ssglue_view_allocate(ss_system * system);
void sc_ssglue_view_deallocate(ss_system * system, int viewid);
void sc_ssglue_view_enable(ss_system * system, int viewid);
void sc_ssglue_view_evaluate(ss_system * system, int viewid);

void sc_ssglue_view_pre_frame(ss_system * system, int viewid);
int sc_ssglue_view_post_frame(ss_system * system, int viewid);

int sc_ssglue_view_render(ss_system * system, int viewid);
void sc_ssglue_view_set_culling_post_callback(ss_system * system, int viewid,
                                              ss_cull_post_cb * postcb, 
                                              void * postclosure);

void sc_ssglue_view_set_culling_pre_callback(ss_system * system, int viewid,
                                             ss_cull_pre_cb * precb, 
                                             void * preclosure);

void sc_ssglue_view_set_evaluate_rottger_parameters(ss_system * system,
                                                    int viewid,
                                                    float C, float c);
void sc_ssglue_view_get_evaluate_rottger_parameters(ss_system * system,
                                                    int viewid,
                                                    float * C, float * c);
void sc_ssglue_view_set_hotspots(ss_system * system, int viewid,
                                 int numhotspots, double * hotspots); 
void sc_ssglue_view_set_load_rottger_parameters(ss_system * system, int viewid,
                                                float C, float c);
void sc_ssglue_view_get_load_rottger_parameters(ss_system * system, int viewid,
                                                float * C, float * c);
void sc_ssglue_view_set_render_callback(ss_system * system, int viewid,
                                        ss_render_cb * cb, void * closure);

void sc_ssglue_view_set_render_pre_callback(ss_system * system, int viewid,
                                            ss_render_block_cb * precb,
                                            void * closure);

void sc_ssglue_view_set_render_post_callback(ss_system * system, int viewid,
					     ss_render_block_cb * postcb,
					     void * closure);

void sc_ssglue_view_set_undef_render_callback(ss_system * system, int viewid,
                                              ss_render_cb * cb,
                                              void * closure);
void sc_ssglue_view_set_render_sequence_a(ss_system * system, int viewid,
                                          int num, int * sequence);

/*************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* !SMALLCHANGE_SCENERYGLUE_H */
