using namespace std;

#include "PostingsEnum.h"

namespace org::apache::lucene::index
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using BytesRef = org::apache::lucene::util::BytesRef;

bool PostingsEnum::featureRequested(int flags, short feature)
{
  return (flags & feature) == feature;
}

PostingsEnum::PostingsEnum() {}

shared_ptr<AttributeSource> PostingsEnum::attributes()
{
  if (atts == nullptr) {
    atts = make_shared<AttributeSource>();
  }
  return atts;
}
} // namespace org::apache::lucene::index