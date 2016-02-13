#ifndef _CELLMLMODELDEFINITION_H_
#define _CELLMLMODELDEFINITION_H_

#include <map>
#include <vector>
#include <IfaceCCGS.hxx>

#ifdef CELLML_USE_CSIM
#include "csim/model.h"
#endif

/**
 * The primary object used to define a CellML model for use in openCMISS.
 *
 * This is the interface object sitting between a CellML description of a mathematical model and the use of that model in openCMISS.
 */
class CellMLModelDefinition
{
 public:
  /**
   * Default constructor
   */
  CellMLModelDefinition();
  /**
   * TODO: Need to do a copy constructor
   */
  CellMLModelDefinition(const CellMLModelDefinition& src) {}

  /**
   * Construct a model definition from a given source document at the givien URL.
   * @param url The URL of the source document form which to create the model defintion.
   */
  CellMLModelDefinition(const char* url);
  /**
   * Destructor.
   */
  ~CellMLModelDefinition();

  /**
   * Get the initial value (if specified) of the named variable.
   * @param name The name of the model variable to get the initial value of. This string
   * should be in the format of 'component_name/variable_name'.
   * @param value On successful exit, will be the initial value of the named variable; otherwise uninitialised.
   * @return zero if no error occured; otherwise non-zero to indicate and error occured and value is not set.
   */
  int getInitialValue(const char* name,double* value);

  /**
   * Get the initial value (if specified) of the index'th variable of the given type.
   * Will search the local components of this model for a variable flagged with the specified type and assigned the
   * specified index. If that variable (or its corresponding source variable) has an initial value, return that value.
   * \todo Really need to look at using the evaluation type constant to do this, as it is possible to assign initial
   * values and parameters using 'x = value' type equations.
   * @param type The type of the variable to look for.
   * @param index The index of the variable to look for (this is the index of the variable in the array that corresponds
   * to the given type.
   * @param value On successful exit, will be the initial value of the found variable; otherwise uninitialised.
   * @return zero if no error occured; otherwise non-zero to indicate and error occured and value is not set.
   */
  int getInitialValueByIndex(const int type,const int index,double* value);

  /**
   * Get the current type of the specified variable.
   * @param name The name of the model variable to get the type of. This string should be in the
   * format of 'component_name/variable_name'.
   * @param type On successful exit, will be the type of the named variable; otherwise uninitialised. State=1, known=2, wanted=3, independent=4?
   * @return zero if no error occured; otherwise non-zero to indicate and error occured and variable_type is not set.
   */
  int getVariableType(const char* name,int* type);

  /**
   * Get the current index of the specified variable. C-style index will be returned (starting from 0).
   * @param name The name of the model variable to get the index of. This string
   * should be in the format of 'component_name/variable_name'.
   * @param index On successful exit, will be the C index of the named variable; otherwise uninitialised.
   * @return zero if no error occured; otherwise non-zero to indicate and error occured and variable_index is not set.
   */
  int getVariableIndex(const char* name,int* index);

  /**
   * Flag the specified variable as being 'known' for the purposes of code generation. This implies
   * that the variable will have its value set externally to the CellML model.
   * @param name The name of the model variable to flag. This string should be in the format of 'component_name/variable_name'.
   * @return 0 if no error, non-zero otherwise.
   */
  int setVariableAsKnown(const char* name);

  /**
   * Flag the specified variable as being 'wanted' for the purposes of code generation. This implies
   * that the variable will have its value used externally to the CellML model.
   * @param name The name of the model variable to flag. This string should be in the format of 'component_name/variable_name'.
   * @return 0 if no error, non-zero otherwise.
   */
  int setVariableAsWanted(const char* name);

  /**
   * Instantiate the model definition into simulat-able code.
   * @return 0 if success; non-zero otherwise.
   */
  int instantiate();

#ifndef CELLML_USE_CSIM
  /**
   * Set the compile command to use to compile the generated code into a dynamic shared object.
   * @param command The compile command.
   */
  void compileCommand(const std::string& command)
  {
    mCompileCommand = command;
  }
  /**
   * Get the current compile command used to compile generated code.
   * @return The current compile command.
   */
  std::string compileCommand()
  {
    return mCompileCommand;
  }
#endif

  /**
   * Set the save temporary files flag.
   * @param state Should be set to true to save generated temporary files.
   */
  void saveTempFiles(bool state)
  {
    mSaveTempFiles = state;
  }
  /**
   * Get the current state of the save temporary files flag.
   * @return The state of the save temporary files flag.
   */
  bool saveTempFiles()
  {
    return mSaveTempFiles;
  }


  /**
   * Check model instantiation.
   * @return True if the model is instantiated; false otherwise.
   */
  bool instantiated();

  int32_t nBound;
  int32_t nRates;
  int32_t nAlgebraic;
  int32_t nConstants;

  // loaded from the generated and compiled DSO
  /* Compute the RHS of the system of the ODE system
   */
  void (*rhsRoutine)(double VOI,double* STATES,double* RATES,
    double* WANTED,double* KNOWN);

 private:
#ifndef CELLML_USE_CSIM
  /**
   * Generate C code for the given CellML model.
   * @param model The CellML model for which to generate C code.
   * @param cevas The variable mapping collection for the given model.
   * @param annotations The annotation set used to annotate known and wanted variables.
   * @return The generated code, if successful; and empty string otherwise.
   */
  std::wstring getModelAsCCode(void* model,void* cevas,void* annotations);
#endif

  int flagVariable(const char* name, int type,
                   std::vector<iface::cellml_services::VariableEvaluationType> vets,int& count, int& specificCount);
  std::string mURL;

#ifdef CELLML_USE_CSIM
  csim::Model* model;
#else
  std::string mTmpDirName;
  bool mTmpDirExists;
  std::string mCodeFileName;
  bool mCodeFileExists;
  std::string mDsoFileName;
  bool mDsoFileExists;
  std::string mCompileCommand;
  bool mInstantiated;
#endif

  bool mSaveTempFiles;
  void* mHandle;
  std::map<std::pair<int,int>, double> mInitialValues;
  std::map<std::string, int> mVariableTypes;
  std::map<std::string, int> mVariableIndices;

  // need to access these?
 public:
  void* mModel;
#ifndef CELLML_USE_CSIM
  void* mCodeInformation;
  void* mAnnotations;
  void* mCevas;
#endif
  int mNumberOfWantedVariables;
  int mNumberOfKnownVariables;
  int mNumberOfIndependentVariables;
  int mStateCounter;
  int mIntermediateCounter;
  int mParameterCounter;
};

#endif // _CELLMLMODELDEFINITION_H_
