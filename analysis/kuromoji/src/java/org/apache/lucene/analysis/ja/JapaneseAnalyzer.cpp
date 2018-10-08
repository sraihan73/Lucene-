using namespace std;

#include "JapaneseAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/cjk/CJKWidthFilter.h"
#include "JapaneseBaseFormFilter.h"
#include "JapaneseKatakanaStemFilter.h"
#include "JapanesePartOfSpeechStopFilter.h"
#include "dict/UserDictionary.h"

namespace org::apache::lucene::analysis::ja
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CJKWidthFilter = org::apache::lucene::analysis::cjk::CJKWidthFilter;
using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;

JapaneseAnalyzer::JapaneseAnalyzer()
    : JapaneseAnalyzer(nullptr, JapaneseTokenizer::DEFAULT_MODE,
                       DefaultSetHolder::DEFAULT_STOP_SET,
                       DefaultSetHolder::DEFAULT_STOP_TAGS)
{
}

JapaneseAnalyzer::JapaneseAnalyzer(shared_ptr<UserDictionary> userDict,
                                   Mode mode,
                                   shared_ptr<CharArraySet> stopwords,
                                   shared_ptr<Set<wstring>> stoptags)
    : org::apache::lucene::analysis::StopwordAnalyzerBase(stopwords),
      mode(mode), stoptags(stoptags), userDict(userDict)
{
}

shared_ptr<CharArraySet> JapaneseAnalyzer::getDefaultStopSet()
{
  return DefaultSetHolder::DEFAULT_STOP_SET;
}

shared_ptr<Set<wstring>> JapaneseAnalyzer::getDefaultStopTags()
{
  return DefaultSetHolder::DEFAULT_STOP_TAGS;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    JapaneseAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;
const shared_ptr<java::util::Set<wstring>>
    JapaneseAnalyzer::DefaultSetHolder::DEFAULT_STOP_TAGS;

JapaneseAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
  try {
    DEFAULT_STOP_SET = StopwordAnalyzerBase::loadStopwordSet(
        true, JapaneseAnalyzer::typeid, L"stopwords.txt", L"#"); // ignore case
    shared_ptr<CharArraySet> *const tagset =
        StopwordAnalyzerBase::loadStopwordSet(false, JapaneseAnalyzer::typeid,
                                              L"stoptags.txt", L"#");
    DEFAULT_STOP_TAGS = unordered_set<>();
    for (auto element : tagset) {
      std::deque<wchar_t> chars = static_cast<std::deque<wchar_t>>(element);
      DEFAULT_STOP_TAGS->add(wstring(chars));
    }
  } catch (const IOException &ex) {
    // default set should always be present as it is part of the distribution
    // (JAR)
    throw runtime_error(L"Unable to load default stopword or stoptag set");
  }
}

DefaultSetHolder::StaticConstructor
    JapaneseAnalyzer::DefaultSetHolder::staticConstructor;

shared_ptr<TokenStreamComponents>
JapaneseAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<JapaneseTokenizer>(userDict, true, mode);
  shared_ptr<TokenStream> stream =
      make_shared<JapaneseBaseFormFilter>(tokenizer);
  stream = make_shared<JapanesePartOfSpeechStopFilter>(stream, stoptags);
  stream = make_shared<CJKWidthFilter>(stream);
  stream = make_shared<StopFilter>(stream, stopwords);
  stream = make_shared<JapaneseKatakanaStemFilter>(stream);
  stream = make_shared<LowerCaseFilter>(stream);
  return make_shared<TokenStreamComponents>(tokenizer, stream);
}

shared_ptr<TokenStream> JapaneseAnalyzer::normalize(const wstring &fieldName,
                                                    shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<CJKWidthFilter>(in_);
  result = make_shared<LowerCaseFilter>(result);
  return result;
}
} // namespace org::apache::lucene::analysis::ja