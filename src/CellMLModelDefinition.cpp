#include <stdlib.h>
#include <string>
#include <string.h>
#include <wchar.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#ifdef DYNAMIC_COMPILE
#  include <unistd.h>
#  include <dlfcn.h>
#endif

#ifdef _MSC_VER
#	include <direct.h>
#endif

/* needs to be before CellML headers to avoid conflict with uint32_t in cda_config.h */
#include "occellml_config.h"

#include <IfaceCellML_APISPEC.hxx>
#include <IfaceCCGS.hxx>
#include <CeVASBootstrap.hpp>
#include <MaLaESBootstrap.hpp>
#include <CCGSBootstrap.hpp>
#include <CellMLBootstrap.hpp>
#include <IfaceAnnoTools.hxx>
#include <AnnoToolsBootstrap.hpp>

#include "CellMLModelDefinition.hpp"
#include "utils.hxx"

/*
 * Prototype local methods
 */
typedef std::pair<std::string,std::string> CVpair;
static CVpair splitName(const char* s);

/*static std::wstring formatNumber(const int value)
{
  wchar_t valueString[100];
  swprintf(valueString,100,L"%d",value);
  return std::wstring(valueString);
}
static std::wstring formatNumber(const uint32_t value)
{
  wchar_t valueString[100];
  swprintf(valueString,100,L"%u",value);
  return std::wstring(valueString);
}*/
#if 0
static char* getURIFromURIWithFragmentID(const char* uri);
#endif
//static char* wstring2string(const wchar_t* str);
static wchar_t* string2wstring(const char* str);

CellMLModelDefinition::CellMLModelDefinition()
{
  mCompileCommand = "gcc -fPIC -O3 -shared -x c -o";
  mTmpDirExists = false;
  mCodeFileExists = false;
  mDsoFileExists = false;
  mSaveTempFiles = false;
  mInstantiated = false;
  nBound = -1;
  nRates = -1;
  nAlgebraic = -1;
  nConstants = -1;
  mNumberOfWantedVariables = 0;
  mNumberOfKnownVariables = 0;
  mModel = NULL;
  mCodeInformation = NULL;
  mAnnotations = NULL;
}

CellMLModelDefinition::CellMLModelDefinition(const char* url) :
		mURL(url)
{
  mCompileCommand = "gcc -fPIC -O3 -shared -x c -o";
  mTmpDirExists = false;
  mCodeFileExists = false;
  mDsoFileExists = false;
  mSaveTempFiles = false;
  mInstantiated = false;
  nBound = -1;
  nRates = -1;
  nAlgebraic = -1;
  nConstants = -1;
  mNumberOfWantedVariables = 0;
  mNumberOfKnownVariables = 0;
  mModel = NULL;
  mCodeInformation = NULL;
  mAnnotations = NULL;
  std::cout << "Creating CellMLModelDefinition from the URL: " 
	    << url << std::endl;
  if (! mURL.empty())
  {
    //std::cout << "Have a valid simulation description." << std::endl;
    //std::cout << "  CellML model URI: " << mURL.c_str() << std::endl;
    RETURN_INTO_WSTRING(URL,string2wstring(mURL.c_str()));
    RETURN_INTO_OBJREF(cb,iface::cellml_api::CellMLBootstrap,
      CreateCellMLBootstrap());
    RETURN_INTO_OBJREF(ml,iface::cellml_api::ModelLoader,cb->modelLoader());
    iface::cellml_api::Model* model = (iface::cellml_api::Model*)NULL;
    try
    {
      model = ml->loadFromURL(URL.c_str());
      model->fullyInstantiateImports();
      mModel = static_cast<void*>(model);
      // create the annotation set
      RETURN_INTO_OBJREF(ats,iface::cellml_services::AnnotationToolService,CreateAnnotationToolService());
      iface::cellml_services::AnnotationSet* as = ats->createAnnotationSet();
      mAnnotations = static_cast<void*>(as);
      // make sure we can generate code and get the initial code information
      RETURN_INTO_OBJREF(cgb,iface::cellml_services::CodeGeneratorBootstrap,
        CreateCodeGeneratorBootstrap());
      RETURN_INTO_OBJREF(cg,iface::cellml_services::CodeGenerator,
        cgb->createCodeGenerator());
      try
      {
        RETURN_INTO_OBJREF(cci,iface::cellml_services::CodeInformation,
          cg->generateCode(model));
        // need to keep a handle on the code information
        cci->add_ref();
        mCodeInformation = static_cast<void*>(cci);
        // and add all state variables as wanted and the variable of integration as known
        RETURN_INTO_OBJREF(cti,iface::cellml_services::ComputationTargetIterator,cci->iterateTargets());
        while(1)
        {
          RETURN_INTO_OBJREF(ct,iface::cellml_services::ComputationTarget,cti->nextComputationTarget());
          if (ct == NULL) break;
          if (ct->type() == iface::cellml_services::STATE_VARIABLE)
          {
            as->setStringAnnotation(ct->variable(),L"flag",L"WANTED");
            mNumberOfWantedVariables++;
          }
          else if (ct->type() == iface::cellml_services::VARIABLE_OF_INTEGRATION)
          {
            as->setStringAnnotation(ct->variable(),L"flag",L"KNOWN");
            mNumberOfKnownVariables++;
          }
        }
      }
      catch (...)
      {
        std::wcerr << L"Error generating the code information for the model" << std::endl;
        mCodeInformation = static_cast<void*>(NULL);
      }
    }
    catch (...)
    {
      std::wcerr << L"Error loading model URL: " << URL.c_str() << std::endl;
      mModel = static_cast<void*>(NULL);
    }
  }
}

CellMLModelDefinition::~CellMLModelDefinition()
{
  if (mAnnotations)
  {
    iface::cellml_services::AnnotationSet* as = static_cast<iface::cellml_services::AnnotationSet*>(mAnnotations);
    as->release_ref();
  }
  if (mCodeInformation)
  {
    iface::cellml_services::CodeInformation* cci = static_cast<iface::cellml_services::CodeInformation*>(mCodeInformation);
    cci->release_ref();
  }
  if (mModel)
  {
    iface::cellml_api::Model* model = static_cast<iface::cellml_api::Model*>(mModel);
    model->release_ref();
  }
  // delete temporary files
  if (mSaveTempFiles)
  {
    std::cout << "At users request, leaving generated files for model: "
	      << mURL.c_str() << std::endl;
  }
  if (mCodeFileExists)
  {
    if (!mSaveTempFiles) unlink(mCodeFileName.c_str());
    else std::cout << "Leaving generated code file: "
		   << mCodeFileName.c_str() << std::endl;
  }
  if (mDsoFileExists)
  {
    if (!mSaveTempFiles) unlink(mDsoFileName.c_str());
    else std::cout << "Leaving generated shared object file: "
		   << mDsoFileName.c_str() << std::endl;
  }
  if (mTmpDirExists)
  {
    if (!mSaveTempFiles) rmdir(mTmpDirName.c_str());
    else std::cout << "Leaving generated temporary directory: "
		   << mTmpDirName.c_str() << std::endl;
  }
}

static int flagVariable(CellMLModelDefinition* d,const char* name, const wchar_t* typeString,
    std::vector<iface::cellml_services::VariableEvaluationType> vets)
{
  if (! d->mCodeInformation)
  {
    std::cerr << "CellMLModelDefinition::flagVariable -- missing model?" << std::endl;
    return -1;
  }
  CVpair cv = splitName(name);
  //std::cout << "setting variable '" << cv.second.c_str() << "' from component '" << cv.first.c_str() << "' as KNOWN" << std::endl;
  iface::cellml_api::Model* model = static_cast<iface::cellml_api::Model*>(d->mModel);
  iface::cellml_services::CodeInformation* cci= static_cast<iface::cellml_services::CodeInformation*>(d->mCodeInformation);
  iface::cellml_services::AnnotationSet* as = static_cast<iface::cellml_services::AnnotationSet*>(d->mAnnotations);
  // find named variable - in local components only!
  RETURN_INTO_OBJREF(components,iface::cellml_api::CellMLComponentSet,model->localComponents());
  RETURN_INTO_WSTRING(cname,string2wstring(cv.first.c_str()));
  RETURN_INTO_OBJREF(component,iface::cellml_api::CellMLComponent,components->getComponent(cname.c_str()));
  if (!component)
  {
    std::cerr << "CellMLModelDefinition::flagVariable -- unable to find component: " << cv.first.c_str() << std::endl;
    return -2;
  }
  RETURN_INTO_OBJREF(variables,iface::cellml_api::CellMLVariableSet,component->variables());
  RETURN_INTO_WSTRING(vname,string2wstring(cv.second.c_str()));
  RETURN_INTO_OBJREF(variable,iface::cellml_api::CellMLVariable,variables->getVariable(vname.c_str()));
  if (!variable)
  {
    std::cerr << "CellMLModelDefinition::flagVariable -- unable to find variable: " << cv.first.c_str() << " / "
        << cv.second.c_str() << std::endl;
    return -3;
  }
  // get source variable
  RETURN_INTO_OBJREF(sv,iface::cellml_api::CellMLVariable,variable->sourceVariable());
  if (!sv)
  {
    std::cerr << "CellMLModelDefinition::flagVariable -- unable get source variable for variable: "
        << cv.first.c_str() << " / " << cv.second.c_str() << std::endl;
    return -4;
  }
  // check if source is already marked as known - what to do? safe to continue with no error
  RETURN_INTO_WSTRING(currentAnnotation,as->getStringAnnotation(sv,L"flag"));
  if (!currentAnnotation.empty())
  {
    if (currentAnnotation == typeString)
    {
      std::cout << "Already flagged same type, nothing to do." << std::endl;
      return 0;
    }
    else
    {
      std::cerr << "CellMLModelDefinition::flagVariacompname, cv->componentName()ble -- variable already flagged something else: "
          << cv.first.c_str() << " / " << cv.second.c_str() << std::endl;
      return -5;
    }
  }
  // find corresponding computation target
  RETURN_INTO_OBJREF(cti,iface::cellml_services::ComputationTargetIterator,cci->iterateTargets());
  iface::cellml_services::ComputationTarget* ct = NULL;
  while(1)
  {
    ct = cti->nextComputationTarget();
    if (ct == NULL) break;
    if (ct->variable() == sv)
    {
//      std::cout << "found a computation target for the source variable of the variable: "
//          << cv.first.c_str() << " / " << cv.second.c_str() << std::endl;
      break;
    }
    else
    {
      ct->release_ref();
      ct = NULL;
    }
  }
  if (!ct)
  {
    std::cerr << "CellMLModelDefinition::flagVariable -- unable get computation target for the source of variable: "
        << cv.first.c_str() << " / " << cv.second.c_str() << std::endl;
    return -5;
  }

  // check type of computation target and make sure compatible
  unsigned int i,compatible = 0;
  for (i=0;i<vets.size();i++)
  {
    if (ct->type() == vets[i])
    {
      compatible = 1;
      break;
    }
  }
  if (compatible)
  {
    as->setStringAnnotation(sv,L"flag",typeString);
  }
  else
  {
    std::cerr << "CellMLModelDefinition::flagVariable -- computation target for variable: "
        << cv.first.c_str() << " / " << cv.second.c_str() << "; is the wrong type to be flagged" << std::endl;
    ct->release_ref();
    return -5;
  }
  ct->release_ref();
  return 0;
}

int CellMLModelDefinition::setVariableAsKnown(const char* name)
{
  std::vector<iface::cellml_services::VariableEvaluationType> vets;
  vets.push_back(iface::cellml_services::CONSTANT);
  vets.push_back(iface::cellml_services::VARIABLE_OF_INTEGRATION);
  vets.push_back(iface::cellml_services::FLOATING);
  int code = flagVariable(this,name,L"KNOWN",vets);
  if (code == 0) mNumberOfKnownVariables++;
  return code;
}

int CellMLModelDefinition::setVariableAsWanted(const char* name)
{
  std::vector<iface::cellml_services::VariableEvaluationType> vets;
  vets.push_back(iface::cellml_services::STATE_VARIABLE);
  vets.push_back(iface::cellml_services::PSEUDOSTATE_VARIABLE);
  vets.push_back(iface::cellml_services::ALGEBRAIC);
  vets.push_back(iface::cellml_services::LOCALLY_BOUND);
  int code = flagVariable(this,name,L"WANTED",vets);
  if (code == 0) mNumberOfWantedVariables++;
  return code;
}

int CellMLModelDefinition::instantiate()
{
  int code = -1;
  std::wstring codeString = getModelAsCCode(mModel,mAnnotations);
  if (codeString.length() > 1)
  {
#ifdef DYNAMIC_COMPILE
    /* We have code, so dump it out to a temporary file in a temporary
       directory so we can have the compiled object nice and handy to
       delete */
    char templ[64] = "tmp.cellml2code.XXXXXX";
    if (mkdtemp(templ))
    {
      // \todo should check for an error...
    }
    mTmpDirName = templ;
    mTmpDirExists = true;
    sprintf(templ,"%s/cellml2code.XXXXXX",mTmpDirName.c_str());
    int tmpFileDes = mkstemp(templ);
    mCodeFileName = templ;
    mCodeFileExists = true;
    FILE* cFile = fdopen(tmpFileDes,"w");
    fprintf(cFile,"%S",codeString.c_str());
    fclose(cFile);
    char* dso = (char*)malloc(mCodeFileName.length()+10);
    /* need to make the dso name right so that it'll load */
    sprintf(dso,"%s%s.so",(templ[0]=='/'?"":"./"),mCodeFileName.c_str());
    mDsoFileName = dso;
    free(dso);
    /* compile the code into a shared object */
    char* compileCommand =
        (char*)malloc(mCompileCommand.length()+mDsoFileName.length()+
            mCodeFileName.length()+3);
    sprintf(compileCommand,"%s %s %s",mCompileCommand.c_str(),
        mDsoFileName.c_str(),mCodeFileName.c_str());
    std::cout << "Compile command: \"" << compileCommand << "\"" << std::endl;
    if (system(compileCommand) == 0)
    {
      mDsoFileExists = true;
      // now load the DSO back into memory and get the required functions
      mHandle = dlopen(mDsoFileName.c_str(),RTLD_LOCAL|RTLD_LAZY);
      if (mHandle)
      {
        /* find the required methods */
        SetupFixedConstants = (void (*)(double*,double*,double*))
	      dlsym(mHandle,"SetupFixedConstants");
        if (SetupFixedConstants == NULL) fprintf(stderr,
            "Error getting method: SetupFixedConstants\n");
        ComputeRates = (void (*)(double,double*,double*,double*,double*))
	      dlsym(mHandle,"ComputeRates");
        if (ComputeRates == NULL) fprintf(stderr,
            "Error getting method: ComputeRates\n");
        EvaluateVariables = (void (*)(double,double*,double*,double*,double*))
	      dlsym(mHandle,"EvaluateVariables");
        if (EvaluateVariables == NULL) fprintf(stderr,
            "Error getting method: EvaluateVariables\n");
        if (SetupFixedConstants && ComputeRates && EvaluateVariables)
        {
          mInstantiated = true;
          code = 0;
        }
        else
        {
          std::cerr << "Error getting one or more of the required methods."
              << std::endl;
          code = -6;
        }
      }
      else
      {
        fprintf(stderr,"Error opening shared object (%s): %s\n",
            mDsoFileName.c_str(),dlerror());
        code = -5;
      }
    }
    else
    {
      std::cerr << "Error compiling the code into a shared object" << std::endl;
      code = -4;
    }
#else
    std::cerr << codeString.c_str() << std::endl;
    code = 0;
#endif
  }
  else
  {
    std::wcerr << L"Error getting C-code for model." << std::endl;
    code = -3;
  }
  return code;
}

/*
 * Local methods
 */
static CVpair splitName(const char* s)
{
  CVpair p;
  p.first = std::string();
  p.second = std::string();
  if (s)
  {
    const char* separator = strchr(s,'/');
    if (separator)
    {
      int l1 = strlen(s) - strlen(separator);
      p.first = std::string(s,l1);
      p.second = std::string(++separator);
    }
  }
  return p;
}

#if 0
// Bit of a hack, but will do the job until the full URI functionality in the current trunk CellML API makes it into a release - or could look at using the xmlURIPtr that comes with libxml2...
static char* getURIFromURIWithFragmentID(const char* uri)
{
  char* u = (char*)NULL;
  if (uri)
  {
    const char* hash = strchr(uri,'#');
    if (hash)
    {
      int l = strlen(uri) - strlen(hash);
      u = (char*)malloc(l+1);
      strncpy(u,uri,l);
      u[l] = '\0';
    }
    else
    {
      /* assume no fragment and return copy of original string */
      u = (char*)malloc(strlen(uri)+1);
      strcpy(u,uri);
    }
  }
  return(u);
}
#endif

/*char* wstring2string(const wchar_t* str)
{
  if (str)
  {
    size_t len = wcsrtombs(NULL,&str,0,NULL);
    if (len > 0)
    {
      len++;
      char* s = (char*)malloc(len);
      wcsrtombs(s,&str,len,NULL);
      return(s);
    }
  }
  return((char*)NULL);
}
*/

wchar_t* string2wstring(const char* str)
{
  if (str)
  {
    wchar_t* s;
    size_t l = strlen(str);
    s = (wchar_t*)malloc(sizeof(wchar_t)*(l+1));
    memset(s,0,(l+1)*sizeof(wchar_t));
    mbsrtowcs(s,&str,l,NULL);
    return(s);
  }
  return((wchar_t*)NULL);
}

std::wstring 
CellMLModelDefinition::getModelAsCCode(void* _model,void* _annotations)
{
  iface::cellml_api::Model* model = static_cast<iface::cellml_api::Model*>(_model);
  iface::cellml_services::AnnotationSet* as = static_cast<iface::cellml_services::AnnotationSet*>(_annotations);
  std::wstring code;
  RETURN_INTO_OBJREF(cgb,iface::cellml_services::CodeGeneratorBootstrap,
    CreateCodeGeneratorBootstrap());
  RETURN_INTO_OBJREF(cg,iface::cellml_services::CodeGenerator,
    cgb->createCodeGenerator());
  RETURN_INTO_OBJREF(ccg, iface::cellml_services::CustomGenerator,
      cg->createCustomGenerator(model));
  RETURN_INTO_OBJREF(cti, iface::cellml_services::ComputationTargetIterator,
      ccg->iterateTargets());
  while (true)
  {
    RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget,
        cti->nextComputationTarget());
    if (ct == NULL) break;
    RETURN_INTO_WSTRING(flag,as->getStringAnnotation(ct->variable(),L"flag"));
    if (flag == L"WANTED")
    {
      ccg->requestComputation(ct);
    }
    else if (flag == L"KNOWN")
    {
      ccg->markAsKnown(ct);
    }
  }
  /* The trunk MaLaES has been updated since the 1.5 release, so define a
   * "custom" MaLaES here
   */
  RETURN_INTO_OBJREF(mbs,iface::cellml_services::MaLaESBootstrap,
    CreateMaLaESBootstrap());
  RETURN_INTO_OBJREF(mt,iface::cellml_services::MaLaESTransform,
    mbs->compileTransformer(
      L"opengroup: (\r\n"
      L"closegroup: )\r\n"
      L"abs: #prec[H]fabs(#expr1)\r\n"
      L"and: #prec[20]#exprs[&&]\r\n"
      L"arccos: #prec[H]acos(#expr1)\r\n"
      L"arccosh: #prec[H]acosh(#expr1)\r\n"
      L"arccot: #prec[1000(900)]atan(1.0/#expr1)\r\n"
      L"arccoth: #prec[1000(900)]atanh(1.0/#expr1)\r\n"
      L"arccsc: #prec[1000(900)]asin(1/#expr1)\r\n"
      L"arccsch: #prec[1000(900)]asinh(1/#expr1)\r\n"
      L"arcsec: #prec[1000(900)]acos(1/#expr1)\r\n"
      L"arcsech: #prec[1000(900)]acosh(1/#expr1)\r\n"
      L"arcsin: #prec[H]asin(#expr1)\r\n"
      L"arcsinh: #prec[H]asinh(#expr1)\r\n"
      L"arctan: #prec[H]atan(#expr1)\r\n"
      L"arctanh: #prec[H]atanh(#expr1)\r\n"
      L"ceiling: #prec[H]ceil(#expr1)\r\n"
      L"cos: #prec[H]cos(#expr1)\r\n"
      L"cosh: #prec[H]cosh(#expr1)\r\n"
      L"cot: #prec[900(0)]1.0/tan(#expr1)\r\n"
      L"coth: #prec[900(0)]1.0/tanh(#expr1)\r\n"
      L"csc: #prec[900(0)]1.0/sin(#expr1)\r\n"
      L"csch: #prec[900(0)]1.0/sinh(#expr1)\r\n"
      L"diff: #lookupDiffVariable\r\n"
      L"divide: #prec[900]#expr1/#expr2\r\n"
      L"eq: #prec[30]#exprs[==]\r\n"
      L"exp: #prec[H]exp(#expr1)\r\n"
      L"factorial: #prec[H]factorial(#expr1)\r\n"
      L"factorof: #prec[30(900)]#expr1 % #expr2 == 0\r\n"
      L"floor: #prec[H]floor(#expr1)\r\n"
      L"gcd: #prec[H]gcd_multi(#count, #exprs[, ])\r\n"
      L"geq: #prec[30]#exprs[>=]\r\n"
      L"gt: #prec[30]#exprs[>]\r\n"
      L"implies: #prec[10(950)] !#expr1 || #expr2\r\n"
      L"int: #prec[H]defint(func#unique1, BOUND, CONSTANTS, RATES, VARIABLES, "
      L"#bvarIndex, pret)#supplement double func#unique1(double* BOUND, "
      L"double* CONSTANTS, double* RATES, double* VARIABLES, int* pret) { return #expr1; }\r\n"
      L"lcm: #prec[H]lcm_multi(#count, #exprs[, ])\r\n"
      L"leq: #prec[30]#exprs[<=]\r\n"
      L"ln: #prec[H]log(#expr1)\r\n"
      L"log: #prec[H]arbitrary_log(#expr1, #logbase)\r\n"
      L"lt: #prec[30]#exprs[<]\r\n"
      L"max: #prec[H]multi_max(#count, #exprs[, ])\r\n"
      L"min: #prec[H]multi_min(#count, #exprs[, ])\r\n"
      L"minus: #prec[500]#expr1 - #expr2\r\n"
      L"neq: #prec[30]#expr1 != #expr2\r\n"
      L"not: #prec[950]!#expr1\r\n"
      L"or: #prec[10]#exprs[||]\r\n"
      L"plus: #prec[500]#exprs[+]\r\n"
      L"power: #prec[H]pow(#expr1, #expr2)\r\n"
      L"quotient: #prec[1000(0)] (double)(((int)#expr2) == 0 ? #expr1 / 0.0 : (int)(#expr1) / (int)(#expr2))\r\n"
      L"rem: #prec[1000(0)] (double)(((int)#expr2) == 0 ? (#expr1) / 0.0 : (int)(#expr1) % (int)(#expr2))\r\n"
      L"root: #prec[1000(900)] pow(#expr1, 1.0 / #degree)\r\n"
      L"sec: #prec[900(0)]1.0 / cos(#expr1)\r\n"
      L"sech: #prec[900(0)]1.0 / cosh(#expr1)\r\n"
      L"sin: #prec[H] sin(#expr1)\r\n"
      L"sinh: #prec[H] sinh(#expr1)\r\n"
      L"tan: #prec[H] tan(#expr1)\r\n"
      L"tanh: #prec[H] tanh(#expr1)\r\n"
      L"times: #prec[900] #exprs[*]\r\n"
      L"unary_minus: #prec[950]- #expr1\r\n"
      L"units_conversion: #prec[500(900)]#expr1*#expr2 + #expr3\r\n"
      L"units_conversion_factor: #prec[900]#expr1*#expr2\r\n"
      L"units_conversion_offset: #prec[500]#expr1+#expr2\r\n"
      L"xor: #prec[25(30)] (#expr1 != 0) ^ (#expr2 != 0)\r\n"
      L"piecewise_first_case: #prec[1000(5)](#expr1 ? #expr2 : \r\n"
      L"piecewise_extra_case: #prec[1000(5)]#expr1 ? #expr2 : \r\n"
      L"piecewise_otherwise: #prec[1000(5)]#expr1)\r\n"
      L"piecewise_no_otherwise: #prec[1000(5)]0.0/0.0)\r\n"
      L"eulergamma: #prec[999]0.577215664901533\r\n"
      L"exponentiale: #prec[999]2.71828182845905\r\n"
      L"false: #prec[999]0.0\r\n"
      L"infinity: #prec[900]1.0/0.0\r\n"
      L"notanumber: #prec[999]0.0/0.0\r\n"
      L"pi: #prec[999] 3.14159265358979\r\n"
      L"true: #prec[999]1.0\r\n"));
  /* now can use the standard transformation?
  cg->transform(mt);
  */
  try
  {
    RETURN_INTO_OBJREF(cci,iface::cellml_services::CustomCodeInformation,ccg->generateCode());
    printf("Constraint level = ");
    switch (cci->constraintLevel())
    {
    case iface::cellml_services::UNDERCONSTRAINED:
      printf("UNDERCONSTRAINED\n");
      break;
    case iface::cellml_services::UNSUITABLY_CONSTRAINED:
      printf("UNSUITABLY_CONSTRAINED\n");
      break;
    case iface::cellml_services::OVERCONSTRAINED:
      printf("OVERCONSTRAINED\n");
      break;
    case iface::cellml_services::CORRECTLY_CONSTRAINED:
      printf("CORRECTLY_CONSTRAINED\n");
      break;
    default:
      printf("Unkown value\n");
    }
    printf("Index count: %u\n", cci->indexCount());
    cti = already_AddRefd<iface::cellml_services::ComputationTargetIterator>(cci->iterateTargets());
    while (true)
    {
      RETURN_INTO_OBJREF(ct, iface::cellml_services::ComputationTarget,
                         cti->nextComputationTarget());
      if (ct == NULL)
        break;

      RETURN_INTO_OBJREF(cv, iface::cellml_api::CellMLVariable, ct->variable());
      RETURN_INTO_WSTRING(compname, cv->componentName());
      RETURN_INTO_WSTRING(varname, cv->name());
      printf("* Computation target %S/%S:%u:\n", compname.c_str(), varname.c_str(),
             ct->degree());
      printf("  => Type = ");
      switch (ct->type())
      {
      case iface::cellml_services::VARIABLE_OF_INTEGRATION:
        printf("VARIABLE_OF_INTEGRATION - was marked as independent.\n");
        break;
      case iface::cellml_services::CONSTANT:
        printf("CONSTANT - this should not happen!\n");
        break;
      case iface::cellml_services::STATE_VARIABLE:
        printf("STATE_VARIABLE - was requested, and is available.\n");
        break;
      case iface::cellml_services::ALGEBRAIC:
        printf("ALGEBRAIC - is used as an intermediate.\n");
        break;
      case iface::cellml_services::FLOATING:
        printf("FLOATING - unused and not requested.\n");
        break;
      case iface::cellml_services::LOCALLY_BOUND:
        printf("LOCALLY_BOUND - locally bound in expressions only.\n");
        break;
      case iface::cellml_services::PSEUDOSTATE_VARIABLE:
        printf("PSEUDOSTATE_VARIABLE - target was requested, but could "
               "not be computed from the independent variables and model.\n");
        break;
      default:
        printf("Unknown type!\n");
      }
      RETURN_INTO_WSTRING(targname, ct->name());
      printf("  => Name = %S\n", targname.c_str());
      printf("  => Index = %u\n", ct->assignedIndex());
    }
    // To do: Print output from cci->iterateTargets();
    RETURN_INTO_WSTRING(functionsString, cci->functionsString());
    printf("Functions: %S\n", functionsString.c_str());
    RETURN_INTO_WSTRING(codeS, cci->generatedCode());
    printf("Code: %S\n", codeS.c_str());
#if defined (OLD_CODE)
    wchar_t* m = cci->errorMessage();
    if (!wcscmp(m,L""))
    {
      std::cout << "whoo hoo!" << std::endl;
      iface::cellml_services::ModelConstraintLevel mcl = cci->constraintLevel();
      if (mcl == iface::cellml_services::UNDERCONSTRAINED)
      {
        std::cerr << "Model is underconstrained" << std::endl;
      }
      else if (mcl == iface::cellml_services::OVERCONSTRAINED)
      {
        std::cerr << "Model is overconstrained" << std::endl;
      }
      else if (mcl == iface::cellml_services::UNSUITABLY_CONSTRAINED)
      {
        std::cerr << "Model is unsuitably constrained" << std::endl;
      }
      else
      {
        std::cout << "Model is correctly constrained" << std::endl;
        // create the code in the format we know how to handle
        code += L"#include <math.h>\n";
        code += L"#include <stdio.h>\n";
        /* required functions */
        code += L"extern double fabs(double x);\n";
        code += L"extern double acos(double x);\n";
        code += L"extern double acosh(double x);\n";
        code += L"extern double atan(double x);\n";
        code += L"extern double atanh(double x);\n";
        code += L"extern double asin(double x);\n";
        code += L"extern double asinh(double x);\n";
        code += L"extern double acos(double x);\n";
        code += L"extern double acosh(double x);\n";
        code += L"extern double asin(double x);\n";
        code += L"extern double asinh(double x);\n";
        code += L"extern double atan(double x);\n";
        code += L"extern double atanh(double x);\n";
        code += L"extern double ceil(double x);\n";
        code += L"extern double cos(double x);\n";
        code += L"extern double cosh(double x);\n";
        code += L"extern double tan(double x);\n";
        code += L"extern double tanh(double x);\n";
        code += L"extern double sin(double x);\n";
        code += L"extern double sinh(double x);\n";
        code += L"extern double exp(double x);\n";
        code += L"extern double floor(double x);\n";
        code += L"extern double pow(double x, double y);\n";
        code += L"extern double factorial(double x);\n";
        code += L"extern double log(double x);\n";
        code += L"extern double arbitrary_log(double x, double base);\n";
        code += L"extern double gcd_pair(double a, double b);\n";
        code += L"extern double lcm_pair(double a, double b);\n";
        code += L"extern double gcd_multi(unsigned int size, ...);\n";
        code += L"extern double lcm_multi(unsigned int size, ...);\n";
        code += L"extern double multi_min(unsigned int size, ...);\n";
        code += L"extern double multi_max(unsigned int size, ...);\n";
        code += L"extern void NR_MINIMISE(double(*func)"
          L"(double VOI, double *C, double *R, double *S, double *A),"
          L"double VOI, double *C, double *R, double *S, double *A, "
          L"double *V);\n";
        wchar_t* frag = cci->functionsString();
        code += frag;
        free(frag);

        nBound = 1;
        nRates = cci->rateIndexCount();
        nAlgebraic = cci->algebraicIndexCount();
          nConstants = cci->constantIndexCount();

        // start the model code...
        /* https://svn.physiomeproject.org/svn/physiome/CellML_DOM_API/trunk/interfaces/CCGS.idl for full description */

        /* initConsts - all variables which aren't state variables but have
         *              an initial_value attribute, and any variables & rates
         *              which follow.
         */
        frag = cci->initConstsString();
        code += L"void SetupFixedConstants(double* CONSTANTS,double* RATES,"
          L"double* STATES)\n{\n";
        code += frag;
        code += L"}\n";
        free(frag);

        /* rates      - All rates which are not static.
         */
        frag = cci->ratesString();
        code += L"void ComputeRates(double VOI,double* STATES,double* RATES,"
          L"double* CONSTANTS,double* ALGEBRAIC)\n{\n";
        code += frag;
        code += L"}\n";
        free(frag);

        /* variables  - All variables not computed by initConsts or rates
         *  (i.e., these are not required for the integration of the model and
         *   thus only need to be called for output or presentation or similar
         *   purposes)
         */
        frag = cci->variablesString();
        code += L"void EvaluateVariables(double VOI,double* CONSTANTS,"
          L"double* RATES, double* STATES, double* ALGEBRAIC)\n{\n";
        code += frag;
        code += L"}\n";
        free(frag);
      }
    }
    else
    {
      std::wcerr << "Error generating code: " << m << std::endl;
    }
    free(m);
#endif
  }
  catch (...)
  {
    std::wcerr << L"Error generating the code information for model"
	       << std::endl;
  }
  return code;
}
