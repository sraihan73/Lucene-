using namespace std;

#include "BitSetTermsEnum.h"
#include "../../index/PostingsEnum.h"
#include "../../util/BitSet.h"
#include "../../util/BytesRef.h"
#include "BitSetPostingsEnum.h"

namespace org::apache::lucene::codecs::blocktree
{
using PostingsWriterBase = org::apache::lucene::codecs::PostingsWriterBase;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BitSet = org::apache::lucene::util::BitSet;
using BytesRef = org::apache::lucene::util::BytesRef;

BitSetTermsEnum::BitSetTermsEnum(shared_ptr<BitSet> docs)
    : postingsEnum(make_shared<BitSetPostingsEnum>(docs))
{
}

SeekStatus BitSetTermsEnum::seekCeil(shared_ptr<BytesRef> text)
{
  throw make_shared<UnsupportedOperationException>();
}

void BitSetTermsEnum::seekExact(int64_t ord)
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef> BitSetTermsEnum::term()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<BytesRef> BitSetTermsEnum::next()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t BitSetTermsEnum::ord()
{
  throw make_shared<UnsupportedOperationException>();
}

int BitSetTermsEnum::docFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t BitSetTermsEnum::totalTermFreq()
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PostingsEnum>
BitSetTermsEnum::postings(shared_ptr<PostingsEnum> reuse, int flags)
{
  if (flags != PostingsEnum::NONE) {
    // We only work with DOCS_ONLY fields
    return nullptr;
  }
  postingsEnum->reset();
  return postingsEnum;
}
} // namespace org::apache::lucene::codecs::blocktree