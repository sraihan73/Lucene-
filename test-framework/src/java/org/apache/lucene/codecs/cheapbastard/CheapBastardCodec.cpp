using namespace std;

#include "CheapBastardCodec.h"

namespace org::apache::lucene::codecs::cheapbastard
{
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using TestUtil = org::apache::lucene::util::TestUtil;

CheapBastardCodec::CheapBastardCodec()
    : org::apache::lucene::codecs::FilterCodec(L"CheapBastard",
                                               TestUtil::getDefaultCodec())
{
}

shared_ptr<PostingsFormat> CheapBastardCodec::postingsFormat()
{
  return postings;
}
} // namespace org::apache::lucene::codecs::cheapbastard