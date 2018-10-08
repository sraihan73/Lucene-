using namespace std;

#include "UpperCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharacterUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis::core
{
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

UpperCaseFilter::UpperCaseFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool UpperCaseFilter::incrementToken() 
{
  if (input->incrementToken()) {
    CharacterUtils::toUpperCase(termAtt->buffer(), 0, termAtt->length());
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis::core