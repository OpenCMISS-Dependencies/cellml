#include <iostream>
#include <wchar.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "occellml_config.h"

#include "CellMLModelDefinitionF.h"
#include "CellMLModelDefinition.hpp"
#include "ccgs_required_functions.h"

#ifdef _MSC_VER
#	include <direct.h>
#	define getcwd _getcwd
#endif

static char* getAbsoluteURI(const char* uri);

void* create_cellml_model_definition_f(const char* uri)
{
  void* cellml_model_definition = (void*)NULL;
  if (uri && strlen(uri) > 1)
  {
    // make sure we have an absolute URI
    std::cout << "uri: \"" << uri << "\"" << std::endl;
    char* inputURI = getAbsoluteURI(uri);
    std::cout << "C uri = \"" << inputURI << "\"" << std::endl;
    /*
     * Create the model defintion object
     */
    CellMLModelDefinition* def = new CellMLModelDefinition(inputURI);
    free(inputURI);
    cellml_model_definition = (void*)def;
  }
  else
  {
    std::cerr << "Invalid model URI" << std::endl;
  }
  return(cellml_model_definition);
}

void destroy_cellml_model_definition_f(void** _ptr)
{
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && *_ptr && (def = (CellMLModelDefinition*)(*_ptr)))
  {
    delete def;
    // want to make sure that we don't try to delete it again...
    *_ptr = (void*)NULL;
    _ptr = (void**)NULL;
    std::cout << "C destroying CellML model definition." << std::endl;
  }
  else
  {
    std::cerr << "[destroy_cellml_model_definition_f] Invalid arguments." 
	      << std::endl;
  }
}

int cellml_model_definition_get_initial_value_f(void* _ptr,const char* name,double* value)
{
  int return_code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    return_code = def->getInitialValue(name,value);
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_initial_value_f] "
        << "Invalid arguments." << std::endl;
  }
  return return_code;
}

int cellml_model_definition_get_initial_value_by_index_f(void* _ptr,const int* const type,const int* const index,double* value)
{
  int return_code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if ((*type > 0) && (*index > 0) && _ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    // need to map from 0-indexed C arrays to 1-indexed Fortran arrays
    int i = (*index)-1;
    return_code = def->getInitialValueByIndex((*type),i,value);
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_initial_value_by_index_f] "
        << "Invalid arguments. " << *type << " " << *index << std::endl;
  }
  return return_code;
}

int cellml_model_definition_get_variable_type_f(void* _ptr,const char* name,int* variable_type)
{
  int return_code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    return_code = def->getVariableType(name,variable_type);
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_variable_type_f] "
        << "Invalid arguments." << std::endl;
  }
  return return_code;
}

int cellml_model_definition_get_variable_index_f(void* _ptr,const char* name,int* variable_index)
{
  int return_code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    std::cout << "getting index of variable: '" << name << "'" << std::endl;
    return_code = def->getVariableIndex(name,variable_index);
    // need to map from 0-indexed C arrays to 1-indexed Fortran arrays
    if (return_code == 0) (*variable_index) += 1;
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_variable_index_f] "
        << "Invalid arguments." << std::endl;
  }
  return return_code;
}

int cellml_model_definition_set_variable_as_known_f(void* _ptr,const char* name)
{
  int return_code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    return_code = def->setVariableAsKnown(name);
  }
  else
  {
    std::cerr << "[cellml_model_definition_set_variable_as_known_f] "
        << "Invalid arguments." << std::endl;
  }
  return return_code;
}

int cellml_model_definition_set_variable_as_wanted_f(void* _ptr,const char* name)
{
  int return_code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    return_code = def->setVariableAsWanted(name);
  }
  else
  {
    std::cerr << "[cellml_model_definition_set_variable_as_wanted_f] "
        << "Invalid arguments." << std::endl;
  }
  return return_code;
}

void cellml_model_definition_set_save_temp_files_f(void* _ptr,int state)
{
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    if (state != 0) def->saveTempFiles(true);
    else def->saveTempFiles(false);
  }
  else
  {
    std::cerr << "[cellml_model_definition_set_save_temp_files_f] "
	      << "Invalid arguments." << std::endl;
  }
}

int cellml_model_definition_get_save_temp_files_f(void* _ptr)
{
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    if (def->saveTempFiles()) return 1;
    else return 0;
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_save_temp_files_f] "
	      << "Invalid arguments." << std::endl;
  }
  return -1;
}

int cellml_model_definition_instantiate_f(void* _ptr)
{
  int code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr))
  {
    code = def->instantiate();
  }
  else
  {
    std::cerr << "[cellml_model_definition_instantiate_f] "
	      << "Invalid arguments." << std::endl;
  }
  return code;
}

int cellml_model_definition_get_n_constants_f(void* _ptr)
{
  int code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr) && def->instantiated())
  {
    code = def->mParameterCounter;
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_n_constants_f] "
	      << "Invalid arguments." << std::endl;
  }
  return code;
}

int cellml_model_definition_get_n_rates_f(void* _ptr)
{
  int code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr) && def->instantiated())
  {
    code = def->mStateCounter;
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_n_rates_f] "
	      << "Invalid arguments." << std::endl;
  }
  return code;
}

int cellml_model_definition_get_n_algebraic_f(void* _ptr)
{
  int code = -1;
  CellMLModelDefinition* def = (CellMLModelDefinition*)NULL;
  if (_ptr && (def = (CellMLModelDefinition*)_ptr) && def->instantiated())
  {
    code = def->mIntermediateCounter;
  }
  else
  {
    std::cerr << "[cellml_model_definition_get_n_algebraic_f] "
	      << "Invalid arguments." << std::endl;
  }
  return code;
}

void cellml_model_definition_call_rhs_routine_f(void* _ptr,
  double voi,double* states, double* rates, double* wanted,
  double* known)
{
  // no error checks to optimise?
  CellMLModelDefinition* def = (CellMLModelDefinition*)_ptr;
  def->rhsRoutine(voi,states,rates,wanted,known);
}

static char* getAbsoluteURI(const char* uri)
{
  if (uri)
  {
    if (strstr(uri,"://") != NULL)
    {
      /*printf("URI (%s) already absolute.\n",uri);*/
      char* abs = (char*)malloc(strlen(uri)+1);
      strcpy(abs,uri);
      return(abs);
    }
    else if (uri[0]=='/')
    {
      /*printf("URI (%s) absolute path, making absolute URI: ",uri);*/
      char* abs = (char*)malloc(strlen(uri)+1+7);
      sprintf(abs,"file://%s",uri);
      /*printf("%s\n",abs);*/
      return(abs);
    }
    else
    {
      /* relative filename ? append absoulte path */
      /*printf("URI (%s) is relative path, making absolute URI: ",uri);*/
		int size;
#ifdef WIN32
		size = _MAX_PATH;
#else
		size = pathconf(".",_PC_PATH_MAX);
#endif
      char* cwd = (char*)malloc(size);
      if (getcwd(cwd,size))
			{
				// FIXME: should check for an error...
			}
      char* abs = (char*)malloc(strlen(cwd)+strlen(uri)+1+8);
      sprintf(abs,"file://%s/%s",cwd,uri);
      free(cwd);
      // and make sure \'s become /'s
      unsigned int i;
      for (i=0; i < strlen(abs); ++i) if (abs[i] == '\\') abs[i] = '/';
      /*printf("%s\n",abs);*/
      return(abs);
    }
  }
  return((char*)NULL);
}
