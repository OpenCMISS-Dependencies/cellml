#ifndef _CELLMLMODELDEFINITION_H_
#define _CELLMLMODELDEFINITION_H_

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
  bool instantiated()
  {
    return mInstantiated;
  }

  int32_t nBound;
  int32_t nRates;
  int32_t nAlgebraic;
  int32_t nConstants;

  // loaded from the generated and compiled DSO
  /* Initialise all variables which aren't state variables but have an
   * initial_value attribute, and any variables & rates which follow.
   */
  void (*SetupFixedConstants)(double* CONSTANTS,double* RATES,double* STATES);
  /* Compute all rates which are not static
   */
  void (*ComputeRates)(double VOI,double* STATES,double* RATES,
    double* CONSTANTS,double* ALGEBRAIC);
  /* Compute all variables not computed by initConsts or rates
   *  (i.e., these are not required for the integration of the model and
   *   thus only need to be called for output or presentation or similar
   *   purposes)
   */
  void (*EvaluateVariables)(double VOI,double* CONSTANTS,double* RATES,
    double* STATES,double* ALGEBRAIC);

 private:
  /**
   * Generate C code for the given CellML model.
   * @param model The CellML model for which to generate C code.
   * @return The generated code, if successful; and empty string otherwise.
   */
  std::wstring getModelAsCCode(void* model);
  std::string mURL;
  std::string mTmpDirName;
  bool mTmpDirExists;
  std::string mCodeFileName;
  bool mCodeFileExists;
  std::string mDsoFileName;
  bool mDsoFileExists;
  std::string mCompileCommand;
  bool mSaveTempFiles;
  void* mHandle;
  bool mInstantiated;

  // need to access these?
 public:
  void* mModel;
  void* mCodeInformation;
  void* mAnnotations;
};

#endif // _CELLMLMODELDEFINITION_H_
