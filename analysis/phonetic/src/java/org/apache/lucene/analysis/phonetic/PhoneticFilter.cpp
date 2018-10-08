using namespace std;

#include "PhoneticFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::Encoder;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;

PhoneticFilter::PhoneticFilter(shared_ptr<TokenStream> in_,
                               shared_ptr<Encoder> encoder, bool inject)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
  this->encoder = encoder;
  this->inject = inject;
}

bool PhoneticFilter::incrementToken() 
{
  if (save != nullptr) {
    // clearAttributes();  // not currently necessary
    restoreState(save);
    save.reset();
    return true;
  }

  if (!input->incrementToken()) {
    return false;
  }

  // pass through zero-length terms
  if (termAtt->length() == 0) {
    return true;
  }

  // C++ TODO: There is no native C++ equivalent to 'toString':
  wstring value = termAtt->toString();
  wstring phonetic = L"";
  try {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring v = encoder->encode(value)->toString();
    if (v.length() > 0 && value != v) {
      phonetic = v;
    }
  } catch (const runtime_error &ignored) {
  } // just use the direct text

  if (phonetic == L"") {
    return true;
  }

  if (!inject) {
    // just modify this token
    termAtt->setEmpty()->append(phonetic);
    return true;
  }

  // We need to return both the original and the phonetic tokens.
  // to avoid a orig=captureState() change_to_phonetic() saved=captureState()
  // restoreState(orig) we return the phonetic alternative first

  int origOffset = posAtt->getPositionIncrement();
  posAtt->setPositionIncrement(0);
  save = captureState();

  posAtt->setPositionIncrement(origOffset);
  termAtt->setEmpty()->append(phonetic);
  return true;
}

void PhoneticFilter::reset() 
{
  input->reset();
  save.reset();
}
} // namespace org::apache::lucene::analysis::phonetic