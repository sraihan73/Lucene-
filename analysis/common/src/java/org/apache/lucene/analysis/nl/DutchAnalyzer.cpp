using namespace std;

#include "DutchAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/WordlistLoader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../tartarus/snowball/ext/DutchStemmer.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "../miscellaneous/StemmerOverrideFilter.h"
#include "../snowball/SnowballFilter.h"

namespace org::apache::lucene::analysis::nl
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArrayMap = org::apache::lucene::analysis::CharArrayMap;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StemmerOverrideMap = org::apache::lucene::analysis::miscellaneous::
    StemmerOverrideFilter::StemmerOverrideMap;
using StemmerOverrideFilter =
    org::apache::lucene::analysis::miscellaneous::StemmerOverrideFilter;
using SnowballFilter = org::apache::lucene::analysis::snowball::SnowballFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using IOUtils = org::apache::lucene::util::IOUtils;
const wstring DutchAnalyzer::DEFAULT_STOPWORD_FILE = L"dutch_stop.txt";

shared_ptr<CharArraySet> DutchAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    DutchAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;
const shared_ptr<org::apache::lucene::analysis::CharArrayMap<wstring>>
    DutchAnalyzer::DefaultSetHolder::DEFAULT_STEM_DICT;

DutchAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET =
        WordlistLoader::getSnowballWordSet(IOUtils::getDecodingReader(
            SnowballFilter::typeid, DEFAULT_STOPWORD_FILE,
            StandardCharsets::UTF_8));
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }

  DEFAULT_STEM_DICT = make_shared<CharArrayMap<wstring>>(4, false);
  DEFAULT_STEM_DICT->put(L"fiets", L"fiets");         // otherwise fiet
  DEFAULT_STEM_DICT->put(L"bromfiets", L"bromfiets"); // otherwise bromfiet
  DEFAULT_STEM_DICT->put(L"ei", L"eier");
  DEFAULT_STEM_DICT->put(L"kind", L"kinder");
}

DefaultSetHolder::StaticConstructor
    DutchAnalyzer::DefaultSetHolder::staticConstructor;

DutchAnalyzer::DutchAnalyzer()
    : DutchAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET, CharArraySet::EMPTY_SET,
                    DefaultSetHolder::DEFAULT_STEM_DICT)
{
}

DutchAnalyzer::DutchAnalyzer(shared_ptr<CharArraySet> stopwords)
    : DutchAnalyzer(stopwords, CharArraySet::EMPTY_SET,
                    DefaultSetHolder::DEFAULT_STEM_DICT)
{
}

DutchAnalyzer::DutchAnalyzer(shared_ptr<CharArraySet> stopwords,
                             shared_ptr<CharArraySet> stemExclusionTable)
    : DutchAnalyzer(stopwords, stemExclusionTable,
                    DefaultSetHolder::DEFAULT_STEM_DICT)
{
}

DutchAnalyzer::DutchAnalyzer(shared_ptr<CharArraySet> stopwords,
                             shared_ptr<CharArraySet> stemExclusionTable,
                             shared_ptr<CharArrayMap<wstring>> stemOverrideDict)
    : stoptable(CharArraySet::unmodifiableSet(CharArraySet::copy(stopwords)))
{
  this->excltable =
      CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionTable));
  if (stemOverrideDict->isEmpty()) {
    this->stemdict.reset();
  } else {
    // we don't need to ignore case here since we lowercase in this analyzer
    // anyway
    shared_ptr<StemmerOverrideFilter::Builder> builder =
        make_shared<StemmerOverrideFilter::Builder>(false);
    shared_ptr<CharArrayMap<wstring>::EntryIterator> iter =
        stemOverrideDict->entrySet()->begin();
    shared_ptr<CharsRefBuilder> spare = make_shared<CharsRefBuilder>();
    while (iter->hasNext()) {
      std::deque<wchar_t> nextKey = iter->nextKey();
      spare->copyChars(nextKey, 0, nextKey.size());
      builder->add(spare->get(), iter->currentValue());
      iter++;
    }
    try {
      this->stemdict = builder->build();
    } catch (const IOException &ex) {
      // C++ TODO: This exception's constructor requires only one argument:
      // ORIGINAL LINE: throw new RuntimeException("can not build stem dict",
      // ex);
      throw runtime_error(L"can not build stem dict");
    }
  }
}

shared_ptr<Analyzer::TokenStreamComponents>
DutchAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stoptable);
  if (!excltable->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, excltable);
  }
  if (stemdict != nullptr) {
    result = make_shared<StemmerOverrideFilter>(result, stemdict);
  }
  result = make_shared<SnowballFilter>(
      result, make_shared<org::tartarus::snowball::ext::DutchStemmer>());
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<TokenStream> DutchAnalyzer::normalize(const wstring &fieldName,
                                                 shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::nl