using namespace std;

#include "SinglePostingsEnum.h"

namespace org::apache::lucene::codecs::idversion
{
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

SinglePostingsEnum::SinglePostingsEnum() : payload(make_shared<BytesRef>(8))
{
  payload->length = 8;
}

void SinglePostingsEnum::reset(int singleDocID, int64_t version)
{
  doc = -1;
  this->singleDocID = singleDocID;
  this->version = version;
}

int SinglePostingsEnum::nextDoc()
{
  if (doc == -1) {
    doc = singleDocID;
  } else {
    doc = NO_MORE_DOCS;
  }
  pos = -1;

  return doc;
}

int SinglePostingsEnum::docID() { return doc; }

int SinglePostingsEnum::advance(int target)
{
  if (doc == -1 && target <= singleDocID) {
    doc = singleDocID;
    pos = -1;
  } else {
    doc = NO_MORE_DOCS;
  }
  return doc;
}

int64_t SinglePostingsEnum::cost() { return 1; }

int SinglePostingsEnum::freq() { return 1; }

int SinglePostingsEnum::nextPosition()
{
  assert(pos == -1);
  pos = 0;
  IDVersionPostingsFormat::longToBytes(version, payload);
  return pos;
}

shared_ptr<BytesRef> SinglePostingsEnum::getPayload() { return payload; }

int SinglePostingsEnum::startOffset() { return -1; }

int SinglePostingsEnum::endOffset() { return -1; }
} // namespace org::apache::lucene::codecs::idversion