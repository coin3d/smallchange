#ifndef COIN_SOSOUNDSOURCE_H
#define COIN_SOSOUNDSOURCE_H

#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSfVec3f.h>
#include <Inventor/fields/SoSfFloat.h>
#include <Inventor/fields/SoSfNode.h>

class SoSoundSource : 
  public SoNode
{
  SO_NODE_HEADER(SoSoundSource);

public:

  static void initClass();
  SoSoundSource();

  SoSFVec3f velocity;
  SoSFFloat gain;
  SoSFNode  source;



  // See OpenAL spec, chap. 4.3.
  // min_gain
  // max_gain
  // reference_distance
  // rolloff_factor
  // max_distance
  // pitch
  // direction
  // cone_inner_angle
  // cone_outer_angle
  // cone_outer_gain

protected:

  virtual void GLRender(SoGLRenderAction *action);

  class SoFieldSensor * sourcesensor;
  static void sourceSensorCB(void *, SoSensor *);

private:

  virtual ~SoSoundSource();

};

#endif COIN_SOSOUNDSOURCE_H