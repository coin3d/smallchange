#ifndef SM_ENVELOPE_H
#define SM_ENVELOPE_H

class SmEnvelopeP;
class SoNode;

#include <Inventor/SbBasic.h>

class SmEnvelope {
public:

  SmEnvelope(void);
  ~SmEnvelope();

  SbBool importFile(const char * infile);
  // void importScene(SoNode * node);
  
  SbBool exportGeometry(const char * outfile, 
                        const int octtreelevels = 0,
                        const SbBool vrml2 = FALSE);

private:
  SmEnvelopeP * pimpl;

};

#endif // SM_ENVELOPE_H
