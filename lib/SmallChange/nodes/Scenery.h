#ifndef COIN_SCENERY_H
#define COIN_SCENERY_H

#include <Inventor/SbBasic.h>
#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/nodes/SoShape.h>
#include <Inventor/fields/SoSFString.h>
#include <Inventor/fields/SoSFBool.h>
#include <Inventor/fields/SoMFInt32.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/C/base/hash.h>
#include <Inventor/lists/SbList.h>
#include <Inventor/fields/SoSFFloat.h>
// #include <sim/hsvl/hsvl.h>

#include <SmallChange/basic.h>

class SoFieldSensor;
class SoSensor;
class SoPrimitiveVertex;
class SoFaceDetail;
class SoGLImage;

typedef struct ss_system ss_system;
typedef struct ss_render_pre_cb_info ss_render_pre_cb_info;

class SMALLCHANGE_DLL_API Scenery : public SoShape {
  typedef SoShape inherited;

  SO_NODE_HEADER(Scenery);

public:
  static void initClass(void);
  Scenery(void);

  SoSFString filename;
  SoMFInt32 renderSequence;
  SoSFFloat blockRottger;
  SoSFFloat loadRottger;
  SoSFBool visualDebug;

  SoSFBool colorTexture;
  SoMFFloat colorMap; // r, g, b, a

  virtual void GLRender(SoGLRenderAction * action);
  virtual void callback(SoCallbackAction * action);
  virtual void rayPick(SoRayPickAction * action);

  void preFrame(void);
  int postFrame(void);

  void setBlockRottger(const float c);
  float getBlockRottger(void) const;
  void setLoadRottger(const float c);
  float getLoadRottger(void) const;
  void refreshTextures(const int id);

  typedef struct {
    float blocksize;
    double vspacing[2];
    double voffset[2];
    float tscale[2];
    float toffset[2];
    unsigned int texid;
    float * elevdata;
    signed char * normaldata;

    // temporary
    unsigned char * texdata;
    int texw, texh, texnc;
  } RenderState;

protected:
  virtual ~Scenery();
  virtual void generatePrimitives(SoAction * action);
  virtual void computeBBox(SoAction * action, SbBox3f & box, SbVec3f & center);

private:
  SoFieldSensor * filenamesensor;
  SoFieldSensor * blocksensor;
  SoFieldSensor * loadsensor;
  SoFieldSensor * colormapsensor;
  SoFieldSensor * colortexturesensor;

  static int render_pre_cb(void * closure, ss_render_pre_cb_info * info);
  static void filenamesensor_cb(void * data, SoSensor * sensor);
  static void blocksensor_cb(void * data, SoSensor * sensor);
  static void loadsensor_cb(void * data, SoSensor * sensor);
  static void colormapsensor_cb(void * data, SoSensor * sensor);
  static uint32_t colortexgen_cb(void * closure, double * pos, float elevation, double * spacing);
  void colormaptexchange(void);
  
  static void undefrender_cb(void * closure, const int x, const int y, const int len, 
                             const unsigned int bitmask_org); 
  static void render_cb(void * closure, const int x, const int y,
                        const int len, const unsigned int bitmask);

  static int gen_pre_cb(void * closure, ss_render_pre_cb_info * info);
  static void gen_cb(void * closure, const int x, const int y,
                     const int len, const unsigned int bitmask);
  static void undefgen_cb(void * closure, const int x, const int y, const int len, 
                          const unsigned int bitmask_org);
    
  void GEN_VERTEX(RenderState * state, const int x, const int y, const float elev);

  ss_system * system;
  int blocksize;

  // the rest of the data should really be stored in tls
  SoPrimitiveVertex * pvertex;
  SoFaceDetail * facedetail;
  SbVec3f currhotspot;

  SoAction * curraction;
  SoState * currstate;
  int viewid;

  RenderState renderstate;
  SoGLImage * dummyimage;

  SbBool dotex;
  SbBool texisenabled;
  unsigned int currtexid;
  int colormaptexid;

  SoGLImage * findReuseTexture(const unsigned int texid);
  SoGLImage * createTexture(const unsigned int texid);

  void deleteUnusedTextures(void);
  
  class TexInfo {
  public:
    TexInfo() {
      this->image = NULL;
    }
    unsigned int texid;
    SoGLImage * image;
    int unusedcount;
  };
  
  static void hash_clear(unsigned long key, void * val, void * closure);
  static void hash_inc_unused(unsigned long key, void * val, void * closure);
  static void hash_check_unused(unsigned long key, void * val, void * closure);
  static void hash_add_all(unsigned long key, void * val, void * closure);

//   static int raypick_pre_cb(void * closure, const double * bmin, const double * bmax);
//   static void raypick_post_cb(void * closure);
  
  SbList <TexInfo*> reusetexlist;
  cc_hash * texhash;
  SbList <unsigned int> tmplist;
  SbList <float> debuglist;
  int numnewtextures;
  
  double bboxmin[3];
  double bboxmax[3];
};


#endif // COIN_SCENERY_H

