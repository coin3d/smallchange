#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdlib.h>

#include <Inventor/C/glue/dl.h>
#include <Inventor/C/tidbits.h>

#include <SmallChange/misc/SceneryGlue.h>

/* ********************************************************************** */

typedef int ss_initialize_f(void);
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
typedef int ss_system_get_blocksize_f(ss_system * system);
typedef int ss_system_get_dataset_type_f(ss_system * system, int id);
typedef int ss_system_get_num_datasets_f(ss_system * system);
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
typedef int ss_system_get_elevation_f(ss_system * system, 
                                      int numdatasets, int * datasets,
                                      int numpoints, double * points,
                                      float * normals, uint32_t * rgba,
                                      int * datasetids,
                                      unsigned int flags);

/* ********************************************************************** */

typedef struct sc_scenery_api {
  ss_initialize_f * initialize;
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
  ss_system_get_blocksize_f * system_get_blocksize;
  ss_system_get_dataset_type_f * system_get_dataset_type;
  ss_system_get_num_datasets_f * system_get_num_datasets;
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
  ss->method = (ss_##method##_f *) cc_dl_sym(lib, "ss_" #method); \
  assert(ss->method)
    SC_SCENERY_API_REGISTER(initialize);
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
    SC_SCENERY_API_REGISTER(system_get_elevation);
#undef SC_SCENERY_REGISTER
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

int
sc_ssglue_initialize(void)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->initialize();
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
sc_ssglue_system_get_blocksize(ss_system * system)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_get_blocksize(system);
}

int
sc_ssglue_system_get_dataset_type(ss_system * system, int id)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_get_dataset_type(system, id);
}

int
sc_ssglue_system_get_num_datasets(ss_system * system)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_get_num_datasets(system);
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

void
sc_ssglue_view_set_render_sequence_a(ss_system * system, int viewid, int num, int * sequence)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  ss->view_set_render_sequence_a(system, viewid, num, sequence);
}

int
sc_ssglue_system_get_elevation(ss_system * system, 
                               int numdatasets, int * datasets,
                               int numpoints, double * points,
                               float * normals, uint32_t * rgba,
                               int * datasetids,
                               unsigned int flags)
{
  assert(sc_scenery_available());
  const sc_scenery_api * ss = sc_scenery();
  return ss->system_get_elevation(system, numdatasets, datasets,
                                  numpoints, points,
                                  normals, rgba,
                                  datasetids, flags);
}

/* EOF ****************************************************************** */
