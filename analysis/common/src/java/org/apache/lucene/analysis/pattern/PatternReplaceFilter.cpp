using namespace std;

#include "PatternReplaceFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::pattern
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

PatternReplaceFilter::PatternReplaceFilter(shared_ptr<TokenStream> in_,
                                           shared_ptr<Pattern> p,
                                           const wstring &replacement, bool all)
    : org::apache::lucene::analysis::TokenFilter(in_),
      replacement((L"" == replacement) ? L"" : replacement),
      m(p->matcher(termAtt))
{
  this->all = all;
}

bool PatternReplaceFilter::incrementToken() 
{
  if (!input->incrementToken()) {
    return false;
  }

  m->reset();
  if (m->find()) {
    // replaceAll/replaceFirst will reset() this previous find.
    wstring transformed =
        all ? m->replaceAll(replacement) : m->replaceFirst(replacement);
    termAtt->setEmpty()->append(transformed);
  }

  return true;
}
} // namespace org::apache::lucene::analysis::pattern