using namespace std;

#include "BeiderMorseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::language::bm::BeiderMorseEncoder;
using org::apache::commons::codec::language::bm::PhoneticEngine;
using org::apache::commons::codec::language::bm::Languages::LanguageSet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
const shared_ptr<java::util::regex::Pattern> BeiderMorseFilter::pattern =
    java::util::regex::Pattern::compile(L"([^()|-]+)");

BeiderMorseFilter::BeiderMorseFilter(shared_ptr<TokenStream> input,
                                     shared_ptr<PhoneticEngine> engine)
    : BeiderMorseFilter(input, engine, nullptr)
{
}

BeiderMorseFilter::BeiderMorseFilter(shared_ptr<TokenStream> input,
                                     shared_ptr<PhoneticEngine> engine,
                                     shared_ptr<LanguageSet> languages)
    : org::apache::lucene::analysis::TokenFilter(input), engine(engine),
      languages(languages)
{
}

bool BeiderMorseFilter::incrementToken() 
{
  if (matcher->find()) {
    assert(state != nullptr && encoded != L"");
    restoreState(state);
    termAtt->setEmpty()->append(encoded, matcher->start(1), matcher->end(1));
    posIncAtt->setPositionIncrement(0);
    return true;
  }

  if (input->incrementToken()) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    encoded = (languages == nullptr)
                  ? engine->encode(termAtt->toString())
                  : engine->encode(termAtt->toString(), languages);
    state = captureState();
    matcher->reset(encoded);
    if (matcher->find()) {
      termAtt->setEmpty()->append(encoded, matcher->start(1), matcher->end(1));
    }
    return true;
  } else {
    return false;
  }
}

void BeiderMorseFilter::reset() 
{
  TokenFilter::reset();
  matcher->reset(L"");
}
} // namespace org::apache::lucene::analysis::phonetic