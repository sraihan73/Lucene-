using namespace std;

#include "FixBrokenOffsetsFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

FixBrokenOffsetsFilter::FixBrokenOffsetsFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool FixBrokenOffsetsFilter::incrementToken() 
{
  if (input->incrementToken() == false) {
    return false;
  }
  fixOffsets();
  return true;
}

void FixBrokenOffsetsFilter::end() 
{
  TokenFilter::end();
  fixOffsets();
}

void FixBrokenOffsetsFilter::reset() 
{
  TokenFilter::reset();
  lastStartOffset = 0;
  lastEndOffset = 0;
}

void FixBrokenOffsetsFilter::fixOffsets()
{
  int startOffset = offsetAtt->startOffset();
  int endOffset = offsetAtt->endOffset();
  if (startOffset < lastStartOffset) {
    startOffset = lastStartOffset;
  }
  if (endOffset < startOffset) {
    endOffset = startOffset;
  }
  offsetAtt->setOffset(startOffset, endOffset);
  lastStartOffset = startOffset;
  lastEndOffset = endOffset;
}
} // namespace org::apache::lucene::analysis::miscellaneous