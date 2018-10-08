using namespace std;

#include "BeiderMorseFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "BeiderMorseFilter.h"

namespace org::apache::lucene::analysis::phonetic
{
using org::apache::commons::codec::language::bm::NameType;
using org::apache::commons::codec::language::bm::PhoneticEngine;
using org::apache::commons::codec::language::bm::RuleType;
using org::apache::commons::codec::language::bm::Languages::LanguageSet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;

BeiderMorseFilterFactory::BeiderMorseFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      engine(make_shared<PhoneticEngine>(nameType, ruleType, concat)),
      languageSet(
          (nullptr == langs || (1 == langs->size() && langs->contains(L"auto")))
              ? nullptr
              : LanguageSet::from(langs))
{
  // PhoneticEngine = NameType + RuleType + concat
  // we use common-codec's defaults: GENERIC + APPROX + true
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<NameType> nameType = StringHelper::fromString<NameType>(
      get(args, L"nameType", NameType::GENERIC->toString()));
  // C++ TODO: There is no native C++ equivalent to 'toString':
  shared_ptr<RuleType> ruleType = StringHelper::fromString<RuleType>(
      get(args, L"ruleType", RuleType::APPROX->toString()));

  bool concat = getBoolean(args, L"concat", true);

  // LanguageSet: defaults to automagic, otherwise a comma-separated deque.
  shared_ptr<Set<wstring>> langs = getSet(args, L"languageSet");
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<TokenStream>
BeiderMorseFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<BeiderMorseFilter>(input, engine, languageSet);
}
} // namespace org::apache::lucene::analysis::phonetic