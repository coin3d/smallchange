#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdlib.h>

#include <Inventor/C/glue/dl.h>
#include <Inventor/C/tidbits.h>
#include <Inventor/errors/SoDebugError.h>

#include <SmallChange/misc/SceneryGlue.h>

/* ********************************************************************** */

typedef int ss_initialize_f(void);
typedef ss_system * ss_system_create_for_cross_and_line_f(int maxviews, double * minpos, double * spacing, int * minelements);
typedef ss_system * ss_system_open_f(const char *, int);
typedef void ss_system_close_f(ss_system *);
typedef ss_system * ss_system_construct_f(int maxviews, double * origo, double * spacing, int * elements, float * values, float undef);
typedef ss_system * ss_system_construct_rotated_f(int maxviews, int cols, int rows, double * xyzgrid, double undefz);
typedef ss_system * ss_system_construct_randomized_f(int maxviews, int points, double * xyzvals, double reach);
typedef void ss_render_get_elevation_measures_f(ss_render_block_cb_info * info, double * offset, double * vertex_spacing, int * dimensions, float ** elevation_array, signed char ** normal_array, int ** dataset_array);
typedef void ss_render_get_texture_image_f(ss_render_block_cb_info * info, unsigned int texture_id, unsigned char ** bytes, int * width, int * height, int * components);
typedef void ss_render_get_texture_measures_f(ss_render_block_cb_info * info, unsigned int * texture_id, float * offset, float * scaling);
typedef const signed char * ss_render_get_undef_array_f(unsigned int bitmask);
typedef int ss_system_add_runtime_texture2d_f(ss_system * system, int dataset, ss_rttexture2d_cb * callback, void * closure);
typedef int ss_system_add_runtime_texture0d_f(ss_system * system, uint32_t color);
typedef int ss_system_get_blocksize_f(ss_system * system);
typedef void ss_system_get_origo_world_position_f(ss_system * system, double * coordinates);
typedef void ss_system_get_object_box_f(ss_system * system, double * bbmin, double * bbmax);
typedef void ss_system_get_elevation_data_box_f(ss_system * system, int id, double * bbmin, double * bbmax);
typedef void ss_system_refresh_runtime_texture2d_f(ss_system * system, int id);
typedef int ss_view_allocate_f(ss_system * system);
typedef void ss_view_deallocate_f(ss_system * system, int viewid);
typedef void ss_view_enable_f(ss_system * system, int viewid);
typedef void ss_view_evaluate_f(ss_system * system, int viewid);
typedef void ss_view_pre_frame_f(ss_system * system, int viewid);
typedef int ss_view_post_frame_f(ss_system * system, int viewid);
typedef int ss_view_render_f(ss_system * system, int viewid);
typedef void ss_view_set_culling_post_callback_f(ss_system * system, int viewid, ss_cull_post_cb * postcb, void * postclosure);
typedef void ss_view_set_culling_pre_callback_f(ss_system * system, int viewid, ss_cull_pre_cb * precb, void * preclosure);
typedef void ss_view_set_evaluate_rottger_parameters_f(ss_system * system, int viewid, float C, float c);
typedef void ss_view_get_evaluate_rottger_parameters_f(ss_system * system, int viewid, float * C, float * c);
typedef void ss_view_set_hotspots_f(ss_system * system, int viewid, int numhotspots, double * hotspots);
typedef void ss_view_set_load_rottger_parameters_f(ss_system * system, int viewid, float C, float c);
typedef void ss_view_get_load_rottger_parameters_f(ss_system * system, int viewid, float * C, float * c);
typedef void ss_view_set_render_callback_f(ss_system * system, int viewid, ss_render_cb * cb, void * closure);
typedef void ss_view_set_render_pre_callback_f(ss_system * system, int viewid, ss_render_block_cb * precb, void * closure);
typedef void ss_view_set_render_post_callback_f(ss_system * system, int viewid, ss_render_block_cb * postcb, void * closure);
typedef void ss_view_set_undef_render_callback_f(ss_system * system, int viewid, ss_render_cb * cb, void * closure);
typedef void ss_view_set_render_sequence_a_f(ss_system * system, int viewid, int num, int * sequence);

typedef int ss_system_get_num_datasets_f(ss_system * system);
typedef int ss_system_get_datasetid_f(ss_system * system, int datasetindex);
typedef int ss_system_has_dataset_f(ss_system * system, int datasetid);
typedef int ss_system_get_dataset_type_f(ss_system * system, int datasetid);
typedef int ss_system_get_dataset_name_f(ss_system * system, int datasetid, int maxchars, char * name);
typedef int ss_system_add_dataset_f(ss_system * system, int type, const char * name, int flags);
typedef int ss_system_delete_dataset_f(ss_system * system, int datasetid);

typedef void ss_system_set_dataset_cross_and_line_data_f(ss_system * handle, int dataset, int lodlevel, int flags, int startcross, int startline, int numcross, int numline, float * crosslineelevation);
typedef float ss_system_get_undef_elevation_f(ss_system * system);
typedef void ss_system_get_spacing_for_lodlevel_f(ss_system * system, int lodlevel, double * spacing);

typedef void ss_system_change_dataset_proximity_f(ss_system * handle, int datasetid, int numdatasets, int * datasets, float epsilon, float newval);

typedef void ss_system_cull_dataset_above_f(ss_system * handle, int datasetid, int numdatasets, int * datasets, float distance);
typedef void ss_system_cull_dataset_below_f(ss_system * handle, int datasetid, int numdatasets, int * datasets, float distance);

typedef void ss_system_oversample_dataset_f(ss_system * handle, int datasetid);
typedef void ss_system_smooth_dataset_f(ss_system * handle, int datasetid);
typedef void ss_system_strip_verticals_f(ss_system * handle, int datasetid, float dropsize);
typedef void ss_system_strip_horizontals_f(ss_system * handle, int datasetid, float maxskew);

typedef int ss_system_get_elevation_f(ss_system * system,
                                      int numdatasets, int * datasets,
                                      int numpoints, double * points,
                                      float * normals, uint32_t * rgba,
                                      int * datasetids,
                                      unsigned int flags);


/* ********************************************************************** */

typedef struct sc_scenery_api {
  ss_initialize_f * initialize;
  ss_system_create_for_cross_and_line_f * system_create_for_cross_and_line;
  ss_system_open_f * system_open;
  ss_system_close_f * system_close;
  ss_system_construct_f * system_construct;
  ss_system_construct_rotated_f * system_construct_rotated;
  ss_system_construct_randomized_f * system_construct_randomized;
  ss_render_get_elevation_measures_f * render_get_elevation_measures;
  ss_render_get_texture_image_f * render_get_texture_image;
  ss_render_get_texture_measures_f * render_get_texture_measures;
  ss_render_get_undef_array_f * render_get_undef_array;
  ss_system_add_runtime_texture2d_f * system_add_runtime_texture2d;
  ss_system_add_runtime_texture0d_f * system_add_runtime_texture0d;
  ss_system_get_blocksize_f * system_get_blocksize;
  ss_system_get_origo_world_position_f * system_get_origo_world_position;
  ss_system_get_object_box_f * system_get_object_box;
  ss_system_get_elevation_data_box_f * system_get_elevation_data_box;
  ss_system_refresh_runtime_texture2d_f * system_refresh_runtime_texture2d;
  ss_view_allocate_f * view_allocate;
  ss_view_deallocate_f * view_deallocate;
  ss_view_enable_f * view_enable;
  ss_view_evaluate_f * view_evaluate;
  ss_view_pre_frame_f * view_pre_frame;
  ss_view_post_frame_f * view_post_frame;
  ss_view_render_f * view_render;
  ss_view_set_culling_post_callback_f * view_set_culling_post_callback;
  ss_view_set_culling_pre_callback_f * view_set_culling_pre_callback;
  ss_view_set_evaluate_rottger_parameters_f * view_set_evaluate_rottger_parameters;
  ss_view_get_evaluate_rottger_parameters_f * view_get_evaluate_rottger_parameters;
  ss_view_set_hotspots_f * view_set_hotspots;
  ss_view_set_load_rottger_parameters_f * view_set_load_rottger_parameters;
  ss_view_get_load_rottger_parameters_f * view_get_load_rottger_parameters;
  ss_view_set_render_callback_f * view_set_render_callback;
  ss_view_set_render_pre_callback_f * view_set_render_pre_callback;
  ss_view_set_render_post_callback_f * view_set_render_post_callback;
  ss_view_set_undef_render_callback_f * view_set_undef_render_callback;
  ss_view_set_render_sequence_a_f * view_set_render_sequence_a;
  ss_system_get_num_datasets_f * system_get_num_datasets;
  ss_system_get_datasetid_f * system_get_datasetid;
  ss_system_has_dataset_f * system_has_dataset;
  ss_system_get_dataset_type_f * system_get_dataset_type;
  ss_system_get_dataset_name_f * system_get_dataset_name;
  ss_system_add_dataset_f * system_add_dataset;
  ss_system_delete_dataset_f * system_delete_dataset;
  ss_system_set_dataset_cross_and_line_data_f * system_set_dataset_cross_and_line_data;
  ss_system_get_undef_elevation_f * system_get_undef_elevation;
  ss_system_get_spacing_for_lodlevel_f * system_get_spacing_for_lodlevel;
  ss_system_change_dataset_proximity_f * system_change_dataset_proximity;
  ss_system_cull_dataset_above_f * system_cull_dataset_above;
  ss_system_cull_dataset_below_f * system_cull_dataset_below;
  ss_system_oversample_dataset_f * system_oversample_dataset;
  ss_system_smooth_dataset_f * system_smooth_dataset;
  ss_system_strip_verticals_f * system_strip_verticals;
  ss_system_strip_horizontals_f * system_strip_horizontals;
  ss_system_get_elevation_f * system_get_elevation;
} sc_scenery_api;

/* ********************************************************************** */

static void sc_scenery_close(void);

static struct sc_scenery_api * sc_scenery_simpleton = NULL;
static cc_libhandle sc_scenery_handle;
static const char * SCENERYGLUE_DLL_NAME = "SCENERYGLUE_DLL_NAME";

static
const struct sc_scenery_api *
sc_scenery(void)
{
  static int initialized = FALSE;
  if ( !initialized ) {
    initialized = TRUE;

    const char * overridename = coin_getenv(SCENERYGLUE_DLL_NAME);
    if ( overridename ) {
      sc_scenery_handle = cc_dl_open( overridename );
      if ( !sc_scenery_handle ) {
        // If an override name has been set, and we couldn't open the
        // library on that, don't attempt to use the default names
        // below -- as that is likely to be confusing in certain
        // contexts.
        goto fin;
      }
    }

    if ( !sc_scenery_handle ) {
      sc_scenery_handle = cc_dl_open("scenery");
    }
    if ( !sc_scenery_handle ) {
      sc_scenery_handle = cc_dl_open("scenery.so");
    }
    if ( !sc_scenery_handle ) {
      sc_scenery_handle = cc_dl_open("libscenery");
    }
    if ( !sc_scenery_handle ) {
      sc_scenery_handle = cc_dl_open("libscenery.so");
    }
    if ( !sc_scenery_handle ) {
      goto fin;
    }
    sc_scenery_simpleton =
      (struct sc_scenery_api *) malloc(sizeof(struct sc_scenery_api));
    // FIXME: the above are two one-off memory leaks.
    // can possibly be fixed by installing the sc_scenery_close() function
    // as a coin_atexit() callback.

    sc_scenery_api * ss = sc_scenery_simpleton;
    cc_libhandle lib = sc_scenery_handle;
#define SC_SCENERY_API_REGISTER(method) \
  ss->method = (ss_##method##_f *) NULL; \
  ss->method = (ss_##method##_f *) cc_dl_sym(lib, "ss_" #method);
    SC_SCENERY_API_REGISTER(initialize);
    SC_SCENERY_API_REGISTER(system_create_for_cross_and_line);
    SC_SCENERY_API_REGISTER(system_open);
    SC_SCENERY_API_REGISTER(system_close);
    SC_SCENERY_API_REGISTER(system_construct);
    SC_SCENERY_API_REGISTER(system_construct_rotated);
    SC_SCENERY_API_REGISTER(system_construct_randomized);
    SC_SCENERY_API_REGISTER(render_get_elevation_measures);
    SC_SCENERY_API_REGISTER(render_get_texture_image);
    SC_SCENERY_API_REGISTER(render_get_texture_measures);
    SC_SCENERY_API_REGISTER(render_get_undef_array);
    SC_SCENERY_API_REGISTER(system_add_runtime_texture2d);
    SC_SCENERY_API_REGISTER(system_add_runtime_texture0d);
    SC_SCENERY_API_REGISTER(system_get_blocksize);
    SC_SCENERY_API_REGISTER(system_get_dataset_type);
    SC_SCENERY_API_REGISTER(system_get_num_datasets);
    SC_SCENERY_API_REGISTER(system_get_origo_world_position);
    SC_SCENERY_API_REGISTER(system_get_object_box);
    SC_SCENERY_API_REGISTER(system_get_elevation_data_box);
    SC_SCENERY_API_REGISTER(system_refresh_runtime_texture2d);
    SC_SCENERY_API_REGISTER(view_allocate);
    SC_SCENERY_API_REGISTER(view_deallocate);
    SC_SCENERY_API_REGISTER(view_enable);
    SC_SCENERY_API_REGISTER(view_evaluate);
    SC_SCENERY_API_REGISTER(view_pre_frame);
    SC_SCENERY_API_REGISTER(view_post_frame);
    SC_SCENERY_API_REGISTER(view_render);
    SC_SCENERY_API_REGISTER(view_set_culling_post_callback);
    SC_SCENERY_API_REGISTER(view_set_culling_pre_callback);
    SC_SCENERY_API_REGISTER(view_set_evaluate_rottger_parameters);
    SC_SCENERY_API_REGISTER(view_get_evaluate_rottger_parameters);
    SC_SCENERY_API_REGISTER(view_set_hotspots);
    SC_SCENERY_API_REGISTER(view_set_load_rottger_parameters);
    SC_SCENERY_API_REGISTER(view_get_load_rottger_parameters);
    SC_SCENERY_API_REGISTER(view_set_render_callback);
    SC_SCENERY_API_REGISTER(view_set_render_pre_callback);
    SC_SCENERY_API_REGISTER(view_set_render_post_callback);
    SC_SCENERY_API_REGISTER(view_set_undef_render_callback);
    SC_SCENERY_API_REGISTER(view_set_render_sequence_a);
    SC_SCENERY_API_REGISTER(system_get_num_datasets);
    SC_SCENERY_API_REGISTER(system_get_datasetid);
    SC_SCENERY_API_REGISTER(system_has_dataset);
    SC_SCENERY_API_REGISTER(system_get_dataset_type);
    SC_SCENERY_API_REGISTER(system_get_dataset_name);
    SC_SCENERY_API_REGISTER(system_add_dataset);
    SC_SCENERY_API_REGISTER(system_delete_dataset);
    SC_SCENERY_API_REGISTER(system_set_dataset_cross_and_line_data);
    SC_SCENERY_API_REGISTER(system_get_undef_elevation);
    SC_SCENERY_API_REGISTER(system_get_spacing_for_lodlevel);
    SC_SCENERY_API_REGISTER(system_change_dataset_proximity);
    SC_SCENERY_API_REGISTER(system_cull_dataset_above);
    SC_SCENERY_API_REGISTER(system_cull_dataset_below);
    SC_SCENERY_API_REGISTER(system_oversample_dataset);
    SC_SCENERY_API_REGISTER(system_smooth_dataset);
    SC_SCENERY_API_REGISTER(system_strip_verticals);
    SC_SCENERY_API_REGISTER(system_strip_horizontals);
    SC_SCENERY_API_REGISTER(system_get_elevation);
#undef SC_SCENERY_API_REGISTER
  }
fin:
  return sc_scenery_simpleton;
}

int
sc_scenery_available(void)
{
  return (sc_scenery() != NULL) ? TRUE : FALSE;
}

void
sc_scenery_close(void)
{
  if ( sc_scenery_simpleton ) {
    free(sc_scenery_simpleton);
    sc_scenery_simpleton = NULL;
  }
  if ( sc_scenery_handle ) {
    cc_dl_close(sc_scenery_handle);
    sc_scenery_handle = NULL;
  }
}

/* ********************************************************************** */

void
sc_stub(const char * function)
{
  SoDebugError::post(function, "SmScenery: function '%s' not found "
                     "in scenery library.", function);
}

int
sc_ssglue_initialize(void)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->initialize();
}

ss_system *
sc_ssglue_system_create_for_cross_and_line(int maxviews, double * minpos, double * spacing, int * minelements)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_create_for_cross_and_line(maxviews, minpos, spacing, minelements);
}

ss_system *
sc_ssglue_system_open(const char * filename, int maxviews)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_open(filename, maxviews);
}

void
sc_ssglue_system_close(ss_system * system)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->system_close(system);
}

ss_system *
sc_ssglue_system_construct(int maxviews, double * origo, double * spacing, int * elements, float * values, float undef)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_construct(maxviews, origo, spacing, elements, values, undef);
}

ss_system *
sc_ssglue_system_construct_rotated(int maxviews, int rows, int cols, double * xyzgrid, double undefz)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_construct_rotated(maxviews, rows, cols, xyzgrid, undefz);
}

ss_system *
sc_ssglue_system_construct_randomized(int maxviews, int points, double * xyzvals, float reach)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_construct_randomized(maxviews, points, xyzvals, reach);
}

void
sc_ssglue_render_get_elevation_measures(ss_render_block_cb_info * info, double * offset, double * vertex_spacing, int * dimensions, float ** elevation_array, signed char ** normal_array, int ** dataset_array)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->render_get_elevation_measures(info, offset, vertex_spacing, dimensions, elevation_array, normal_array, dataset_array);
}

void
sc_ssglue_render_get_texture_image(ss_render_block_cb_info * info, unsigned int texture_id, unsigned char ** bytes, int * width, int * height, int * components)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->render_get_texture_image(info, texture_id, bytes, width, height, components);
}

void
sc_ssglue_render_get_texture_measures(ss_render_block_cb_info * info, unsigned int * texture_id, float * offset, float * scaling)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->render_get_texture_measures(info, texture_id, offset, scaling);
}

const signed char *
sc_ssglue_render_get_undef_array(unsigned int bitmask)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->render_get_undef_array(bitmask);
}
 
int
sc_ssglue_system_add_runtime_texture2d(ss_system * system, int dataset, ss_rttexture2d_cb * callback, void * closure)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_add_runtime_texture2d(system, dataset, callback, closure);
}

int
sc_ssglue_system_add_runtime_texture0d(ss_system * system, uint32_t color)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_add_runtime_texture0d(system, color);
}

int
sc_ssglue_system_get_blocksize(ss_system * system)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_get_blocksize(system);
}

void
sc_ssglue_system_get_origo_world_position(ss_system * system, double * coordinates)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->system_get_origo_world_position(system, coordinates);
}

void
sc_ssglue_system_get_object_box(ss_system * system, double * bbmin, double * bbmax)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->system_get_object_box(system, bbmin, bbmax);
}

void
sc_ssglue_system_get_elevation_data_box(ss_system * system, int id, double * bbmin, double * bbmax)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->system_get_elevation_data_box(system, id, bbmin, bbmax);
}

void
sc_ssglue_system_refresh_runtime_texture2d(ss_system * system, int id)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->system_refresh_runtime_texture2d(system, id);
}

int
sc_ssglue_view_allocate(ss_system * system)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->view_allocate(system);
}

void
sc_ssglue_view_deallocate(ss_system * system, int viewid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_deallocate(system, viewid);
}

void
sc_ssglue_view_enable(ss_system * system, int viewid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_enable(system, viewid);
}

void
sc_ssglue_view_evaluate(ss_system * system, int viewid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_evaluate(system, viewid);
}

void
sc_ssglue_view_pre_frame(ss_system * system, int viewid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_pre_frame(system, viewid);
}

int
sc_ssglue_view_post_frame(ss_system * system, int viewid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->view_post_frame(system, viewid);
}

int
sc_ssglue_view_render(ss_system * system, int viewid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->view_render(system, viewid);
}

void
sc_ssglue_view_set_culling_post_callback(ss_system * system, int viewid, ss_cull_post_cb * postcb, void * postclosure)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_culling_post_callback(system, viewid, postcb, postclosure);
}

void
sc_ssglue_view_set_culling_pre_callback(ss_system * system, int viewid, ss_cull_pre_cb * precb, void * preclosure)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_culling_pre_callback(system, viewid, precb, preclosure);
}

void
sc_ssglue_view_set_evaluate_rottger_parameters(ss_system * system, int viewid, float C, float c)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_evaluate_rottger_parameters(system, viewid, C, c);
}

void
sc_ssglue_view_get_evaluate_rottger_parameters(ss_system * system, int viewid, float * C, float * c)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_get_evaluate_rottger_parameters(system, viewid, C, c);
}

void
sc_ssglue_view_set_hotspots(ss_system * system, int viewid, int numhotspots, double * hotspots)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_hotspots(system, viewid, numhotspots, hotspots);
}

void
sc_ssglue_view_set_load_rottger_parameters(ss_system * system, int viewid, float C, float c)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_load_rottger_parameters(system, viewid, C, c);
}

void
sc_ssglue_view_get_load_rottger_parameters(ss_system * system, int viewid, float * C, float * c)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_get_load_rottger_parameters(system, viewid, C, c);
}

void
sc_ssglue_view_set_render_callback(ss_system * system, int viewid, ss_render_cb * cb, void * closure)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_render_callback(system, viewid, cb, closure);
}

void
sc_ssglue_view_set_render_pre_callback(ss_system * system, int viewid, ss_render_block_cb * precb, void * closure)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_render_pre_callback(system, viewid, precb, closure);
}

void
sc_ssglue_view_set_render_post_callback(ss_system * system, int viewid, ss_render_block_cb * postcb, void * closure)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_render_post_callback(system, viewid, postcb, closure);
}

void
sc_ssglue_view_set_undef_render_callback(ss_system * system, int viewid, ss_render_cb * cb, void * closure)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_undef_render_callback(system, viewid, cb, closure);
}

// FIXME: propagate stub-function-use further upwards (for all functions)
void
sc_ssglue_view_set_render_sequence_a(ss_system * system, int viewid, int num, int * sequence)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->view_set_render_sequence_a ) { sc_stub("ss_view_set_render_sequence_a"); return; }
  ss->view_set_render_sequence_a(system, viewid, num, sequence);
}

int
sc_ssglue_system_get_num_datasets(ss_system * system)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_get_num_datasets ) { sc_stub("ss_system_get_num_datasets"); return -1; }
  return ss->system_get_num_datasets(system);
}

int
sc_ssglue_system_get_datasetid(ss_system * system, int datasetindex)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_get_datasetid ) { sc_stub("ss_system_get_datasetid"); return -1; }
  return ss->system_get_datasetid(system, datasetindex);
}

int
sc_ssglue_system_has_dataset(ss_system * system, int datasetid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_has_dataset ) { sc_stub("ss_system_has_dataset"); return -1; }
  return ss->system_has_dataset(system, datasetid);
}

int
sc_ssglue_system_get_dataset_type(ss_system * system, int datasetid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_get_dataset_type ) { sc_stub("ss_system_get_dataset_type"); return -1; }
  return ss->system_get_dataset_type(system, datasetid);
}

int
sc_ssglue_system_get_dataset_name(ss_system * system, int datasetid, int maxchars, char * name)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_get_dataset_name ) { sc_stub("ss_system_get_dataset_name"); return 0; }
  return ss->system_get_dataset_name(system, datasetid, maxchars, name);
}

int
sc_ssglue_system_add_dataset(ss_system * system, int type, const char * name, int flags)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_add_dataset ) { sc_stub("ss_system_add_dataset"); return -1; }
  return ss->system_add_dataset(system, type, name, flags);
}

int
sc_ssglue_system_delete_dataset(ss_system * system, int datasetid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_delete_dataset ) { sc_stub("ss_system_delete_dataset"); return -1; }
  return ss->system_delete_dataset(system, datasetid);
}

void
sc_ssglue_system_set_dataset_cross_and_line_data(ss_system * handle, int dataset, int lodlevel, int flags, int startcross, int startline, int numcross, int numline, float * crosslineelevation)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_set_dataset_cross_and_line_data ) {
    sc_stub("ss_system_set_dataset_cross_and_line_data");
    return;
  }
  ss->system_set_dataset_cross_and_line_data(handle, dataset, lodlevel, flags, startcross, startline, numcross, numline, crosslineelevation);
}

float
sc_ssglue_system_get_undef_elevation(ss_system * handle)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_get_undef_elevation ) { sc_stub("ss_system_get_undef_elevation"); return 0.0f; }
  return ss->system_get_undef_elevation(handle);
}

void
sc_ssglue_system_get_spacing_for_lodlevel(ss_system * handle, int lodlevel, double * spacing)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_get_spacing_for_lodlevel ) { sc_stub("ss_system_get_spacing_for_lodlevel"); return; }
  ss->system_get_spacing_for_lodlevel(handle, lodlevel, spacing);
}

void
sc_ssglue_system_change_dataset_proximity(ss_system * handle, int datasetid, int numdatasets, int * datasets, float epsilon, float newval)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_change_dataset_proximity ) { sc_stub("ss_system_change_dataset_proximity"); return; }
  ss->system_change_dataset_proximity(handle, datasetid, numdatasets, datasets, epsilon, newval);
}

void
sc_ssglue_system_cull_dataset_above(ss_system * handle, int datasetid, int numdatasets, int * datasets, float distance)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_cull_dataset_above ) { sc_stub("ss_system_cull_dataset_above"); return; }
  ss->system_cull_dataset_above(handle, datasetid, numdatasets, datasets, distance);
}

void
sc_ssglue_system_cull_dataset_below(ss_system * handle, int datasetid, int numdatasets, int * datasets, float distance)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_cull_dataset_below ) { sc_stub("ss_system_cull_dataset_below"); return; }
  ss->system_cull_dataset_below(handle, datasetid, numdatasets, datasets, distance);
}

void
sc_ssglue_system_oversample_dataset(ss_system * handle, int datasetid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_oversample_dataset ) { sc_stub("ss_system_oversample_dataset"); return; }
  ss->system_oversample_dataset(handle, datasetid);
}

void
sc_ssglue_system_smooth_dataset(ss_system * handle, int datasetid)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_smooth_dataset ) { sc_stub("ss_system_smooth_dataset"); return; }
  ss->system_smooth_dataset(handle, datasetid);
}

void
sc_ssglue_system_strip_verticals(ss_system * handle, int datasetid, float dropsize)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_strip_verticals ) { sc_stub("ss_system_strip_verticals"); return; }
  ss->system_strip_verticals(handle, datasetid, dropsize);
}

void
sc_ssglue_system_strip_horizontals(ss_system * handle, int datasetid, float maxskew)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_strip_horizontals ) { sc_stub("ss_system_strip_horizontals"); return; }
  ss->system_strip_horizontals(handle, datasetid, maxskew);
}

int
sc_ssglue_system_get_elevation(ss_system * system, int numdatasets, int * datasets, int numpoints, double * points, float * normals, uint32_t * rgba, int * datasetids, unsigned int flags)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  if ( !ss->system_get_elevation ) { sc_stub("ss_system_get_elevation"); return -1; }
  return ss->system_get_elevation(system, numdatasets, datasets, numpoints, points, normals, rgba, datasetids, flags);
}

/* EOF ****************************************************************** */
