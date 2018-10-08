using namespace std;

#include "SimpleHTMLFormatter.h"

namespace org::apache::lucene::search::highlight
{

const wstring SimpleHTMLFormatter::DEFAULT_PRE_TAG = L"<B>";
const wstring SimpleHTMLFormatter::DEFAULT_POST_TAG = L"</B>";

SimpleHTMLFormatter::SimpleHTMLFormatter(const wstring &preTag,
                                         const wstring &postTag)
{
  this->preTag = preTag;
  this->postTag = postTag;
}

SimpleHTMLFormatter::SimpleHTMLFormatter()
    : SimpleHTMLFormatter(DEFAULT_PRE_TAG, DEFAULT_POST_TAG)
{
}

wstring SimpleHTMLFormatter::highlightTerm(const wstring &originalText,
                                           shared_ptr<TokenGroup> tokenGroup)
{
  if (tokenGroup->getTotalScore() <= 0) {
    return originalText;
  }

  // Allocate StringBuilder with the right number of characters from the
  // beginning, to avoid char[] allocations in the middle of appends.
  shared_ptr<StringBuilder> returnBuffer = make_shared<StringBuilder>(
      preTag.length() + originalText.length() + postTag.length());
  returnBuffer->append(preTag);
  returnBuffer->append(originalText);
  returnBuffer->append(postTag);
  return returnBuffer->toString();
}
} // namespace org::apache::lucene::search::highlight