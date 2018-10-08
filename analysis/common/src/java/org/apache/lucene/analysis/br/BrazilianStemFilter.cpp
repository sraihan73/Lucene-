using namespace std;

#include "BrazilianStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "BrazilianStemmer.h"

namespace org::apache::lucene::analysis::br
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

BrazilianStemFilter::BrazilianStemFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool BrazilianStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    const wstring term = termAtt->toString();
    // Check the exclusion table.
    if (!keywordAttr->isKeyword() &&
        (exclusions == nullptr || !exclusions->contains(term))) {
      const wstring s = stemmer->stem(term);
      // If not stemmed, don't waste the time adjusting the token.
      if ((s != L"") && s != term) {
        termAtt->setEmpty()->append(s);
      }
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::br