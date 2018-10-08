using namespace std;

#include "IndonesianAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../miscellaneous/SetKeywordMarkerFilter.h"
#include "IndonesianStemFilter.h"

namespace org::apache::lucene::analysis::id
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using StandardFilter = org::apache::lucene::analysis::standard::StandardFilter;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
const wstring IndonesianAnalyzer::DEFAULT_STOPWORD_FILE = L"stopwords.txt";

shared_ptr<CharArraySet> IndonesianAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    IndonesianAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

IndonesianAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        false, IndonesianAnalyzer::typeid, DEFAULT_STOPWORD_FILE, L"#");
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the
    // distribution (JAR)
    throw runtime_error(L"Unable to load default stopword set");
  }
}

DefaultSetHolder::StaticConstructor
    IndonesianAnalyzer::DefaultSetHolder::staticConstructor;

IndonesianAnalyzer::IndonesianAnalyzer()
    : IndonesianAnalyzer(DefaultSetHolder::DEFAULT_STOP_SET)
{
}

IndonesianAnalyzer::IndonesianAnalyzer(shared_ptr<CharArraySet> stopwords)
    : IndonesianAnalyzer(stopwords, CharArraySet::EMPTY_SET)
{
}

IndonesianAnalyzer::IndonesianAnalyzer(
    shared_ptr<CharArraySet> stopwords,
    shared_ptr<CharArraySet> stemExclusionSet)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      stemExclusionSet(
          CharArraySet::unmodifiableSet(CharArraySet::copy(stemExclusionSet)))
{
}

shared_ptr<TokenStreamComponents>
IndonesianAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const source = make_shared<StandardTokenizer>();
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(source);
  result = make_shared<LowerCaseFilter>(result);
  result = make_shared<StopFilter>(result, stopwords);
  if (!stemExclusionSet->isEmpty()) {
    result = make_shared<SetKeywordMarkerFilter>(result, stemExclusionSet);
  }
  return make_shared<TokenStreamComponents>(
      source, make_shared<IndonesianStemFilter>(result));
}

shared_ptr<TokenStream>
IndonesianAnalyzer::normalize(const wstring &fieldName,
                              shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<StandardFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::id