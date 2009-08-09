#include <string>
#include <iostream>

#include "SimulationDescription.hpp"

bool SimulationDescription::isValid()
{
  bool valid = false;
  if (mModelURI.size() > 1) valid = true;
  return valid;
}

void SimulationDescription::modelURI(const char* uri)
{
  mModelURI = std::string(uri);
}

const char* SimulationDescription::modelURI()
{
  return mModelURI.c_str();
}
