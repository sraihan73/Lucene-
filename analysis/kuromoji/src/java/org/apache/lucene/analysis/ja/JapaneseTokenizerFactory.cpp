using namespace std;

#include "JapaneseTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "dict/UserDictionary.h"

namespace org::apache::lucene::analysis::ja
{
using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using IOUtils = org::apache::lucene::util::IOUtils;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
const wstring JapaneseTokenizerFactory::MODE = L"mode";
const wstring JapaneseTokenizerFactory::USER_DICT_PATH = L"userDictionary";
const wstring JapaneseTokenizerFactory::USER_DICT_ENCODING =
    L"userDictionaryEncoding";
const wstring JapaneseTokenizerFactory::DISCARD_PUNCTUATION =
    L"discardPunctuation";
const wstring JapaneseTokenizerFactory::NBEST_COST = L"nBestCost";
const wstring JapaneseTokenizerFactory::NBEST_EXAMPLES = L"nBestExamples";

JapaneseTokenizerFactory::JapaneseTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      mode(Mode::valueOf(
          get(args, MODE, JapaneseTokenizer::DEFAULT_MODE.toString())
              .toUpperCase(Locale::ROOT))),
      discardPunctuation(getBoolean(args, DISCARD_PUNCTUATION, true)),
      userDictionaryPath(args.erase(USER_DICT_PATH)),
      userDictionaryEncoding(args.erase(USER_DICT_ENCODING)),
      nbestExamples(args.erase(NBEST_EXAMPLES))
{
  nbestCost = getInt(args, NBEST_COST, 0);
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void JapaneseTokenizerFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  if (userDictionaryPath != L"") {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.InputStream stream =
    // loader.openResource(userDictionaryPath))
    {
      java::io::InputStream stream = loader->openResource(userDictionaryPath);
      wstring encoding = userDictionaryEncoding;
      if (encoding == L"") {
        encoding = IOUtils::UTF_8;
      }
      shared_ptr<CharsetDecoder> decoder =
          Charset::forName(encoding)
              .newDecoder()
              .onMalformedInput(CodingErrorAction::REPORT)
              .onUnmappableCharacter(CodingErrorAction::REPORT);
      shared_ptr<Reader> reader =
          make_shared<InputStreamReader>(stream, decoder);
      userDictionary = UserDictionary::open(reader);
    }
  } else {
    userDictionary.reset();
  }
}

shared_ptr<JapaneseTokenizer>
JapaneseTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  shared_ptr<JapaneseTokenizer> t = make_shared<JapaneseTokenizer>(
      factory, userDictionary, discardPunctuation, mode);
  if (nbestExamples != L"") {
    nbestCost = max(nbestCost, t->calcNBestCost(nbestExamples));
  }
  t->setNBestCost(nbestCost);
  return t;
}
} // namespace org::apache::lucene::analysis::ja