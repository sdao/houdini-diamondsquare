#include <limits.h>
#include <SYS/SYS_Math.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <CH/CH_LocalVariable.h>
#include <PRM/PRM_Include.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <stdexcept>
#include <boost/optional/optional.hpp>
#include "SOP_DiamondSquare.h"
#include "Grid.h"

void newSopOperator(OP_OperatorTable *table) {
  table->addOperator(
    new OP_Operator("diamond_square", // Internal name
                    "DiamondSquare", // UI name
                    SOP_DiamondSquare::myConstructor, // How to build the SOP
                    SOP_DiamondSquare::myTemplateList, // My parameters
                    0, // Min # of sources
                    0, // Max # of sources
                    SOP_DiamondSquare::myVariables, // Local variables
                    OP_FLAG_GENERATOR) // Flag it as generator
  );
}

PRM_Name SOP_DiamondSquare::roughnessName("rough", "Roughness");
PRM_Default SOP_DiamondSquare::roughnessDefault(0.1);
PRM_Range SOP_DiamondSquare::roughnessRange(PRM_RANGE_RESTRICTED, 0.0,
                                            PRM_RANGE_RESTRICTED, 1.0);
PRM_Default SOP_DiamondSquare::sizeDefaults[] = {
  PRM_Default(10.0),
  PRM_Default(2.0),
  PRM_Default(10.0)
};
PRM_Default SOP_DiamondSquare::divDefault(6);
PRM_Range SOP_DiamondSquare::divRange(PRM_RANGE_RESTRICTED, 1,
                                      PRM_RANGE_RESTRICTED, 10);
PRM_Default SOP_DiamondSquare::seedDefault(1234);
PRM_Range SOP_DiamondSquare::seedRange(PRM_RANGE_RESTRICTED, 0,
                                       PRM_RANGE_UI, 9999);

PRM_Template SOP_DiamondSquare::myTemplateList[] = {
  PRM_Template(PRM_INT_E, 1, &PRMseedName, &seedDefault, 0, &seedRange),
  PRM_Template(PRM_XYZ_J, 3, &PRMsizeName, sizeDefaults),
  PRM_Template(PRM_FLT_J, 1, &roughnessName, &roughnessDefault, 0,
    &roughnessRange),
  PRM_Template(PRM_INT_E, 1, &PRMdivName, &divDefault, 0, &divRange),
  PRM_Template()
};

// Here's how we define local variables for the SOP.
enum {
  VAR_PT,         // Point number of the star
  VAR_NPT         // Number of points in the star
};

CH_LocalVariable
SOP_DiamondSquare::myVariables[] = {
  { "PT",     VAR_PT, 0 },            // The table provides a mapping
  { "NPT",    VAR_NPT, 0 },           // from text string to integer token
  { 0, 0, 0 },
};

bool SOP_DiamondSquare::evalVariableValue(fpreal &val, int index, int thread) {
  if (curPointIdx >= 0) {
    // Note that "gdp" may be null here, so we do the safe thing
    // and cache values we are interested in.
    switch (index) {
      case VAR_PT:
        val = (fpreal)curPointIdx;
        return true;

      case VAR_NPT:
        val = (fpreal)totalPointCount;
        return true;

      default:
        /* do nothing */;
    }
  }

  // Not one of our variables, must delegate to the base class.
  return SOP_Node::evalVariableValue(val, index, thread);
}

OP_Node*
SOP_DiamondSquare::myConstructor(OP_Network *net,
                                 const char* name,
                                 OP_Operator* op) {
  return new SOP_DiamondSquare(net, name, op);
}

SOP_DiamondSquare::SOP_DiamondSquare(OP_Network *net,
                                     const char* name,
                                     OP_Operator* op)
  : SOP_Node(net, name, op) {
  curPointIdx = -1;
  totalPointCount = 0;
}

SOP_DiamondSquare::~SOP_DiamondSquare() {}

OP_ERROR SOP_DiamondSquare::cookMySop(OP_Context& context) {
  fpreal now = context.getTime();
  int seed = SEED();
  float height = HEIGHT(now);
  float width = WIDTH(now);
  float length = LENGTH(now);
  float rough = ROUGH(now);
  int div = 0x1 << DIV(); // 2^DIV.

  float halfWidth = width / 2.0f;
  float halfLength = length / 2.0f;

  UT_Interrupt *boss;

  Grid grid(width, length, div);
  grid.DiamondSquare(seed, rough, height);

  // Run through unless we detect an interrupt.
  if (error() < UT_ERROR_ABORT) {
    boss = UTgetInterrupt();
    gdp->stashAll();

    try {
      // Start the interrupt server
      if (boss->opStart("Building Diamond-Square Terrain")) {
        GA_RWHandleV3 uv(gdp->addFloatTuple(GA_ATTRIB_POINT,
                                            GEO_STD_ATTRIB_TEXTURE,
                                            3));

        int widthVtx = grid.GetWidthVertices();
        int lengthVtx = grid.GetLengthVertices();

        int widthFaces = grid.GetWidthSegs();
        int lengthFaces = grid.GetLengthSegs();

        float maxU = (float)widthFaces;
        float maxV = (float)lengthFaces;

        GEO_Point* pt = NULL;
        boost::optional<GA_Offset> firstOfs;

        totalPointCount =  widthVtx * lengthVtx;
        curPointIdx = 0;
        for (int i = 0; i < widthVtx; i++) {
          for (int j = 0; j < lengthVtx; j++) {
            if (boss->opInterrupt())
              throw std::runtime_error("Interrupted");

            pt = gdp->appendPoint();
            pt->setPos(i * width / widthFaces - halfWidth,
                       grid.GetVertexHeightWrap(i, j),
                       j * length / lengthFaces - halfLength);
            uv.set(pt->getMapOffset(), UT_Vector3(i / maxU,
                                                  j / maxV,
                                                  0.0));

            if (!firstOfs) {
              firstOfs = pt->getMapOffset();
            }

            ++curPointIdx;
          }
        }
        curPointIdx = -1;

        int totalQuadsCount = widthFaces * lengthFaces;
        GEO_PolyCounts polyCounts;
        polyCounts.append(4, totalQuadsCount);
        int* polyPoints = new int[4 * totalQuadsCount];

        int polyPointsIdx = 0;
        for (int i = 0; i < widthFaces; i++) {
          for (int j = 0; j < lengthFaces; j++) {
            if (boss->opInterrupt())
              throw std::runtime_error("Interrupted");

            polyPoints[polyPointsIdx + 0] = (i + 1) * widthVtx + j;
            polyPoints[polyPointsIdx + 1] = (i + 1) * widthVtx + j + 1;
            polyPoints[polyPointsIdx + 2] = i * widthVtx + j + 1;
            polyPoints[polyPointsIdx + 3] = i * widthVtx + j;
            polyPointsIdx += 4;
          }
        }

        GU_PrimPoly::buildBlock(gdp,
                                *firstOfs,
                                totalPointCount,
                                polyCounts,
                                polyPoints);
      }
    }
    catch (std::runtime_error&) {
      // Do nothing. Just stop whatever we were doing and clean up.
      curPointIdx = -1;
    }

    // Tell the interrupt server that we've completed. Must do this
    // regardless of what opStart() returns.
    boss->opEnd();
    gdp->destroyStashed();
  }

  return error();
}
