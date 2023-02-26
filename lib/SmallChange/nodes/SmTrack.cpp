#ifdef HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#include <SmallChange/nodes/SmTrack.h>

#ifdef __COIN__
#include <Inventor/system/gl.h>
#else // SGI/TGS Inventor
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#include <GL/gl.h>
#endif // SGI/TGS Inventor

#include <Inventor/SbRotation.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/bundles/SoMaterialBundle.h>
#include <Inventor/elements/SoViewVolumeElement.h>
#include <Inventor/sensors/SoFieldSensor.h>

#define PRIVATE(obj) (obj)->pimpl
#define PUBLIC(obj) (obj)->master

class SmTrackP {
public:
    SmTrackP(void)
            : startix(0) {}

    void updateInterval(void)
    {
        float t = PUBLIC(this)->trackLength.getValue();
        int n = PUBLIC(this)->timeStamps.getNum();
        int i = n-1;
        SbTime starttime = PUBLIC(this)->timeStamps[i];

        this->startix = -1;
        for (; i >= 0; i--) {
            SbTime tracktime = PUBLIC(this)->timeStamps[i];
            SbTime interval(starttime - tracktime);
            if (interval.getValue() >= PUBLIC(this)->trackLength.getValue()) {
                this->startix = i;
                break;
            }
        }
        if (this->startix<0) this->startix = 0;
    }

    static void updateIntervalCB(void * closure, SoSensor *)
    {
        SmTrackP * thisp = (SmTrackP *) closure;
        thisp->updateInterval();
    }

    int startix, endix;
    SmTrack * master;
    SoFieldSensor * tracklengthsensor;
};

SO_NODE_SOURCE(SmTrack);

void
SmTrack::initClass(void)
{
    SO_NODE_INIT_CLASS(SmTrack, SoShape, "Shape");
}

SmTrack::SmTrack(void)
{
    SO_NODE_CONSTRUCTOR(SmTrack);

    SO_NODE_ADD_FIELD(track, (SbVec3d(0.0, 0.0, 0.0)));
    SO_NODE_ADD_FIELD(timeStamps, (SbTime::getTimeOfDay()));
    SO_NODE_ADD_FIELD(trackLength, (22.0f));

    SO_NODE_ADD_FIELD(pointInterval, (1));
    SO_NODE_ADD_FIELD(lineInterval, (0));
    SO_NODE_ADD_FIELD(tickInterval, (0));
    SO_NODE_ADD_FIELD(tickSize, (1.0f));

    PRIVATE(this) = new SmTrackP;
    PRIVATE(this)->master = this;

    PRIVATE(this)->tracklengthsensor =
            new SoFieldSensor(SmTrackP::updateIntervalCB, PRIVATE(this));
    PRIVATE(this)->tracklengthsensor->attach(&this->trackLength);
}

SmTrack::~SmTrack()
{
    delete PRIVATE(this)->tracklengthsensor;
    delete PRIVATE(this);
}

void
SmTrack::GLRender(SoGLRenderAction * action)
{
    if (!this->shouldGLRender(action)) return;
    if (this->trackLength.getValue() == 0.0) return;

    SoMaterialBundle mb(action);
    mb.sendFirst();

    GLboolean lighting = glIsEnabled(GL_LIGHTING);
    if (lighting) glDisable(GL_LIGHTING);

    unsigned short pointInterval = this->pointInterval.getValue();
    if (pointInterval > 0) {
        glBegin(GL_POINTS);
        for (int i = PRIVATE(this)->startix; i < this->track.getNum(); i += pointInterval) {
            glVertex3dv(this->track[i].getValue());
        }
        glEnd();
    }

    unsigned short lineInterval = this->lineInterval.getValue();
    if (lineInterval > 0) {
        glBegin(GL_LINES);
        for (int i = PRIVATE(this)->startix + 1; i < this->track.getNum(); i += lineInterval) {
            glVertex3dv(this->track[i - 1].getValue());
            glVertex3dv(this->track[i].getValue());
        }
        glEnd();
    }

    unsigned short tickInterval = this->tickInterval.getValue();
    if (tickInterval > 0) {
        float s = this->tickSize.getValue();
        SbRotation r(SbVec3f(0, 0, 1), 0.125f * float(M_PI));
        for (int i = PRIVATE(this)->startix + 1; i < this->track.getNum(); i += tickInterval) {
            SbVec3f p = SbVec3f(this->track[i]);
            SbVec3f v = SbVec3f(this->track[i - 1]) - p;

#if 1
            v *=  s / v.length();
#else
            v *= SoViewVolumeElement::get(action->getState()).getWorldToScreenScale(p, s) / v.length();
#endif

            SbVec3f left, right;
            r.multVec(v, left);
            r.inverse().multVec(v, right);
            left += p;
            right += p;
            glBegin(GL_LINE_STRIP);
            glVertex3fv(left.getValue());
            glVertex3fv(p.getValue());
            glVertex3fv(right.getValue());
            glEnd();
        }
    }

    if (lighting) glEnable(GL_LIGHTING);
}

void
SmTrack::append(const SbVec3d & pos,
                const SbTime & timestamp)
{
    int index = this->timeStamps.getNum();
    assert(index == this->track.getNum());
    this->timeStamps.set1Value(index, timestamp);
    this->track.set1Value(index, pos);

    PRIVATE(this)->updateInterval();
}


void
SmTrack::deleteValues()
{
    this->track.deleteValues(0, this->track.getNum());
    this->timeStamps.deleteValues(0, this->timeStamps.getNum());
}


void
SmTrack::computeBBox(SoAction * action, SbBox3f & box,
                     SbVec3f & center)
{
    box.makeEmpty();

    for (int i = PRIVATE(this)->startix; i < this->track.getNum(); i++) {
        SbVec3d coord = this->track[i].getValue();
        box.extendBy(SbVec3f(float(coord[0]), float(coord[1]), float(coord[2])));
    }
    if (!box.isEmpty()) center = box.getCenter();
}

void
SmTrack::generatePrimitives(SoAction * action)
{
    // FIXME: implement! (20060602 frodo)
}

#undef PRIVATE
#undef PUBLIC
