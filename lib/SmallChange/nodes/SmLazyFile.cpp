
#include <SmallChange/nodes/SmLazyFile.h>
#include <Inventor/SoInput.h>
#include <Inventor/actions/SoAction.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

class SmLazyFileP {
public:
  SbBool loaded;
  SbBool isloading;
  SoInput * input;
};

SO_NODE_SOURCE(SmLazyFile);

void
SmLazyFile::initClass(void)
{
  SO_NODE_INIT_CLASS(SmLazyFile, SoFile, "File");
}

SmLazyFile::SmLazyFile(void)
{
  SO_NODE_CONSTRUCTOR(SmLazyFile);
  PRIVATE(this) = new SmLazyFileP;
  PRIVATE(this)->loaded = FALSE;
  PRIVATE(this)->isloading = FALSE;
  PRIVATE(this)->input = NULL;
}

SmLazyFile::~SmLazyFile()
{
  delete PRIVATE(this);
}

// Doc from superclass.
void
SmLazyFile::getBoundingBox(SoGetBoundingBoxAction * action)
{
  if (!PRIVATE(this)->loaded) {
    SbBox3f bbox(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    action->setCenter(SbVec3f(0.0f, 0.0f, 0.0f), FALSE);  
    action->extendBy(bbox);
  } else {
    inherited::getBoundingBox(action);
  }
}

void 
SmLazyFile::GLRender(SoGLRenderAction * action)
{
  if (!PRIVATE(this)->loaded && !PRIVATE(this)->isloading) {
    PRIVATE(this)->isloading = TRUE;
    SoInput in;
    PRIVATE(this)->loaded = inherited::readNamedFile(&in);
    PRIVATE(this)->isloading = FALSE;
  }
  SoFile::doAction((SoAction *)action);
}

// Doc from superclass.
SbBool
SmLazyFile::readInstance(SoInput * in, unsigned short flags)
{
  return inherited::readInstance(in, flags);
}

SbBool 
SmLazyFile::readNamedFile(SoInput * in)
{
  return TRUE;
}

#undef PRIVATE
#undef PUBLIC
