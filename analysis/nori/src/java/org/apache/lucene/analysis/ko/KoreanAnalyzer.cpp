using namespace std;

#include "KoreanAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/LowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "KoreanPartOfSpeechStopFilter.h"
#include "KoreanReadingFormFilter.h"
#include "dict/UserDictionary.h"

namespace org::apache::lucene::analysis::ko
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using LowerCaseFilter = org::apache::lucene::analysis::LowerCaseFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using DecompoundMode =
    org::apache::lucene::analysis::ko::KoreanTokenizer::DecompoundMode;
using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;
//    import static
//    org.apache.lucene.analysis.TokenStream.DEFAULT_TOKEN_ATTRIBUTE_FACTORY;

KoreanAnalyzer::KoreanAnalyzer()
    : KoreanAnalyzer(nullptr, KoreanTokenizer::DEFAULT_DECOMPOUND,
                     KoreanPartOfSpeechStopFilter::DEFAULT_STOP_TAGS, false)
{
}

KoreanAnalyzer::KoreanAnalyzer(shared_ptr<UserDictionary> userDict,
                               DecompoundMode mode,
                               shared_ptr<Set<POS::Tag>> stopTags,
                               bool outputUnknownUnigrams)
    : org::apache::lucene::analysis::Analyzer(), userDict(userDict), mode(mode),
      stopTags(stopTags), outputUnknownUnigrams(outputUnknownUnigrams)
{
}

shared_ptr<Analyzer::TokenStreamComponents>
KoreanAnalyzer::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<KoreanTokenizer>(TokenStream::DEFAULT_TOKEN_ATTRIBUTE_FACTORY,
                                   userDict, mode, outputUnknownUnigrams);
  shared_ptr<TokenStream> stream =
      make_shared<KoreanPartOfSpeechStopFilter>(tokenizer, stopTags);
  stream = make_shared<KoreanReadingFormFilter>(stream);
  stream = make_shared<LowerCaseFilter>(stream);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

shared_ptr<TokenStream> KoreanAnalyzer::normalize(const wstring &fieldName,
                                                  shared_ptr<TokenStream> in_)
{
  shared_ptr<TokenStream> result = make_shared<LowerCaseFilter>(in_);
  return result;
}
} // namespace org::apache::lucene::analysis::ko