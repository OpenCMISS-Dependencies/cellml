#ifndef _SIMULATIONDESCRIPTION_H_
#define _SIMULATIONDESCRIPTION_H_

/**
 * Contains a complete description of a CellML simulation.
 * About the only thing we need from a CellML simulation is the URI of the model to be used in the simulation. For now we are ignoring all other information as openCMISS will override the numerical methods and integration paramters.
 */
class SimulationDescription
{
 public:
  /** 
   * Check if the simulation description is currently valid.
   * @return true if valid; false otherwise.
   */
  bool isValid();
  /** 
   * Set the URI of the CellML model defined for this simulation.
   * @param uri The URI of the CellML model.
   */
  void modelURI(const char* uri);
  /** 
   * Get the URI of the CellML model defined for this simulation.
   * @return The URI of the CellML model.
   */
  const char* modelURI();

 private:
  std::string mModelURI;
};

#endif // _SIMULATIONDESCRIPTION_H_
