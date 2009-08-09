#ifndef _RDFGRAPH_H_
#define _RDFGRAPH_H_

#include <redland.h>

class RDFGraph;

/* User function prototype called in iterate methods */
typedef int (*QueryIterFunc)(RDFGraph*,void**,int,void*);

/**
 * A basic wrapper providing the RDF functionality required for openCMISS.
 */
class RDFGraph
{
 public:
  RDFGraph();
  ~RDFGraph();

  void print(FILE* file);
  /**
   * Build a RDF graph from a given source URL.
   * Create a RDF graph from the given source RDF/XML document. Expects a 
   * straight RDF/XML document only.
   * @param url The URL of the source document.
   * @return 0 on success, non-zero for error.
   */
  int buildFromURL(const char* url);
  /**
   * Attempt to find a simulation description it this graph.
   * Will look for the first simulation description found in this RDF graph and craete the corresponding simulation description.
   * @return The created simulation description, if found; NULL otherwise.
   */
  class SimulationDescription* createSimulationDescription();
  /**
   * Check if the given graph node is a resource node.
   * @param node Node pointer to check.
   * @return true if node is a resource node; false otherwise.
   */
  bool nodeIsResource(void* node);
  /**
   * Get the URI of the given resource node.
   * @return The URI of the given node if possible; NULL otherwise.
   */
  char* nodeGetURI(void* node);

 private:
  librdf_world* world;
  librdf_storage* storage;
  librdf_model* rdfmodel;
  librdf_parser* parser;
  librdf_uri* uri;
  int iterateQueryResults(const char* query,const std::vector<std::string>& names,
    QueryIterFunc func,void* user_data);
};

#endif // _RDFGRAPH_H_
