#include "SoAudioRenderAction.h"

#include <Inventor/elements/SoModelMatrixElement.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoSwitchElement.h>
#include <Inventor/elements/SoUnitsElement.h>
#include <Inventor/nodes/SoCoordinate3.h>
#include <Inventor/nodes/SoCoordinate4.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodekits/SoBaseKit.h>

#include "SoListener.h"
#include "SoSound.h"

#define SO_ACTION_ADD_METHOD_INTERNAL(nodeclass, method) \
  do { \
    SO_ACTION_ADD_METHOD(nodeclass, method); \
  } while (0)



SO_ACTION_SOURCE(SoAudioRenderAction);

void SoAudioRenderAction::initClass()
{
  SO_ACTION_INIT_CLASS(SoAudioRenderAction, SoAction);

  SO_ENABLE(SoAudioRenderAction, SoModelMatrixElement);
  SO_ENABLE(SoAudioRenderAction, SoCoordinateElement);
  SO_ENABLE(SoAudioRenderAction, SoSwitchElement);
  SO_ENABLE(SoAudioRenderAction, SoUnitsElement);

/*
//  SO_ACTION_ADD_METHOD(SoNode, nullAction);
  SO_ACTION_ADD_METHOD(SoNode, callDoAction);

  SO_ACTION_ADD_METHOD(SoListener, callAudioRender);
  SO_ACTION_ADD_METHOD(SoSound,   callAudioRender);

  SO_ACTION_ADD_METHOD(SoCoordinate3,   callDoAction);
  SO_ACTION_ADD_METHOD(SoCoordinate4,   callDoAction);
  SO_ACTION_ADD_METHOD(SoGroup,   callDoAction);
  SO_ACTION_ADD_METHOD(SoTransformation,   callDoAction);

  SO_ACTION_ADD_METHOD(SoBaseKit, callDoAction);
*/
};

SoAudioRenderAction::SoAudioRenderAction()
{
  SO_ACTION_CONSTRUCTOR(SoAudioRenderAction);

  static int first = 1;
  if (first) {
    first = 0;
    SO_ACTION_ADD_METHOD_INTERNAL(SoNode, nullAction);
//    SO_ACTION_ADD_METHOD_INTERNAL(SoSeparator, callDoAction);

    SO_ACTION_ADD_METHOD_INTERNAL(SoListener, callAudioRender);
    SO_ACTION_ADD_METHOD_INTERNAL(SoSound,   callAudioRender);

    SO_ACTION_ADD_METHOD_INTERNAL(SoCoordinate3,   callDoAction);
    SO_ACTION_ADD_METHOD_INTERNAL(SoCoordinate4,   callDoAction);
    SO_ACTION_ADD_METHOD_INTERNAL(SoGroup,   callDoAction);
    SO_ACTION_ADD_METHOD_INTERNAL(SoTransformation,   callDoAction);

    SO_ACTION_ADD_METHOD_INTERNAL(SoBaseKit, callDoAction);
  }
};

SoAudioRenderAction::~SoAudioRenderAction()
{
};

void SoAudioRenderAction::beginTraversal(SoNode *node)
{
  traverse(node);
};

void SoAudioRenderAction::callDoAction(SoAction *action, SoNode *node)
{
  node->doAction(action);
};

void SoAudioRenderAction::callAudioRender(SoAction *action, SoNode *node)
{
  SoAudioRenderAction *audioRenderAction = (SoAudioRenderAction *) action;

//  if (typeid(*node) == typeid(SoListener))
  
  if (node->isOfType(SoListener::getClassTypeId()))
  {
    SoListener *listener;
    listener = (SoListener *)node;
    listener->audioRender(audioRenderAction);
  }
  else if (node->isOfType(SoSound::getClassTypeId()))
  {
    SoSound *sound;
    sound = (SoSound *)node;
    sound->audioRender(audioRenderAction);
  }

};
