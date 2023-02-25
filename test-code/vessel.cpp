#include <Inventor/Wx/SoWx.h>
#include <Inventor/Wx/viewers/SoWxExaminerViewer.h>
#include <Inventor/SoDB.h>
#include <Inventor/SoInput.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoMatrixTransform.h>
#include <SmallChange/nodekits/SmVesselKit.h>
#include <SmallChange/nodekits/SmOceanKit.h>
#include <SmallChange/misc/Init.h>


static void
setup_vessel(SmVesselKit* vessel)
{
    SmOceanKit* ocean = new SmOceanKit;
    ocean->size.setValue(1000,1000);
    ocean->lightDirection.setValue(0, -2, -1);
    ocean->windDirection.setValue(0,1);
    ocean->distanceAttenuation.setValue(1,0.2, 0.0002);
    ocean->angleDeviation.setValue(30);
    ocean->amplitudeRatio.setValue(0.15);
    ocean->gravConst.setValue(5.0);
    vessel->oceanKit.setValue(ocean);
    vessel->speed.setValue(1.0);
}

int
main(int argc, char ** argv)
{
    wxWindow* window = SoWx::init( argv[0] );

    smallchange_init();
    SmVesselKit::initClass();

    SoSeparator * root = new SoSeparator;
    root->ref();

    SmVesselKit * vessel = new SmVesselKit;

    setup_vessel(vessel);

    root->addChild(vessel);

    SoWxExaminerViewer * examinerviewer = new SoWxExaminerViewer(window);
    examinerviewer->setSceneGraph(root);
    examinerviewer->show();
    SoWx::show( window );
    SoWx::mainLoop();

    delete examinerviewer;

    root->unref();

    return 0;
} // main()
