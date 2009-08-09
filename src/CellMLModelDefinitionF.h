#ifndef _CELLMLMODELDEFINITIONF_H_
#define _CELLMLMODELDEFINITIONF_H_

#ifdef __cplusplus
extern "C"
{
#endif

  /**
   * Create the CellMLModelDefinition from the provided URI.
   * @param uri NULL-terminated string containing the URI of the model definition.
   * @return A newly created cellml_model_definition_f object.
   */
  void* create_cellml_model_definition_f(const char* uri);

  /**
   * Destroy an allocated CellMLModelDefinition.
   * @param model An existing CellML model definition object
   */
  void destroy_cellml_model_definition_f(void** model);

  /**
   * Set the save temporary files flag.
   * @param model The CellML model definition to use.
   * @param state Should be set to non-zero to save generated temporary files.
   */
  void cellml_model_definition_set_save_temp_files_f(void* model,int state);

  /**
   * Get the current state of the save temporary files flag.
   * @param model The CellML model definition to use.
   * @return 1 if the current state is true (save), zero if it is false, and -1 in case of error.
   */
  int cellml_model_definition_get_save_temp_files_f(void* model);

  /**
   * Instantiate the CellML model definition into simulat-able code.
   * @return 0 if success; non-zero otherwise.
   */
  int cellml_model_definition_instantiate_f(void* model);

  /**
   * Get the required size of the constants array for the given instantiated model.
   * @param model The CellML model definition to use.
   * @return The required size of the constants array.
   */
  int cellml_model_definition_get_n_constants_f(void* model);
  /**
   * Get the required size of the rates array for the given instantiated model.
   * @param model The CellML model definition to use.
   * @return The required size of the rates array.
   */
  int cellml_model_definition_get_n_rates_f(void* model);
  /**
   * Get the required size of the algebraic array for the given instantiated model.
   * @param model The CellML model definition to use.
   * @return The required size of the algebraic array.
   */
  int cellml_model_definition_get_n_algebraic_f(void* model);

  /**
   * Call the model's SetupFixedConstant method.
   * @param model The CellML model definition to use.
   * @param constants The array to use for constants.
   * @param rates The array to use for rates.
   * @param states The array to use for states.
   */
  void cellml_model_definition_call_setup_fixed_constants_f(void* model,
    double* constants, double* rates, double* states);

  /**
   * Call the model's ComputeRates method.
   * @param model The CellML model definition to use.
   * @param voi The current value of the variable of integration.
   * @param states The array to use for states.
   * @param rates The array to use for rates.
   * @param rates The array to use for constants.
   * @param rates The array to use for algebraic.
   */
  void cellml_model_definition_call_compute_rates_f(void* model,
    double voi, double* states, double* rates, double* constants, 
    double* algebraic);

  /**
   * Call the model's EvaluateVariables method.
   * @param model The CellML model definition to use.
   * @param voi The current value of the variable of integration.
   * @param states The array to use for states.
   * @param rates The array to use for rates.
   * @param rates The array to use for constants.
   * @param rates The array to use for algebraic.
   */
  void cellml_model_definition_call_evaluate_variables_f(void* model,
    double voi, double* states, double* rates, double* constants, 
    double* algebraic);

#ifdef __cplusplus
} /* extern C */
#endif

#endif // _CELLMLMODELDEFINITIONF_H_

