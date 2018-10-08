using namespace std;

#include "GermanStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "GermanStemmer.h"

namespace org::apache::lucene::analysis::de
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

GermanStemFilter::GermanStemFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool GermanStemFilter::incrementToken() 
{
  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring term = termAtt->toString();

    if (!keywordAttr->isKeyword()) {
      wstring s = stemmer->stem(term);
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

void GermanStemFilter::setStemmer(shared_ptr<GermanStemmer> stemmer)
{
  if (stemmer != nullptr) {
    this->stemmer = stemmer;
  }
}
} // namespace org::apache::lucene::analysis::de