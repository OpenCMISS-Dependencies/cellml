#include <string.h>
#include <iostream>
#include <string>
#include <vector>

#include <libxml/xmlreader.h>

#include "occellml_config.h"

#include "SimulationDescription.hpp"
#include "RDFGraph.hpp"
#include "namespaces.h"

/* Only need to have one world at a time */
struct RDFWorld
{
  librdf_world* world;
  int counter;
};
static struct RDFWorld* rdfWorld = (struct RDFWorld*)NULL;
static librdf_world* AccessRDFWorld()
{
  if (rdfWorld) ++(rdfWorld->counter);
  else
  {
    rdfWorld = (struct RDFWorld*)malloc(sizeof(struct RDFWorld));
    rdfWorld->world = librdf_new_world();
    librdf_world_open(rdfWorld->world);
    rdfWorld->counter = 1;
  }
  return rdfWorld->world;
}
static void DeaccessRDFWorld()
{
  if (rdfWorld)
  {
    if (rdfWorld->counter > 1) --(rdfWorld->counter);
    else
    {
      librdf_free_world(rdfWorld->world);
      free(rdfWorld);
      rdfWorld = (struct RDFWorld*)NULL;
    }
  }
}
static int rdfWorldAccessCount()
{
  if (rdfWorld) return rdfWorld->counter;
  else return(0);
}

/* prototype local methods */
static char* getXMLFileContentsAsString(const char* uri);
static int buildSimulationDescription(RDFGraph* graph,void** nodes,int N,
  void* user_data);


RDFGraph::RDFGraph()
{
  this->world = AccessRDFWorld();
  this->storage = (librdf_storage*)NULL;
  this->rdfmodel = (librdf_model*)NULL;
  this->parser = (librdf_parser*)NULL;
  this->uri = (librdf_uri*)NULL;
}

RDFGraph::~RDFGraph()
{
  if (this->storage)  librdf_free_storage(this->storage);
    if (this->rdfmodel) librdf_free_model(this->rdfmodel);
    if (this->parser) librdf_free_parser(this->parser);
    if (this->uri) librdf_free_uri(this->uri);
    DeaccessRDFWorld();
}

void RDFGraph::print(FILE* file)
{
  if (file && this->rdfmodel)
  {
    librdf_model_print(this->rdfmodel,file);
  }
  else std::cerr << "rdfGraphPrint - Invalid argument(s)\n" << std::endl;
}

int RDFGraph::buildFromURL(const char* uri)
{
  int code = -1;
  if (this->world && uri)
  {
    /* do we have any metadata? */
    char* metadata = getXMLFileContentsAsString(uri);
    /* if not, we can quit now */
    if (metadata)
    {
      /* make an in-memory storage to store our RDF triples */
      if (this->storage == NULL)
      {
        char* identifier = (char*)malloc(16);
        sprintf(identifier,"%015d",rdfWorldAccessCount());
        //DEBUG(0,"rdfGraphBuildFromURI","Making new storage with id: %s\n",
        //  identifier);
        this->storage = librdf_new_storage(this->world,"memory",
          identifier,/*options*/NULL);
        free(identifier);
      }
      /* make a new RDF graph */
      if (this->rdfmodel == NULL) this->rdfmodel =
        librdf_new_model(this->world,this->storage,/*options*/NULL);
      /* and a RDF parser */
      if (this->parser == NULL) this->parser =
        librdf_new_parser(this->world,/*parser name*/"rdfxml",
          /*mime type*/NULL,/*syntax URI*/NULL);
      if (this->uri == NULL) this->uri =
        librdf_new_uri(this->world,(const unsigned char*)uri);
      /* PARSE the metadata string as RDF/XML */
      if (librdf_parser_parse_string_into_model(this->parser,
          (unsigned char*)metadata,this->uri,this->rdfmodel) == 0) code = 0;
      free(metadata);
    }
  }
  else std::cerr << "rdfGraphBuildFromURL - Invalid argument(s)" << std::endl;
  return(code);
}

SimulationDescription* RDFGraph::createSimulationDescription()
{
  SimulationDescription* description = new SimulationDescription();
  // create the SPARQL query to find the simulation
  std::string query = "prefix cs: <";
  query += CS_NS;
  query += ">";
  query += " select ?parent ?simulation";
  query += " where {";
  query += "    ?parent cs:simulation ?simulation";
  query += "    filter isURI(?parent)";
  query += " }";
  std::vector<std::string> names(2);
  names[0] = "parent";
  names[1] = "simulation";
  int code = this->iterateQueryResults(query.c_str(),names,
    &buildSimulationDescription,(void*)description);
  if (code != 0)
  {
    std::cerr << "RDFGraph::createSimulationDescription(): "
	      << "error creating simulation description" << std::endl;
    delete description;
    description = (SimulationDescription*)NULL;
  }
  return(description);
}

int RDFGraph::iterateQueryResults(const char* query,
  const std::vector<std::string>& names,QueryIterFunc func,void* user_data)
{
  int code = -1;
  if (this->rdfmodel)
  {
    librdf_query* q = librdf_new_query(this->world,"sparql",NULL,
      (unsigned char*)query,NULL);
    librdf_query_results* results =
      librdf_model_query_execute(this->rdfmodel,q);
    if (results)
    {
      code = 0;
      while(!librdf_query_results_finished(results))
      {
	int N = names.size();
        void** nodes = (void**)malloc(sizeof(void*)*N);
        int i;
        for (i=0;i<N;i++) nodes[i] =
          (void*)librdf_query_results_get_binding_value_by_name(
            results,names[i].c_str());
        if (nodes)
        {
          code = func(this,nodes,N,user_data);
          for (i=0;i<N;i++) librdf_free_node((librdf_node*)nodes[i]);
          free(nodes);
        }
        librdf_query_results_next(results);
        if (code != 0) break;
      }
      librdf_free_query_results(results);
    }
    librdf_free_query(q);
  }
  else
  {
    std::cerr << "iterateQueryResults: missing RDF model" << std::endl;
  }
  return(code);
}

bool RDFGraph::nodeIsResource(void* _node)
{
  int code = 0;
  if (_node)
  {
    librdf_node* node = (librdf_node*)_node;
    code = librdf_node_is_resource(node);
  }
  if (code) return(true);
  return(false);
}

char* RDFGraph::nodeGetURI(void* _node)
{
  char* uri = (char*)NULL;
  if (_node)
  {
    librdf_node* node = (librdf_node*)_node;
    if (librdf_node_is_resource(node)) uri =
      (char*)librdf_uri_to_string(librdf_node_get_uri(node));
  }
  else std::cerr << "rdfGraphGetURI -- Invalid argument(s)" << std::endl;
  return(uri);
}

/*
 * Local methods
 */
/** Print the contents of the given RDF node */
/*static void printNode(void* _node,FILE* file)
{
  if (_node && file)
  {
    librdf_node* node = (librdf_node*)_node;
    librdf_node_print(node,file);
  }
  else std::cerr << "rdfGraphPrint - Invalid argument(s)" << std::endl;
  }*/

/** Get the contents of the given XML file as a standard C string */
static char* getXMLFileContentsAsString(const char* uri)
{
  char* string = (char*)NULL;
  if (uri)
  {
    /* ensure startup libxml */
    xmlInitParser();
    LIBXML_TEST_VERSION;
    /* make an xml text reader for the given URI */
    xmlTextReaderPtr reader = xmlNewTextReaderFilename(uri);
    if (reader != NULL)
    {
      /* parse and preserve the entire document */
      int ret = xmlTextReaderRead(reader);
      xmlTextReaderPreservePattern(reader,BAD_CAST "*",NULL);
      while (ret == 1) ret = xmlTextReaderRead(reader);
      if (ret != 0)
      {
	std::cerr << "getXMLFileContentsAsString" << uri
		  << ": failed to parse" << std::endl;
      }
      else
      {
        /* grab the resulting XML document */
        xmlDocPtr doc = xmlTextReaderCurrentDoc(reader);
        if (doc)
        {
          /* and dump it into the result string */
          xmlChar* s;
          int l;
          /*xmlDocDump(stdout,doc);*/
          xmlDocDumpMemory(doc,&s,&l);
          if (s)
          {
	    string = (char*)malloc(strlen((const char*)s)+1);
	    strcpy(string,(const char*)s);
            /*printf("string: **%s**\n",string);*/
            xmlFree(s);
          }
          else std::cerr << "getXMLFileContentsAsString" << uri
			 << ": failed to dump to string" << std::endl;
          /* free up the XML doc */
          xmlFreeDoc(doc);
        }
      }
      /* free the reader */
      xmlFreeTextReader(reader);
    }
    else
    {
      std::cerr << "getXMLFileContentsAsString - "
		<< "Unable to make the XML reader for the uri: " << uri
		<< std::endl;
    }
    /* Shutdown libxml - don't want to as it may be used elsewhere */
    //xmlCleanupParser();
  }
  else std::cerr << "Invalid args: getXMLFileContentsAsString" << std::endl;
  return(string);
}

/** Callback function for the iterate query results method to build a simulation description.
 * Will only populate the provided simulation description if it is not already a valid description.
 */
static int buildSimulationDescription(RDFGraph* graph,void** nodes,
  int N,void* _ptr)
{
  int code = -1;
  SimulationDescription* description = (SimulationDescription*)_ptr;
  if (description->isValid())
  {
    // already have a valid simulation description, look no further
    code = 0;
    std::cout << "Already have a valid simulation description" << std::endl;
  }
  else
  {
    std::cout << "Setting up simulation description" << std::endl;
    void* parent = nodes[0];
    if (graph->nodeIsResource(parent))
    {
      char* modelURI = graph->nodeGetURI(parent);
      description->modelURI(modelURI);
      free(modelURI);
      code = 0;
    }
  }
  return(code);
}
