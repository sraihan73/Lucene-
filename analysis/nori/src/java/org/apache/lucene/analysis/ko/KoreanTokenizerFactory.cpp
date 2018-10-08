using namespace std;

#include "KoreanTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "dict/UserDictionary.h"

namespace org::apache::lucene::analysis::ko
{
using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using IOUtils = org::apache::lucene::util::IOUtils;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
const wstring KoreanTokenizerFactory::USER_DICT_PATH = L"userDictionary";
const wstring KoreanTokenizerFactory::USER_DICT_ENCODING =
    L"userDictionaryEncoding";
const wstring KoreanTokenizerFactory::DECOMPOUND_MODE = L"decompoundMode";
const wstring KoreanTokenizerFactory::OUTPUT_UNKNOWN_UNIGRAMS =
    L"outputUnknownUnigrams";

KoreanTokenizerFactory::KoreanTokenizerFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      userDictionaryPath(args.erase(USER_DICT_PATH)),
      userDictionaryEncoding(args.erase(USER_DICT_ENCODING)),
      mode(KoreanTokenizer::DecompoundMode::valueOf(
          get(args, DECOMPOUND_MODE,
              KoreanTokenizer::DEFAULT_DECOMPOUND.toString())
              .toUpperCase(Locale::ROOT))),
      outputUnknownUnigrams(getBoolean(args, OUTPUT_UNKNOWN_UNIGRAMS, false))
{

  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void KoreanTokenizerFactory::inform(shared_ptr<ResourceLoader> loader) throw(
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

shared_ptr<KoreanTokenizer>
KoreanTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  return make_shared<KoreanTokenizer>(factory, userDictionary, mode,
                                      outputUnknownUnigrams);
}
} // namespace org::apache::lucene::analysis::ko