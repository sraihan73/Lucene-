using namespace std;

#include "MockLowerCaseFilter.h"

namespace org::apache::lucene::analysis
{
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

MockLowerCaseFilter::MockLowerCaseFilter(shared_ptr<TokenStream> in_)
    : TokenFilter(in_)
{
}

bool MockLowerCaseFilter::incrementToken() 
{
  if (input->incrementToken()) {
    CharacterUtils::toLowerCase(termAtt->buffer(), 0, termAtt->length());
    return true;
  } else {
    return false;
  }
}
} // namespace org::apache::lucene::analysis