#ifndef COIN_SOSOUNDSCAPE_H
#define COIN_SOSOUNDSCAPE_H


#include <Inventor/nodes/SoSubNode.h>
#include <Inventor/fields/SoSfVec3f.h>
#include <Inventor/fields/SoSfRotation.h>
#include <Inventor/fields/SoSfFloat.h>

#include "SoAudioRenderAction.h"

class SoSoundscape : public SoNode
{
  SO_NODE_HEADER(SoSoundscape);

  friend class SoAudioRenderAction;

public:

  static void initClass();
  SoSoundscape();

  SoSFVec3f position;
  SoSFRotation orientation;
  SoSFVec3f velocity;
  SoSFFloat gain;

protected:

  virtual void audioRender(SoAudioRenderAction *action);
//  virtual void GLRender(SoGLRenderAction *action);

private:

  virtual ~SoSoundscape();

};

#endif // COIN_SOSOUNDSCAPE_H
