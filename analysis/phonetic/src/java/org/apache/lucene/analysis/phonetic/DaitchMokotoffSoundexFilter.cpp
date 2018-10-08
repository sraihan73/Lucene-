using namespace std;

#include "DaitchMokotoffSoundexFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::language::DaitchMokotoffSoundex;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
const shared_ptr<java::util::regex::Pattern>
    DaitchMokotoffSoundexFilter::pattern =
        java::util::regex::Pattern::compile(L"([^|]+)");

DaitchMokotoffSoundexFilter::DaitchMokotoffSoundexFilter(
    shared_ptr<TokenStream> in_, bool inject)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
  this->inject = inject;
}

bool DaitchMokotoffSoundexFilter::incrementToken() 
{
  if (matcher->find()) {
    assert(state != nullptr && encoded != L"");
    restoreState(state);
    termAtt->setEmpty()->append(encoded, matcher->start(1), matcher->end(1));
    posAtt->setPositionIncrement(0);
    return true;
  }

  if (input->incrementToken()) {
    // pass through zero-length terms
    if (termAtt->length() == 0) {
      return true;
    }

    // C++ TODO: There is no native C++ equivalent to 'toString':
    encoded = encoder->soundex(termAtt->toString());
    state = captureState();
    matcher->reset(encoded);

    if (!inject) {
      if (matcher->find()) {
        termAtt->setEmpty()->append(encoded, matcher->start(1),
                                    matcher->end(1));
      }
    }
    return true;
  } else {
    return false;
  }
}

void DaitchMokotoffSoundexFilter::reset() 
{
  TokenFilter::reset();
  matcher->reset(L"");
  state.reset();
}
} // namespace org::apache::lucene::analysis::phonetic