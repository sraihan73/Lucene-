using namespace std;

#include "TypeAsSynonymFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeSource.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;

TypeAsSynonymFilter::TypeAsSynonymFilter(shared_ptr<TokenStream> input)
    : TypeAsSynonymFilter(input, nullptr)
{
}

TypeAsSynonymFilter::TypeAsSynonymFilter(shared_ptr<TokenStream> input,
                                         const wstring &prefix)
    : org::apache::lucene::analysis::TokenFilter(input), prefix(prefix)
{
}

bool TypeAsSynonymFilter::incrementToken() 
{
  if (savedToken != nullptr) { // Emit last token's type at the same position
    restoreState(savedToken);
    savedToken.reset();
    termAtt->setEmpty();
    if (prefix != L"") {
      termAtt->append(prefix);
    }
    termAtt->append(typeAtt->type());
    posIncrAtt->setPositionIncrement(0);
    return true;
  } else if (input->incrementToken()) { // Ho pending token type to emit
    savedToken = captureState();
    return true;
  }
  return false;
}

void TypeAsSynonymFilter::reset() 
{
  TokenFilter::reset();
  savedToken.reset();
}
} // namespace org::apache::lucene::analysis::miscellaneous