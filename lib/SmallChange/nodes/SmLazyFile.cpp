
#include <SmallChange/nodes/SmLazyFile.h>
#include <Inventor/SoInput.h>
#include <Inventor/actions/SoAction.h>
#include <Inventor/actions/SoGLRenderAction.h>

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

class SmLazyFileP {
public:
  SbBool loaded;
  SbString name;
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
}

SmLazyFile::~SmLazyFile()
{
  delete PRIVATE(this);
}

void 
SmLazyFile::GLRender(SoGLRenderAction * action)
{
  if (!PRIVATE(this)->loaded || strcmp(this->name.getValue().getString(), 
                                       PRIVATE(this)->name.getString()) != 0) {
    SoInput in;
    inherited::readNamedFile(&in);
    PRIVATE(this)->loaded = TRUE;
    PRIVATE(this)->name = this->name.getValue();
  }
  inherited::doAction((SoAction *)action);
}

SbBool 
SmLazyFile::readNamedFile(SoInput * in)
{
  PRIVATE(this)->name = this->name.getValue();
  return TRUE;
}

#undef PRIVATE
#undef PUBLIC
