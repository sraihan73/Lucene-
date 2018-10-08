using namespace std;

#include "SingleDocsEnum.h"

namespace org::apache::lucene::codecs::idversion
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

void SingleDocsEnum::reset(int singleDocID)
{
  doc = -1;
  this->singleDocID = singleDocID;
}

int SingleDocsEnum::nextDoc()
{
  if (doc == -1) {
    doc = singleDocID;
  } else {
    doc = NO_MORE_DOCS;
  }

  return doc;
}

int SingleDocsEnum::docID() { return doc; }

int SingleDocsEnum::advance(int target)
{
  if (doc == -1 && target <= singleDocID) {
    doc = singleDocID;
  } else {
    doc = NO_MORE_DOCS;
  }
  return doc;
}

int64_t SingleDocsEnum::cost() { return 1; }

int SingleDocsEnum::freq() { return 1; }

int SingleDocsEnum::nextPosition()  { return -1; }

int SingleDocsEnum::startOffset()  { return -1; }

int SingleDocsEnum::endOffset()  { return -1; }

shared_ptr<BytesRef> SingleDocsEnum::getPayload() 
{
  throw make_shared<UnsupportedOperationException>();
}
} // namespace org::apache::lucene::codecs::idversion