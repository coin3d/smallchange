/*

-> ender opp med å lage VRML2 kompatibelt først

  ToDo:

  *** Denne listen er flyttet til ToDo.txt ***
  
V få ting til å virke med nye noder
V location støtte
V startTime, stopTime

  
V Pitch
V bruke ny searchforfile
- bakgrunnslyder (mono, stereo, musikk) - det ser ut som om stereo input er svaret. evt
  endre buffer i minnet fra mono til stereo.
V Jobbe med SoSound felter (intensity/gain, ...)
V render action for audio
- se på eax ting (soundscapes)
V SoAudioDevice + contexts. se main() i simple2
- Streaming (se movie texture og async buffer)
- hva hvis parametre i SoAudioClip endrer seg, f.eks buffer url, dette bør plukkes opp av SoSound


- Se på dempingsmodellen. Tilnærme OpenAL og VRML2.
*/
#include <math.h>
#include <stdlib.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

//#define NUM_BUFFERS 1	// Number of buffers to be Generated
//ALuint	g_Buffers[NUM_BUFFERS];		// Array of Buffer IDs

ALuint	sourcebuf[2];

//ALfloat source0Pos[]={ -2.0, 0.0, 2.0};	// Behind and to the left of the listener
ALfloat source0Pos[]={ 0.0, 0.0, 0.0};	// Behind and to the left of the listener
ALfloat source0Vel[]={ 0.0, 0.0, 0.0};

ALfloat source1Pos[]={ 2.0, 0.0,-2.0};	// Front and right of the listener
ALfloat source1Vel[]={ 0.0, 0.0, 0.0};


//ALfloat listenerPos[]={0.0, 0.0, 0.0};
ALfloat listenerVel[]={0.0, 0.0, 0.0};
//ALfloat	listenerOri[]={0.0, 0.0, -1.0, 0.0, 1.0, 0.0};	// Listener facing into the screen


#include <Inventor/Win/SoWin.h>
#include <Inventor/Win/viewers/SoWinExaminerViewer.h>

#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/SbRotation.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoCone.h>
#include <Inventor/nodes/SoCube.h>
#include <Inventor/nodes/SoCylinder.h>
#include <Inventor/nodes/SoShapeHints.h>
#include <Inventor/nodes/SoSphere.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoDirectionalLight.h>
#include <Inventor/nodes/SoPerspectiveCamera.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/Win/viewers/SoWinViewer.h>
#include <Inventor/sensors/SoTimerSensor.h>

#include <Inventor/fields/SoMFString.h>
#include <Inventor/sensors/SoFieldSensor.h>
#include <Inventor/errors/SoDebugError.h>

#include <Inventor/nodes/SoShape.h>
#include <Inventor/elements/SoCoordinateElement.h>
#include <Inventor/elements/SoModelMatrixElement.h>
#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "SoListener.h"
#include "SoSoundSource.h"
#include "SoSoundBuffer.h"
#include "ALTools.h"


SoSeparator * root;
SoPerspectiveCamera * camera;
SoTransform *xf;

// class SoSoundBuffer;

SO_NODE_SOURCE(SoSoundSource);

void SoSoundSource::initClass()
{
  SO_NODE_INIT_CLASS(SoSoundSource, SoNode, "Node");
};

SoSoundSource::SoSoundSource()
{
  SO_NODE_CONSTRUCTOR(SoSoundSource);


  SO_NODE_ADD_FIELD(velocity, (0.0f, 0.0f, 0.0f));
  SO_NODE_ADD_FIELD(gain, (0.0f));
  SO_NODE_ADD_FIELD(source, (NULL));

  // use field sensor for filename since we will load an image if
  // filename changes. This is a time-consuming task which should
  // not be done in notify().
  this->sourcesensor = new SoFieldSensor(sourceSensorCB, this);
  this->sourcesensor->setPriority(0);
  this->sourcesensor->attach(&this->source);

};

SoSoundSource::~SoSoundSource()
{
  delete this->sourcesensor;
};

void SoSoundSource::GLRender(SoGLRenderAction *action)
{
  ALint error;
  SbVec3f pos, worldpos;
  ALfloat alfloat3[3];

  pos = SbVec3f(0.0f, 0.0f, 0.0f);
  SoModelMatrixElement::get(action->getState()).multVecMatrix(pos, worldpos); 

//  float x, y, z;
//  worldpos.getValue(x, y, z);
//  printf("(%0.2f, %0.2f, %0.2f)\n", x, y, z);

  SbVec3f2ALfloat3(alfloat3, worldpos);

	// Position ...
	alSourcefv(sourcebuf[0], AL_POSITION, alfloat3);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSoundSource::GLRender",
                              "alSourcefv(,AL_POSITION,) failed. %s",
                              GetALErrorString(errstr, error));
		return;
	}

	// Velocity ...
  SbVec3f2ALfloat3(alfloat3, velocity.getValue());

	alSourcefv(sourcebuf[0], AL_VELOCITY, alfloat3);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSoundSource::GLRender",
                              "alSourcefv(,AL_VELOCITY,) failed. %s",
                              GetALErrorString(errstr, error));
		return;
	}
}

//
// called when source changes
//
void
SoSoundSource::sourceSensorCB(void * data, SoSensor *)
{
  ALint error;
  SoSoundSource * thisp = (SoSoundSource*) data;

  if (!thisp->source.getValue())
    return;

  SoSoundBuffer *soundbuffer = (SoSoundBuffer *)thisp->source.getValue();
  // FIXME: use RTTI instead, to see what kind of node it is (might be a movietexture node)
  // ... or perhaps OI has a convenience method for just that (SoNode::getNodeId ??)

  // FIXME: stop playing existing buffer(s), etc...

	alSourcei(sourcebuf[0],AL_BUFFER, soundbuffer->bufferId);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcei(,AL_BUFFER,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
}


SO_NODE_SOURCE(SoSoundBuffer);

//ALuint SoSoundBuffer = {};

void SoSoundBuffer::initClass()
{
  SO_NODE_INIT_CLASS(SoSoundBuffer, SoNode, "Node");
};

SoSoundBuffer::SoSoundBuffer()
{
  SO_NODE_CONSTRUCTOR(SoSoundBuffer);

  SO_NODE_ADD_FIELD(url, (""));

  this->size = 0;
  this->frequency = 0;
  this->bufferId = 0; // no buffer (NULL), see alIsBuffer(...)
  this->readstatus = 0; // ?

  // use field sensor for url since we will load an image if
  // url changes. This is a time-consuming task which should
  // not be done in notify().
  this->urlsensor = new SoFieldSensor(urlSensorCB, this);
  this->urlsensor->setPriority(0);
  this->urlsensor->attach(&this->url);

};

SoSoundBuffer::~SoSoundBuffer()
{
  if (alIsBuffer(bufferId))
	  alDeleteBuffers(1, &bufferId);

  delete this->urlsensor;
};


SbBool SoSoundBuffer::loadUrl(void)
{
  // similar to SoTexture2::loadFilename()

  if (this->url.getNum() <1)
    return FALSE; // no url specified
//  SbString text;
//  this->url.get1(0, text); // only the first url is used
//  if (!text.getLength())
  const char * str = this->url[0].getString();
//    SbString text;
//    thisp->url.get1(0, text); // only the first url is used
//    if (text.getLength()>0)
  if ( (str == NULL) || (strlen(str)==0) )
    return FALSE; // url is blank

  // use SbImage::searchForFile to search directories
  
  ALint	error;

  // Delete previous buffer

  if (alIsBuffer(bufferId))
	  alDeleteBuffers(1, &bufferId);

  // Generate new buffer

  alGenBuffers(1, &bufferId);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::loadUrl",
                              "alGenBuffers failed. %s",
                              GetALErrorString(errstr, error));
		return FALSE;
	}

	ALsizei size,freq;
	ALenum	format;
	ALvoid	*data;
	ALboolean loop;

	// Load .wav
//	alutLoadWAVFile("lyd1.wav",&format,&data,&size,&freq,&loop);
//	alutLoadWAVFile(const_cast<ALbyte *>(text.getString()), &format, &data, &size, &freq, &loop);
	alutLoadWAVFile(const_cast<ALbyte *>(str), &format, &data, &size, &freq, &loop);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::loadUrl",
                              "Couldn't load file %s. %s",
//                              text.getString(), 
                              str, 
                              GetALErrorString(errstr, error));
		return FALSE;
	}

	// Copy wav data into buffer
	alBufferData(bufferId, format, data, size, freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::GLRender",
                              "alBufferData failed for data read from file %s. %s",
//                              text.getString(),
                              str,
                              GetALErrorString(errstr, error));
		return FALSE;
	}

	// Unload .wav
	alutUnloadWAV(format,data,size,freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::GLRender",
                              "alutUnloadWAV failed for data read from file %s. %s",
//                              text.getString(),
                              str,
                              GetALErrorString(errstr, error));
		return FALSE;
	}

  return TRUE;
};

//
// called when filename changes
//
void
SoSoundBuffer::urlSensorCB(void * data, SoSensor *)
{
  SoSoundBuffer * thisp = (SoSoundBuffer*) data;

  if (thisp->url.getNum()>0)
  {
    const char * str = thisp->url[0].getString();
//    SbString text;
//    thisp->url.get1(0, text); // only the first url is used
//    if (text.getLength()>0)
    if ( (str != NULL) && (strlen(str)>0) )
    {

      if (thisp->loadUrl())
      {
        thisp->readstatus = 1;
      }
      else
      {
        SoDebugError::postWarning("SoSoundBuffer::urlSensorCB",
                                  "Sound file could not be read: %s",
                                  str);
//                                  text.getString());
        thisp->readstatus = 0;
      }
    }
  }
}



class MyWin : public SoWin
{
public:
static void
mainLoop( void )
{
  MSG msg;
  while ( TRUE ) {
    if ( GetQueueStatus( QS_ALLINPUT ) != 0 ) { // if messagequeue != empty
      if ( GetMessage( & msg, NULL, 0, 0 ) ) { // if msg != WM_QUIT
        if (msg.message == WM_TIMER)
        {
          WPARAM wTimerID = msg.wParam;
          if (wTimerID==1)
          {
              int imax=30;

            static i=0;
            float x,y,z;
            SbVec3f cpos;
            cpos=camera->position.getValue();

            cpos.getValue(x, y, z);

            //    x=3.0-(float)i/(float)imax *3.0*2.0;

            x=1.0-(float)i/(float)imax *1.0*2.0;

//            camera->position.setValue(x, y, z);
            i++;
          }

        }
        else if ( (msg.message == MM_JOY1MOVE) 
               || (msg.message == MM_JOY1ZMOVE)
               )
//        else if (0)
        {
	        ALint	error;
          WPARAM fwButtons = msg.wParam; 
          int xPos = LOWORD(msg.lParam); 
          int yPos = HIWORD(msg.lParam); 

          JOYINFOEX ji;
          ji.dwFlags = JOY_RETURNALL;
          MMRESULT jres = joyGetPosEx(JOYSTICKID1, &ji);
          if (jres != JOYERR_NOERROR )
          {
            int i=0;
          }
 


            float x,y,z;
            SbVec3f cpos;
            cpos=camera->position.getValue();

            cpos.getValue(x, y, z);

            //    x=3.0-(float)i/(float)imax *3.0*2.0;

            x=1.0-(float)ji.dwXpos/(float)65535.0 *1.0*2.0;
            y=-(1.0-(float)ji.dwYpos/(float)65535.0 *1.0*2.0);
            z=5.0-2*(1.0-(float)ji.dwZpos/(float)65535.0 *1.0*2.0);

            camera->position.setValue(x, y, z);

            float rot = 1.0-(float)ji.dwRpos/(float)65535.0 *1.0*2.0;

            SbRotation orientation;
            orientation = camera->orientation.getValue();
            SbVec3f axis(1,0,0);
            camera->orientation.setValue(axis, rot);


/*
            listenerPos[0] = x;
            listenerPos[1] = y;
            listenerPos[2] = z;

	          // Position ...
	          alListenerfv(AL_POSITION,listenerPos);
	          if ((error = alGetError()) != AL_NO_ERROR)
	          {
		          DisplayALError("alListenerfv POSITION : ", error);
		          exit(-1);
	          }

	          // Velocity ...
	          alListenerfv(AL_VELOCITY,listenerVel);
	          if ((error = alGetError()) != AL_NO_ERROR)
	          {
		          DisplayALError("alListenerfv VELOCITY : ", error);
		          exit(-1);
	          }

	          // Orientation ...
	          alListenerfv(AL_ORIENTATION,listenerOri);
	          if ((error = alGetError()) != AL_NO_ERROR)
	          {
		          DisplayALError("alListenerfv ORIENTATION : ", error);
		          exit(-1);
	          }
*/

        }
        else if ( (msg.message == MM_JOY1BUTTONDOWN)
          || (msg.message == MM_JOY1BUTTONUP) )
        {
	        ALint	error;
          JOYINFOEX ji;
          ji.dwFlags = JOY_RETURNALL;
          MMRESULT jres = joyGetPosEx(JOYSTICKID1, &ji);
          if (jres != JOYERR_NOERROR )
          {
            int i=0;
          }

          if (ji.dwButtons & 0x01)
          {

            alSourcePlay(sourcebuf[0]);
				      if ((error = alGetError()) != AL_NO_ERROR)
              {
                char errstr[256];
		            SoDebugError::postWarning("SoSoundBuffer::GLRender",
                                          "alSourcePlay failed. %s",
                                          GetALErrorString(errstr, error));
              }
          }
          else
          {
				    alSourceStop(sourcebuf[0]);
				    if ((error = alGetError()) != AL_NO_ERROR)
              {
                char errstr[256];
		            SoDebugError::postWarning("SoSoundBuffer::GLRender",
                                          "alSourceStop failed. %s",
                                          GetALErrorString(errstr, error));
              }
          }

        }
        TranslateMessage( & msg );
	DispatchMessage( & msg );
      }
      else break; // msg == WM_QUIT
    }
    else if ( SoWin::idleSensorActive )
      SoWin::doIdleTasks( );
    else // !idleSensorActive
      WaitMessage( );
  }
}

};

SbBool myEventCallback( void * closure, MSG * event )
{
  if (event->message == MM_JOY1MOVE)
  {
    WPARAM wTimerID = event->wParam;
  }
  return false;
}


static void timerSensorCallback(void *data, SoSensor *)
{
  float x, y, z;
  static counter=0;
  SbVec3f cpos;
  cpos=xf->translation.getValue();
  cpos.getValue(x, y, z);
  x=x+0.1;
  xf->translation.setValue(x, y, z);
}


void
main(
  int argc,
  char ** argv )
{

  HWND window = SoWin::init( argv[0] );

  SoListener::initClass();
  SoSoundSource::initClass();
  SoSoundBuffer::initClass();


	ALCcontext *Context;
	ALCdevice *Device;
	ALint	error;

	//Open device
	Device = alcOpenDevice((ALubyte*)"DirectSound3D");

	if (Device == NULL)
	{
		printf("Failed to Initialize Open AL\n");
		exit(-1);
  }

	//Create context(s)
	Context=alcCreateContext(Device,NULL);
	//Set active context
	alcMakeContextCurrent(Context);

	// Clear Error Code
	alGetError();

/*

	// Generate Buffers
	alGenBuffers(NUM_BUFFERS, g_Buffers);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alGenBuffers :", error);
		exit(-1);
	}



	ALsizei size,freq;
	ALenum	format;
	ALvoid	*data;
	ALboolean loop;


	// Load footsteps.wav
	alutLoadWAVFile("lyd1.wav",&format,&data,&size,&freq,&loop);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alutLoadWAVFile footsteps.wav : ", error);
		// Delete Buffers
		alDeleteBuffers(NUM_BUFFERS, g_Buffers);
		exit(-1);
	}

	// Copy footsteps.wav data into AL Buffer 0
	alBufferData(g_Buffers[0],format,data,size,freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alBufferData buffer 0 : ", error);
		// Delete buffers
		alDeleteBuffers(NUM_BUFFERS, g_Buffers);
		exit(-1);
	}

	// Unload footsteps.wav
	alutUnloadWAV(format,data,size,freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alutUnloadWAV : ", error);
		// Delete buffers
		alDeleteBuffers(NUM_BUFFERS, g_Buffers);
		exit(-1);
	}

*/


	alGenSources(1,sourcebuf);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alGenSources failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
	
	alSourcef(sourcebuf[0],AL_PITCH,1.0f);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcef(,AL_PITCH,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

	alSourcef(sourcebuf[0],AL_GAIN,1.0f);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcef(,AL_GAIN,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
	
	alSourcefv(sourcebuf[0],AL_POSITION,source0Pos);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcefv(,AL_POSITION,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
	
	alSourcefv(sourcebuf[0],AL_VELOCITY,source0Vel);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcefv(,AL_VELOCITY,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

/*	alSourcei(sourcebuf[0],AL_BUFFER, g_Buffers[0]);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcei(,AL_BUFFER,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
*/

	alSourcei(sourcebuf[0],AL_LOOPING,AL_TRUE);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourcei(,AL_LOOPING,) failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }



  SoListener *listener;
  SoSoundSource *sourcenode;
  SoSoundBuffer *buffernode;

  root = new SoSeparator;

  root->addChild( camera = new SoPerspectiveCamera );
  root->addChild(listener = new SoListener );

  root->addChild( new SoDirectionalLight );

  SoShapeHints * hints = new SoShapeHints;
  hints->vertexOrdering = SoShapeHints::COUNTERCLOCKWISE;
  hints->shapeType = SoShapeHints::SOLID;
  hints->creaseAngle = 0.91f;
  root->addChild( hints );
  SoBaseColor * basecol = new SoBaseColor;
  basecol->rgb.setValue( float(rand())/float(RAND_MAX),
                         float(rand())/float(RAND_MAX),
                         float(rand())/float(RAND_MAX) );
  root->addChild( basecol );


	SoNode *node;
  SoSeparator *sep = new SoSeparator;

  xf = new SoTransform;
  xf->translation.setValue(1.3, 0, 0);
  
  sep->addChild(new SoCone);
  sep->addChild(xf);
  sep->addChild(node=new SoCone);
  sep->addChild(sourcenode = new SoSoundSource);
//  sep->addChild(listener = new SoListener);
  
  root->addChild(buffernode = new SoSoundBuffer);
  buffernode->url.setValue("lyd1.wav");
//  buffernode->url.setNum(1);
//  buffernode->url.set1(0, "lyd1.wav");
  sourcenode->source.setValue(buffernode);

//  root->addChild( node=new SoCone );
//  add separator and add several nodes
//  root->addChild( node=new SoListener );

  listener->orientation.connectFrom(&camera->orientation);
  listener->position.connectFrom(&camera->position);

  root->addChild(sep);

	SbViewportRegion vp;
	vp.setWindowSize(SbVec2s(400, 400));

  SoWinRenderArea * viewer = new SoWinRenderArea( window );

  //viewer->glModes;

  viewer->setEventCallback(myEventCallback, viewer);

  viewer->setSceneGraph( root );

	viewer->setViewportRegion(vp);
  camera->viewAll( node, vp);
//  camera->viewAll( sep, vp );

	SoTimerSensor *timerSensor;
	timerSensor = new SoTimerSensor(timerSensorCallback, NULL);
	timerSensor->setInterval(1);
	timerSensor->schedule();


  viewer->show();
  SoWin::show( window );

//  UINT ret = SetTimer(window, 1, 1000, NULL);

  MMRESULT mmres = joySetCapture(window, JOYSTICKID1, 20, TRUE);

//  SoWin::mainLoop();
  MyWin::mainLoop();

  joyReleaseCapture(mmres);

//  KillTimer(window, 1);

  delete viewer;

	// Release resources
	alSourceStopv(1, sourcebuf);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alSourceStopv() failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }

	alDeleteSources(1, sourcebuf);
	if ((error = alGetError()) != AL_NO_ERROR)
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alDeleteSources() failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }




/*	alDeleteBuffers(NUM_BUFFERS, g_Buffers);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
  {
    char errstr[256];
		SoDebugError::postWarning("SoSoundBuffer::xxx",
                              "alDeleteBuffers() failed. %s",
                              GetALErrorString(errstr, error));
    return;
  }
*/

	//Get active context
	Context=alcGetCurrentContext();
	//Get device for active context
	Device=alcGetContextsDevice(Context);
	//Release context(s)
	alcDestroyContext(Context);
	//Close device
	alcCloseDevice(Device);


} // main()



/*

  class SoListener : 
  public SoNode
  //public SoShape
{
  SO_NODE_HEADER(SoListener);

public:

  static void initClass();
  SoListener();

protected:

  virtual void GLRender(SoGLRenderAction *action);
//  virtual void generatePrimitives(SoAction *action);
//  virtual void computeBBox(SoAction *action, SbBox3f &box, SbVec3f &center);

private:

  virtual ~SoListener();
};


SO_NODE_SOURCE(SoListener);

void SoListener::initClass()
{
//  SO_NODE_INIT_CLASS(SoListener, SoShape, "Shape");
  SO_NODE_INIT_CLASS(SoListener, SoNode, "Node");
};

SoListener::SoListener()
{
  SO_NODE_CONSTRUCTOR(SoListener);
};

SoListener::~SoListener()
{
};

void SoListener::GLRender(SoGLRenderAction *action)
{
  SoState *state = action->getState();

  const SoCoordinateElement *coor = SoCoordinateElement::getInstance(state);
  int num = coor->getNum();
  SbVec3f pos = coor->get3(0);
  static int count=0;
  float x, y, z;
  pos.getValue(x, y, z);
//  SbVec3f pos = coor->getDefault3();
//    getValue
//  printf("Pos: (%0.2f, %0.2f, %0.2f) %d\n", count++);
  printf("%d  Num: %d, (%d, %d, %d)\n", count++ , num, x, y, z);
}





void SoListener::GLRender(SoGLRenderAction *action)
{
  ALint error;
  SoState *state = action->getState();

  const SoCoordinateElement *coor = SoCoordinateElement::getInstance(state);
  int num = coor->getNum();
  SbVec3f pos;
  pos = coor->get3(0);
  pos = position.getValue();
  static int count=0;
  float x, y, z;
  pos.getValue(x, y, z);
//  printf("%d  Num: %d, (%d, %d, %d)\n", count++ , num, x, y, z);
  printf("%d  Num: %d, (%0.2f, %0.2f, %0.2f)\n", count++ , num, x, y, z);

  pos = position.getValue();
  SbVec3f worldpos;
  SoModelMatrixElement::get(action->getState()).multVecMatrix(pos, worldpos); 
  worldpos.getValue(x, y, z);

  ALfloat listenerPos[3];

  listenerPos[0] = x;
  listenerPos[1] = y;
  listenerPos[2] = z;

	// Position ...
	alListenerfv(AL_POSITION,listenerPos);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alListenerfv POSITION : ", error);
		exit(-1);
	}

	// Velocity ...
	alListenerfv(AL_VELOCITY,listenerVel);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alListenerfv VELOCITY : ", error);
		exit(-1);
	}

  SbRotation or;
  or = orientation.getValue();
  SbVec3f oraxis;
  float orangle;
  or.getValue(oraxis, orangle);
  oraxis.getValue(x, y, z);

  SbVec3f viewdir;
  SbVec3f viewup;
  ALfloat listenerOri[6];
  
  this->orientation.getValue().multVec(SbVec3f(0,0,-1), viewdir);
  viewdir.getValue(x,y,z);
  listenerOri[0] = x;
  listenerOri[1] = y;
  listenerOri[2] = z;

  this->orientation.getValue().multVec(SbVec3f(0,1,0), viewup);
  viewup.getValue(x,y,z);
  listenerOri[3] = x;
  listenerOri[4] = y;
  listenerOri[5] = z;


//  listenerOri[0] = x;
//  listenerOri[1] = y;
//  listenerOri[2] = z;
//  listenerOri[0] = 0;
  listenerOri[1] = 0;
  listenerOri[2] = -1;

  listenerOri[3] = 0;
  listenerOri[4] = 1;
  listenerOri[5] = 0;

//  ALfloat	listenerOri[]={0.0, 0.0, -1.0, 0.0, 1.0, 0.0};	// Listener facing into the screen

	// Orientation ...
	alListenerfv(AL_ORIENTATION,listenerOri);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alListenerfv ORIENTATION : ", error);
		exit(-1);
	}

}




  */

/*

  #if COIN_DEBUG && 0 // flip 1<->0 to turn texture search trace on or off
#define TRY_FILE_DEBUG(x, result) \
  SoDebugError::postInfo("TRY_FILE", "texture search: %s (%s)", (x), (result))
#else // !COIN_DEBUG
#define TRY_FILE_DEBUG(x, result)
#endif // !COIN_DEBUG

#define TRY_FILE(x) \
  do { \
    FILE * fp = fopen(x.getString(), "rb"); \
    TRY_FILE_DEBUG(x.getString(), fp ? "hit!" : "miss"); \
    if (fp != NULL) { \
      fclose(fp); \
      return x; \
    } \
  } while (0)

SbString searchForFile(const SbString & basename,
                       const SbString * const * dirlist, const int numdirs,
                       const SbString * const * dirpflist, const int numdirpfs)
{

  int i;

  TRY_FILE(basename);

  SbString fullname = basename;

  SbBool trypath = TRUE;
  const char * strptr = basename.getString();
  const char * lastunixdelim = strrchr(strptr, '/');
  const char * lastdosdelim = strrchr(strptr, '\\');
  if (!lastdosdelim) {
    lastdosdelim = strrchr(strptr, ':');
    if (lastdosdelim) trypath = FALSE;
  }
  const char * lastdelim = SbMax(lastunixdelim, lastdosdelim);

  if (lastdelim && trypath) {
    SbString tmpstring;
    for (i = 0; i < numdirs; i++) {
      SbString dirname = *(dirlist[i]);
      int dirlen = dirname.getLength();

      if (dirlen > 0 &&
          dirname[dirlen-1] != '/' &&
          dirname[dirlen-1] != '\\' &&
          dirname[dirlen-1] != ':') {
        dirname += "/";
      }

      tmpstring.sprintf("%s%s", dirname.getString(),
                        fullname.getString());
      TRY_FILE(tmpstring);
    }
  }

  SbString base = lastdelim ?
    basename.getSubString(lastdelim-strptr + 1, -1) :
    basename;

  for (i = 0; i < numdirs; i++) {
    SbString dirname = *(dirlist[i]);
    int dirlen = dirname.getLength();

    if (dirlen > 0 &&
        dirname[dirlen-1] != '/' &&
        dirname[dirlen-1] != '\\' &&
        dirname[dirlen-1] != ':') {
      dirname += "/";
    }

    fullname.sprintf("%s%s", dirname.getString(),
                     base.getString());
    TRY_FILE(fullname);

    // also try come common texture/picture subdirectories
    fullname.sprintf("%stexture/%s", dirname.getString(),
                     base.getString());
    TRY_FILE(fullname);

    fullname.sprintf("%stextures/%s",
                     dirname.getString(),
                     base.getString());
    TRY_FILE(fullname);

    fullname.sprintf("%simages/%s",
                     dirname.getString(),
                     base.getString());
    TRY_FILE(fullname);

    fullname.sprintf("%spics/%s",
                     dirname.getString(),
                     base.getString());
    TRY_FILE(fullname);

    fullname.sprintf("%spictures/%s",
                     dirname.getString(),
                     base.getString());
    TRY_FILE(fullname);
  }

  // none found
  return SbString("");
}



*/

/*!
  Reads image data from \a filename. In Coin, simage is used to
  load image files, and several common file formats are supported.
  simage can be downloaded from our webpages.  If loading
  fails for some reason this method returns FALSE, and the instance
  is set to an empty image. If the file is successfully loaded, the
  file image data is copied into this class.

  If \a numdirectories > 0, this method will search for \a filename
  in all directories in \a searchdirectories.
*/
/*
SbBool
SbImage::readFile(const SbString & filename,
                  const SbString * const * searchdirectories,
                  const int numdirectories)
{

  SbString finalname = SbImage::searchForFile(filename, searchdirectories,
                                              numdirectories);
  if (finalname.getLength()) {
    int w, h, nc;
    unsigned char * simagedata = NULL;
    
    if (simage_wrapper()->available && simage_wrapper()->simage_read_image) {
      simagedata = simage_wrapper()->simage_read_image(finalname.getString(), &w, &h, &nc);
#if COIN_DEBUG
      if (!simagedata) {
        SoDebugError::post("SbImage::readFile", "%s", 
                           simage_wrapper()->simage_get_last_error ?
                           simage_wrapper()->simage_get_last_error() :
                           "Unknown error");
      }
#endif // COIN_DEBUG
    }

    if (simagedata) {
      this->setValue(SbVec2s((short)w, (short)h), nc, simagedata);
      if (simage_wrapper()->simage_free_image) {
        simage_wrapper()->simage_free_image(simagedata);
      }
#if COIN_DEBUG && 1 // debug
      else {
        SoDebugError::postInfo("SbImage::readFile",
                               "Couldn't free image.");
      }
#endif // debug
      return TRUE;
    }
  }

  this->setValue(SbVec2s(0,0), 0, NULL);
  return FALSE;

}
*/
