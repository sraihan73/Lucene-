using namespace std;

#include "DoubleMetaphoneFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::language::DoubleMetaphone;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
const wstring DoubleMetaphoneFilter::TOKEN_TYPE = L"DoubleMetaphone";

DoubleMetaphoneFilter::DoubleMetaphoneFilter(shared_ptr<TokenStream> input,
                                             int maxCodeLength, bool inject)
    : org::apache::lucene::analysis::TokenFilter(input), inject(inject)
{
  this->encoder->setMaxCodeLen(maxCodeLength);
}

bool DoubleMetaphoneFilter::incrementToken() 
{
  for (;;) {

    if (!remainingTokens.empty()) {
      // clearAttributes();  // not currently necessary
      restoreState(remainingTokens.pop_front());
      return true;
    }

    if (!input->incrementToken()) {
      return false;
    }

    int len = termAtt->length();
    if (len == 0) {
      return true; // pass through zero length terms
    }

    int firstAlternativeIncrement = inject ? 0 : posAtt->getPositionIncrement();

    // C++ TODO: There is no native C++ equivalent to 'toString':
    wstring v = termAtt->toString();
    wstring primaryPhoneticValue = encoder->doubleMetaphone(v);
    wstring alternatePhoneticValue = encoder->doubleMetaphone(v, true);

    // a flag to lazily save state if needed... this avoids a save/restore when
    // only one token will be generated.
    bool saveState = inject;

    if (primaryPhoneticValue != L"" && primaryPhoneticValue.length() > 0 &&
        primaryPhoneticValue != v) {
      if (saveState) {
        remainingTokens.push_back(captureState());
      }
      posAtt->setPositionIncrement(firstAlternativeIncrement);
      firstAlternativeIncrement = 0;
      termAtt->setEmpty()->append(primaryPhoneticValue);
      saveState = true;
    }

    if (alternatePhoneticValue != L"" && alternatePhoneticValue.length() > 0 &&
        alternatePhoneticValue != primaryPhoneticValue &&
        primaryPhoneticValue != v) {
      if (saveState) {
        remainingTokens.push_back(captureState());
        saveState = false;
      }
      posAtt->setPositionIncrement(firstAlternativeIncrement);
      termAtt->setEmpty()->append(alternatePhoneticValue);
      saveState = true;
    }

    // Just one token to return, so no need to capture/restore
    // any state, simply return it.
    if (remainingTokens.empty()) {
      return true;
    }

    if (saveState) {
      remainingTokens.push_back(captureState());
    }
  }
}

void DoubleMetaphoneFilter::reset() 
{
  input->reset();
  remainingTokens.clear();
}
} // namespace org::apache::lucene::analysis::phonetic