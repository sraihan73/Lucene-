using namespace std;

#include "UkrainianMorfologikAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/WordlistLoader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/charfilter/MappingCharFilter.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "../morfologik/MorfologikFilter.h"

namespace org::apache::lucene::analysis::uk
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using MappingCharFilter =
    org::apache::lucene::analysis::charfilter::MappingCharFilter;
using NormalizeCharMap =
    org::apache::lucene::analysis::charfilter::NormalizeCharMap;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using MorfologikFilter =
    org::apache::lucene::analysis::morfologik::MorfologikFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;
using morfologik::stemming::Dictionary;
const wstring UkrainianMorfologikAnalyzer::DEFAULT_STOPWORD_FILE =
    L"stopwords.txt";

shared_ptr<CharArraySet> UkrainianMorfologikAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    UkrainianMorfologikAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

UkrainianMorfologikAnalyzer::DefaultSetHolder::StaticConstructor::
    StaticConstructor()
{
  try {
    DEFAULT_STOP_SET =
        WordlistLoader::getSnowballWordSet(IOUtils::getDecodingReader(
            UkrainianMorfologikAnalyzer::typeid, DEFAULT_STOPWORD_FILE,
            StandardCharsets::UTF_8));
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    UkrainianMorfologikAnalyzer::DefaultSetHolder::staticConstructor;

UkrainianMorfologikAnalyzer::UkrainianMorfologikAnalyzer()
    : UkrainianMorfologikAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

UkrainianMorfologikAnalyzer::UkrainianMorfologikAnalyzer(
    shared_ptr<CharArraySet> stopwords)
    : UkrainianMorfologikAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

UkrainianMorfologikAnalyzer::UkrainianMorfologikAnalyzer(
    shared_ptr<CharArraySet> stopwords,
    shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<Reader>
UkrainianMorfologikAnalyzer::initReader(const wstring &fieldName,
                                        shared_ptr<Reader> reader)
{
  shared_ptr<NormalizeCharMap::Builder> builder =
      make_shared<NormalizeCharMap::Builder>();
  // different apostrophes
  builder->add(L"\u2019", L"'");
  builder->add(L"\u2018", L"'");
  builder->add(L"\u02BC", L"'");
  builder->add(L"`", L"'");
  builder->add(L"´", L"'");
  // ignored characters
  builder->add(L"\u0301", L"");
  builder->add(L"\u00AD", L"");
  builder->add(L"ґ", L"г");
  builder->add(L"Ґ", L"Г");

  shared_ptr<NormalizeCharMap> normMap = builder->build();
  reader = make_shared<MappingCharFilter>(normMap, reader);
  return reader;
}

shared_ptr<Analyzer::TokenStreamComponents>
UkrainianMorfologikAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);

  if (stemExclusionSet->isEmpty() == false) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }

  result = make_shared<MorfologikFilter>(result, getDictionary());
  return make_shared<Analyzer::TokenStreamComponents>(source, result);
}

shared_ptr<Dictionary> UkrainianMorfologikAnalyzer::getDictionary()
{
  try {
    return Dictionary::read(
        UkrainianMorfologikAnalyzer::typeid->getClassLoader().getResource(
            L"ua/net/nlp/ukrainian.dict"));
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::analysis::uk