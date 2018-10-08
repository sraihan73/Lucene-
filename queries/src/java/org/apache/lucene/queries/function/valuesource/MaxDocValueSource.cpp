using namespace std;

#include "MaxDocValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

wstring MaxDocValueSource::name() { return L"maxdoc"; }

wstring MaxDocValueSource::description() { return name() + L"()"; }

void MaxDocValueSource::createWeight(
    unordered_map context,
    shared_ptr<IndexSearcher> searcher) 
{
  context.emplace(L"searcher", searcher);
}

shared_ptr<FunctionValues> MaxDocValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  shared_ptr<IndexSearcher> searcher =
      std::static_pointer_cast<IndexSearcher>(context[L"searcher"]);
  return make_shared<ConstIntDocValues>(searcher->getIndexReader()->maxDoc(),
                                        shared_from_this());
}

bool MaxDocValueSource::equals(any o) { return this->getClass() == o.type(); }

int MaxDocValueSource::hashCode() { return this->getClass().hashCode(); }
} // namespace org::apache::lucene::queries::function::valuesource