#include "ALTools.h"


char *GetALErrorString(char *text, ALint errorcode)
{
  // text should be at least char[255]
	switch (errorcode)
	{
		case AL_INVALID_NAME:
			sprintf(text, "AL_INVALID_NAME - Illegal name passed as an argument to an AL call");
			break;
		case AL_INVALID_ENUM:
//		case AL_ILLEGAL_ENUM:
			sprintf(text, "AL_INVALID_ENUM - Illegal enum passed as an argument to an AL call");
			break;
		case AL_INVALID_VALUE:
			sprintf(text, "AL_INVALID_VALUE - Illegal value passed as an argument to an AL call");
			break;
		case AL_INVALID_OPERATION:
//		case AL_ILLEGAL_COMMAND:
			sprintf(text, "AL_INVALID_OPERATION - A function was called at an inappropriate time or in an inappropriate way, causing an illegal state. This can be an incompatible ALenum, object ID, and/or function");
			break;
		case AL_OUT_OF_MEMORY:
			sprintf(text, "AL_OUT_OF_MEMORY - A function could not be completed, because there is not enough memory available.");
			break;
		default:
			sprintf(text, "UNDEFINED ERROR");
			break;
	}
	return text;
}

