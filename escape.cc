#include <stdio.h>

#include "gogo.h"
#include "types.h"
#include "expressions.h"
#include "statements.h"
#include "go-optimize.h"
#include "ast-dump.h"
#include "go-dump.h"
#include "escape.h"

class Escape_analysis;
class Escape_analysis_info;
class Call_graph_traverse_functions;

// Perform stack allocation optimization
void
Gogo::optimize_allocation()
{
  // TODO temporarily use this for debug.
  // Add some flags to determine whether to perform.
  Escape_analysis::perform(this);
}

// TODO;;;;;;
class Call_graph_traverse_functions : public Traverse
{
  public:
    Call_graph_traverse_functions(
        Escape_analysis* escape_analysis_ctx)
      : Traverse(traverse_functions),
      escape_analysis_ctx_(escape_analysis_ctx)
  { }
   
    int
    function(Named_object*);

  private:
    Escape_analysis* escape_analysis_ctx_;
};

int
Call_graph_traverse_functions::function(Named_object* no)
{
  this->escape_analysis_ctx_->add_function(no);
  printf("%s\n", no->name().c_str());

  go_assert(no->is_function());
  // Function* func = no->func_value();

  // TODO
  // Call_graph_traverse_expression

  return TRAVERSE_CONTINUE;
}
void
Escape_analysis::perform(Gogo* gogo)
{
    Escape_analysis escape_analysis;
    escape_analysis.compute_functions_to_process(gogo);
}

void
Escape_analysis::compute_functions_to_process(Gogo* gogo)
{
    Call_graph_traverse_functions cgtf(this);
    gogo->traverse(&cgtf);
}


Escape_analysis::~Escape_analysis() { }
