using namespace std;

#include "SimpleFragmenter.h"

namespace org::apache::lucene::search::highlight
{
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;

SimpleFragmenter::SimpleFragmenter() : SimpleFragmenter(DEFAULT_FRAGMENT_SIZE)
{
}

SimpleFragmenter::SimpleFragmenter(int fragmentSize)
{
  this->fragmentSize = fragmentSize;
}

void SimpleFragmenter::start(const wstring &originalText,
                             shared_ptr<TokenStream> stream)
{
  offsetAtt = stream->addAttribute(OffsetAttribute::typeid);
  currentNumFrags = 1;
}

bool SimpleFragmenter::isNewFragment()
{
  bool isNewFrag = offsetAtt->endOffset() >= (fragmentSize * currentNumFrags);
  if (isNewFrag) {
    currentNumFrags++;
  }
  return isNewFrag;
}

int SimpleFragmenter::getFragmentSize() { return fragmentSize; }

void SimpleFragmenter::setFragmentSize(int size) { fragmentSize = size; }
} // namespace org::apache::lucene::search::highlight