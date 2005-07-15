#ifndef SM_PIECHART_H
#define SM_PIECHART_H

#include <Inventor/fields/SoSFFloat.h>
#include <Inventor/fields/SoSFEnum.h>
#include <Inventor/fields/SoMFColor.h>
#include <Inventor/fields/SoMFFloat.h>
#include <Inventor/nodekits/SoBaseKit.h>

#include <SmallChange/Basic.h>

// *************************************************************************

class SmPieChartP;

class SMALLCHANGE_DLL_API SmPieChart : public SoBaseKit {
  typedef SoBaseKit inherited;
  SO_KIT_HEADER(SmPieChart);

  SO_KIT_CATALOG_ENTRY_HEADER(topSeparator);

public:
  static void initClass(void);
  SmPieChart(void);

  enum ValueType {
    ITEM_SIZE,
    ITEM_BORDER
  };

  SoSFFloat height;
  SoSFFloat radius;

  SoSFEnum valueType;
  SoMFFloat values;
  SoMFColor colors;
  SoMFFloat retraction;

  void generateGeometry(void);

protected:
  virtual ~SmPieChart(void);

  virtual SbBool readInstance(SoInput * input, unsigned short flags);

private:
  SmPieChartP * pimpl;

}; // SmPieChart

// *************************************************************************

#endif // !SM_PIECHART_H
