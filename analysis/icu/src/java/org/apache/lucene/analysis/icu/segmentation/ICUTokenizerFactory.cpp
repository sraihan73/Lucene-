using namespace std;

#include "ICUTokenizerFactory.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include "../../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoader.h"
#include "ICUTokenizer.h"
#include "ICUTokenizerConfig.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using IOUtils = org::apache::lucene::util::IOUtils;
using com::ibm::icu::lang::UCharacter;
using com::ibm::icu::lang::UProperty;
using com::ibm::icu::text::BreakIterator;
using com::ibm::icu::text::RuleBasedBreakIterator;
const wstring ICUTokenizerFactory::RULEFILES = L"rulefiles";

ICUTokenizerFactory::ICUTokenizerFactory(unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenizerFactory(args),
      tailored(unordered_map<>()),
      cjkAsWords(getBoolean(args, L"cjkAsWords", true)),
      myanmarAsWords(getBoolean(args, L"myanmarAsWords", true))
{
  wstring rulefilesArg = get(args, RULEFILES);
  if (rulefilesArg != L"") {
    deque<wstring> scriptAndResourcePaths = splitFileNames(rulefilesArg);
    for (auto scriptAndResourcePath : scriptAndResourcePaths) {
      int colonPos = scriptAndResourcePath.find(L":");
      wstring scriptCode = scriptAndResourcePath.substr(0, colonPos)->trim();
      wstring resourcePath = scriptAndResourcePath.substr(colonPos + 1)->trim();
      tailored.emplace(
          UCharacter::getPropertyValueEnum(UProperty::SCRIPT, scriptCode),
          resourcePath);
    }
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void ICUTokenizerFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  assert((tailored.size() > 0, L"init must be called first!"));
  if (tailored.empty()) {
    config = make_shared<DefaultICUTokenizerConfig>(cjkAsWords, myanmarAsWords);
  } else {
    std::deque<std::shared_ptr<BreakIterator>> breakers(
        1 + UCharacter::getIntPropertyMaxValue(UProperty::SCRIPT));
    for (auto entry : tailored) {
      int code = entry.first;
      wstring resourcePath = entry.second;
      breakers[code] = parseRules(resourcePath, loader);
    }
    config = make_shared<DefaultICUTokenizerConfigAnonymousInnerClass>(
        shared_from_this(), cjkAsWords, myanmarAsWords, breakers);
  }
}

ICUTokenizerFactory::DefaultICUTokenizerConfigAnonymousInnerClass::
    DefaultICUTokenizerConfigAnonymousInnerClass(
        shared_ptr<ICUTokenizerFactory> outerInstance, bool cjkAsWords,
        bool myanmarAsWords, deque<std::shared_ptr<BreakIterator>> &breakers)
    : DefaultICUTokenizerConfig(cjkAsWords, myanmarAsWords)
{
  this->outerInstance = outerInstance;
  this->breakers = breakers;
}

shared_ptr<RuleBasedBreakIterator> ICUTokenizerFactory::
    DefaultICUTokenizerConfigAnonymousInnerClass::getBreakIterator(int script)
{
  if (breakers[script] != nullptr) {
    return std::static_pointer_cast<RuleBasedBreakIterator>(
        breakers[script]->clone());
  } else {
    return outerInstance->super->getBreakIterator(script);
  }
}

shared_ptr<BreakIterator> ICUTokenizerFactory::parseRules(
    const wstring &filename,
    shared_ptr<ResourceLoader> loader) 
{
  shared_ptr<StringBuilder> rules = make_shared<StringBuilder>();
  shared_ptr<InputStream> rulesStream = loader->openResource(filename);
  shared_ptr<BufferedReader> reader = make_shared<BufferedReader>(
      IOUtils::getDecodingReader(rulesStream, StandardCharsets::UTF_8));
  wstring line = L"";
  while ((line = reader->readLine()) != L"") {
    if (!StringHelper::startsWith(line, L"#")) {
      rules->append(line);
    }
    rules->append(L'\n');
  }
  reader->close();
  return make_shared<RuleBasedBreakIterator>(rules->toString());
}

shared_ptr<ICUTokenizer>
ICUTokenizerFactory::create(shared_ptr<AttributeFactory> factory)
{
  assert((config != nullptr, L"inform must be called first!"));
  return make_shared<ICUTokenizer>(factory, config);
}
} // namespace org::apache::lucene::analysis::icu::segmentation