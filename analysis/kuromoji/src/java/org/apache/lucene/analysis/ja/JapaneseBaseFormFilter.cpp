using namespace std;

#include "JapaneseBaseFormFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "tokenattributes/BaseFormAttribute.h"

namespace org::apache::lucene::analysis::ja
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseFormAttribute =
    org::apache::lucene::analysis::ja::tokenattributes::BaseFormAttribute;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;

JapaneseBaseFormFilter::JapaneseBaseFormFilter(shared_ptr<TokenStream> input)
    : org::apache::lucene::analysis::TokenFilter(input)
{
}

bool JapaneseBaseFormFilter::incrementToken() 
{
  if (input->incrementToken()) {
    if (!keywordAtt->isKeyword()) {
      wstring baseForm = basicFormAtt->getBaseForm();
      if (baseForm != L"") {
        termAtt->setEmpty()->append(baseForm);
      }
    }
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::ja