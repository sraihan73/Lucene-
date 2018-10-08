using namespace std;

#include "LowerCaseFilter.h"
#include "CharacterUtils.h"
#include "TokenStream.h"
#include "tokenattributes/CharTermAttribute.h"

namespace org::apache::lucene::analysis
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;

LowerCaseFilter::LowerCaseFilter(shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool LowerCaseFilter::incrementToken() 
{
  if (input->incrementToken()) {
    CharacterUtils::toLowerCase(termAtt->buffer(), 0, termAtt->length());
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis