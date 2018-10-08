using namespace std;

#include "ExpressionFunctionValues.h"

namespace org::apache::lucene::expressions
{
using DoubleValues = org::apache::lucene::search::DoubleValues;

ExpressionFunctionValues::ExpressionFunctionValues(
    shared_ptr<Expression> expression,
    std::deque<std::shared_ptr<DoubleValues>> &functionValues)
    : expression(expression), functionValues(functionValues)
{
  if (expression == nullptr) {
    throw make_shared<NullPointerException>();
  }
  if (functionValues.empty()) {
    throw make_shared<NullPointerException>();
  }
}

bool ExpressionFunctionValues::advanceExact(int doc) 
{
  for (auto v : functionValues) {
    v->advanceExact(doc);
  }
  return true;
}

double ExpressionFunctionValues::doubleValue()
{
  return expression->evaluate(functionValues);
}
} // namespace org::apache::lucene::expressions