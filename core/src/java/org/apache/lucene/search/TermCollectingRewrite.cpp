using namespace std;

#include "TermCollectingRewrite.h"

namespace org::apache::lucene::search
{
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexReaderContext = org::apache::lucene::index::IndexReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;

void TermCollectingRewrite<B>::TermCollector::setReaderContext(
    shared_ptr<IndexReaderContext> topReaderContext,
    shared_ptr<LeafReaderContext> readerContext)
{
  this->readerContext = readerContext;
  this->topReaderContext = topReaderContext;
}
} // namespace org::apache::lucene::search