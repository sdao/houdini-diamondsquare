#include <SOP/SOP_Node.h>

class SOP_DiamondSquare : public SOP_Node {
public:
  static OP_Node *myConstructor(OP_Network* network,
                                const char* name,
                                OP_Operator* op_type);
  static PRM_Template myTemplateList[];
  static CH_LocalVariable myVariables[];

protected:
  SOP_DiamondSquare(OP_Network* network,
                    const char* name,
                    OP_Operator* op_type);
  virtual ~SOP_DiamondSquare();
  virtual OP_ERROR cookMySop(OP_Context &context); // Meat of the plugin.

  // This function is used to lookup local variables that you have
  // defined specific to your SOP.
  virtual bool evalVariableValue(fpreal &val, int index, int thread);
  // Add virtual overload that delegates to the super class to avoid
  // shadow warnings.
  virtual bool evalVariableValue(UT_String &v, int i, int thread) {
    return evalVariableValue(v, i, thread);
  }

private:
  static PRM_Name roughnessName;
  static PRM_Default roughnessDefault;
  static PRM_Range roughnessRange;
  static PRM_Default sizeDefaults[];
  static PRM_Default divDefault;
  static PRM_Range divRange;
  static PRM_Default seedDefault;
  static PRM_Range seedRange;

  int SEED()              { return evalInt  (PRMseedName.getToken(), 0, 0); }
  float HEIGHT(fpreal t)  { return evalFloat(PRMsizeName.getToken(), 1, t); }
  float WIDTH(fpreal t)   { return evalFloat(PRMsizeName.getToken(), 0, t); }
  float LENGTH(fpreal t)  { return evalFloat(PRMsizeName.getToken(), 2, t); }
  float ROUGH(fpreal t)   { return evalFloat(roughnessName.getToken(), 0, t); }
  int DIV()               { return evalInt  (PRMdivName.getToken(), 0, 0); }

  int curPointIdx;
  int totalPointCount;
};
