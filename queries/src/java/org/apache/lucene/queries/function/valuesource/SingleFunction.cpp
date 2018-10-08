using namespace std;

#include "SingleFunction.h"

namespace org::apache::lucene::queries::function::valuesource
{
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

SingleFunction::SingleFunction(shared_ptr<ValueSource> source) : source(source)
{
}

wstring SingleFunction::description()
{
  return name() + StringHelper::toString(L'(') + source->description() +
         StringHelper::toString(L')');
}

int SingleFunction::hashCode()
{
  return source->hashCode() + name().hashCode();
}

bool SingleFunction::equals(any o)
{
  if (this->getClass() != o.type()) {
    return false;
  }
  shared_ptr<SingleFunction> other =
      any_cast<std::shared_ptr<SingleFunction>>(o);
  return this->name() == other->name() && this->source->equals(other->source);
}

void SingleFunction::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  source->createWeight(context, searcher);
}
} // namespace org::apache::lucene::queries::function::valuesource