using namespace std;

#include "NumDocsValueSource.h"

namespace org::apache::lucene::queries::function::valuesource
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using ReaderUtil = org::apache::lucene::index::ReaderUtil;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;
using ValueSource = org::apache::lucene::queries::function::ValueSource;

wstring NumDocsValueSource::name() { return L"numdocs"; }

wstring NumDocsValueSource::description() { return name() + L"()"; }

shared_ptr<FunctionValues> NumDocsValueSource::getValues(
    unordered_map context,
    shared_ptr<LeafReaderContext> readerContext) 
{
  // Searcher has no numdocs so we must use the reader instead
  return make_shared<ConstIntDocValues>(
      ReaderUtil::getTopLevelContext(readerContext)->reader()->numDocs(),
      shared_from_this());
}

bool NumDocsValueSource::equals(any o) { return this->getClass() == o.type(); }

int NumDocsValueSource::hashCode() { return this->getClass().hashCode(); }
} // namespace org::apache::lucene::queries::function::valuesource