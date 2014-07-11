#ifndef GO_ESCAPE_H
#define GO_ESCAPE_H

#include <vector>
#include <set>
#include <map>

class Gogo;
class Named_object;
class Statement;
class Expression_list;
class Expression;
class Call_expression;
class Field_reference_expression;
class Escape_analysis_info;
class Escape_analysis_object;

// Escape analysis information about Go program

// This module performs escape analysis.

// Run eacape analysis on the minimal set of functions. 
// The mutually recursive functions (which have strong connected components) or
// single non-recursive functions. Bottom up.

// We care about whether the pointer to an in-function variable can escape to 
// the outer scope. We make every pointer cannot be proved not escaped stay 
// on heap.

// Graph build: We construct a graph of which value flows to where
// Walk the graph: flood fill the graph to get the escape information.

class Escape_analysis
{
  public:

    // Supposed move to Escape_analysis_object
    // enum Escape_level
    // {
    //   ESCAPE_UNKNOWN,
    //   ESCAPE_NONE,
    //   ESCAPE_SCOPE,   // escape from the current scope
    //   ESCAPE_HEAP,    // escape to the heap.
    //   ESCAPE_RETURN
    // };

    // Constructor
    Escape_analysis()
      : escape_info_map_()
    { }

    // Destructor
    ~Escape_analysis();

    // Perform the escape analysis
    static void
      perform(Gogo*);

    // Compute function list
    void
      compute_functions_to_process(Gogo*);

    // Add functions to the set of functions to explore.
    void
      add_function(Named_object* no)
      { this->functions_.insert(no); }

    // Compute the analysis results for the current package
    void 
      compute_analysis_results();

  private:
    // Typedef for the escape info map
    typedef std::map<Named_object*, Escape_analysis_info*> Escape_info_map;

    typedef std::set<Named_object*> Named_object_set;

    typedef std::map<const Named_object*, Named_object_set> Caller_map;

    typedef std::vector<Named_object*> Named_object_vector;

    // TODO I'll make it record escape infomation for 
    // function set.
    // Escape analysis info for each function.
    Escape_info_map escape_info_map_;

    // Safe functions.
    // This is the most simple sammry for functions.
    // a safe function does not leak anything pointed by a parameter to the
    // heap. A call to a safe function only needs to alias the actual
    // parameters.
    // TODO Refine if necessary.
    // TODO for debug printing
    Named_object_set safe_functions_;

    // The original set of functions in this package.
    Named_object_set functions_;
};

class Escape_analysis_info
{
  public:
    // Constructor
    Escape_analysis_info(Named_object* no)
      : current_function_(no) { }

    // Destructor
    ~Escape_analysis_info();

    // dig into the functions.
    void
      escape_func(Named_object* no);

    // for statements.
    // TODO parameters
    void
      escape_statement_list();
    void
      escape_statement();

    // TODO parameters 
    void
      escape_expression();

    // pulled out from escape_expression
    // TODO parameters
    void
      escape_expression_call();

    // store the link dst <- src in dst
    // build up edges for the connection graph
    // TODO parameters
    void
      escape_flows();

    // Start from the dst node, when hit a reference we make level goes up by
    // one, and hit a dereference we make it minus one. So if we find the level
    // > 0, it now may not leak.
    // if level == 0, that means we can hit the root of the flood, and if now
    // meets dereference, we can mark the current node leaking if the loopdepth
    // of root is smaller than the current node.
    // Once a object has been moved to heap, all its upstreams should also
    // marked escaped to the global scope.
    // escape_flood() walk from dst nodes and touch every node it can reach
    // TODO parameters
    void
      escape_flood();

    // TODO parameters
    void
      escape_walk(int level);

  private:

    // Prepare for using
    typedef std::map<const Expression*, Escape_analysis_object*> Expression_map;

    typedef std::map<source_location, Escape_analysis_object*>
      Location_object_map;

    typedef std::map<const Temporary_statement*, Escape_analysis_object*>
      Temporary_object_map;

    // Vector of analysis objects
    std::vector<Escape_analysis_object*> objects_;

    // Map from expressions to analysis objects (references)
    Expression_map expression_map_;

    // Maps Named_object to analysis objects
    Named_object_map named_object_map_;

    // Contains allocations, parameter placehoders and phantom objects.
    // We create at most 1 phatom object per expression.
    Expression_map expr_object_map_;

    // Contains allocations, parameter placehoders and phantom objects.
    // We create at most 1 phatom object per expression.
    Named_object_map no_object_map_;

    // Contains temporary references.
    // 1 reference per temporary statement.
    Temporary_object_map temporary_reference_map_;

    // The current function being analysised.
    Named_object* current_function_;

    // Phatom act as an abstract of outer scope.
    // Which return values, variables that assigned to global variables
    // ... all flows to.
    Escape_analysis_Object* phatom;

    // Store the dst node appear in this set of functions.
    // Later will search from here.
    Escape_analysis_Object* dsts;
};

// Representation of an internal/external objects.
// TODO Modify here.....
class Escape_analysis_object
{
  public:

    // Constructor
    // Object_type to mark the recent node type. We abstruct a phantom node to
    // represent the outer.
    Escape_analysis_object(
        unsigned int id,    // for debug
        Escape_analysis_info* escape_info,
        const Named_object* no,
        Expression* expr,
      : escape_status_(ESCAPE_NONE),
        escape_info_(escape_info),
        object_(no),
        expression_(expr) { }

  private:

    enum Escape_status {
      ESCAPE_UNKNOWN,
      ESCAPE_NONE,
      ESCAPE_SCOPE,
      ESCAPE_HEAP
    };

    // Recorde the escape status for each node.
    Escape_status escape_status_;

    Escape_analysis_info escape_info_;

    // For debuging.
    unsigned int object_id_;

    // Named_object it represent if any.
    Named_object* object_;

    // Expression it represent if any.
    Expression* expression_;

    // Store the node it can reach.
    // We store the dst->src edge in dst.
    Object_set out_edges_;
};


#endif
