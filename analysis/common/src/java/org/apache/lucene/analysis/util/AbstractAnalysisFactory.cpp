using namespace std;

#include "AbstractAnalysisFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/WordlistLoader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "ResourceLoader.h"

namespace org::apache::lucene::analysis::util
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;
using IOUtils = org::apache::lucene::util::IOUtils;
using Version = org::apache::lucene::util::Version;
const wstring AbstractAnalysisFactory::LUCENE_MATCH_VERSION_PARAM =
    L"luceneMatchVersion";

AbstractAnalysisFactory::AbstractAnalysisFactory(
    unordered_map<wstring, wstring> &args)
    : originalArgs(args)
{
  wstring version = get(args, LUCENE_MATCH_VERSION_PARAM);
  if (version == L"") {
    luceneMatchVersion = Version::LATEST;
  } else {
    try {
      luceneMatchVersion = Version::parseLeniently(version);
    } catch (const ParseException &pe) {
      throw invalid_argument(pe);
    }
  }
  args.erase(CLASS_NAME); // consume the class arg
}

unordered_map<wstring, wstring> AbstractAnalysisFactory::getOriginalArgs()
{
  return originalArgs;
}

shared_ptr<Version> AbstractAnalysisFactory::getLuceneMatchVersion()
{
  return this->luceneMatchVersion;
}

wstring AbstractAnalysisFactory::require(unordered_map<wstring, wstring> &args,
                                         const wstring &name)
{
  wstring s = args.erase(name);
  if (s == L"") {
    throw invalid_argument(L"Configuration Error: missing parameter '" + name +
                           L"'");
  }
  return s;
}

wstring
AbstractAnalysisFactory::require(unordered_map<wstring, wstring> &args,
                                 const wstring &name,
                                 shared_ptr<deque<wstring>> allowedValues)
{
  return require(args, name, allowedValues, true);
}

wstring AbstractAnalysisFactory::require(
    unordered_map<wstring, wstring> &args, const wstring &name,
    shared_ptr<deque<wstring>> allowedValues, bool caseSensitive)
{
  wstring s = args.erase(name);
  if (s == L"") {
    throw invalid_argument(L"Configuration Error: missing parameter '" + name +
                           L"'");
  } else {
    for (auto allowedValue : allowedValues) {
      if (caseSensitive) {
        if (s == allowedValue) {
          return s;
        }
      } else {
        // C++ TODO: The following Java case-insensitive std::wstring method call is
        // not converted:
        if (s.equalsIgnoreCase(allowedValue)) {
          return s;
        }
      }
    }
    throw invalid_argument(L"Configuration Error: '" + name +
                           L"' value must be one of " + allowedValues);
  }
}

wstring AbstractAnalysisFactory::get(unordered_map<wstring, wstring> &args,
                                     const wstring &name)
{
  return args.erase(name); // defaultVal = null
}

wstring AbstractAnalysisFactory::get(unordered_map<wstring, wstring> &args,
                                     const wstring &name,
                                     const wstring &defaultVal)
{
  wstring s = args.erase(name);
  return s == L"" ? defaultVal : s;
}

wstring
AbstractAnalysisFactory::get(unordered_map<wstring, wstring> &args,
                             const wstring &name,
                             shared_ptr<deque<wstring>> allowedValues)
{
  return get(args, name, allowedValues, L""); // defaultVal = null
}

wstring AbstractAnalysisFactory::get(
    unordered_map<wstring, wstring> &args, const wstring &name,
    shared_ptr<deque<wstring>> allowedValues, const wstring &defaultVal)
{
  return get(args, name, allowedValues, defaultVal, true);
}

wstring
AbstractAnalysisFactory::get(unordered_map<wstring, wstring> &args,
                             const wstring &name,
                             shared_ptr<deque<wstring>> allowedValues,
                             const wstring &defaultVal, bool caseSensitive)
{
  wstring s = args.erase(name);
  if (s == L"") {
    return defaultVal;
  } else {
    for (auto allowedValue : allowedValues) {
      if (caseSensitive) {
        if (s == allowedValue) {
          return s;
        }
      } else {
        // C++ TODO: The following Java case-insensitive std::wstring method call is
        // not converted:
        if (s.equalsIgnoreCase(allowedValue)) {
          return s;
        }
      }
    }
    throw invalid_argument(L"Configuration Error: '" + name +
                           L"' value must be one of " + allowedValues);
  }
}

int AbstractAnalysisFactory::requireInt(unordered_map<wstring, wstring> &args,
                                        const wstring &name)
{
  return stoi(require(args, name));
}

int AbstractAnalysisFactory::getInt(unordered_map<wstring, wstring> &args,
                                    const wstring &name, int defaultVal)
{
  wstring s = args.erase(name);
  return s == L"" ? defaultVal : stoi(s);
}

bool AbstractAnalysisFactory::requireBoolean(
    unordered_map<wstring, wstring> &args, const wstring &name)
{
  return StringHelper::fromString<bool>(require(args, name));
}

bool AbstractAnalysisFactory::getBoolean(unordered_map<wstring, wstring> &args,
                                         const wstring &name, bool defaultVal)
{
  wstring s = args.erase(name);
  return s == L"" ? defaultVal : StringHelper::fromString<bool>(s);
}

float AbstractAnalysisFactory::requireFloat(
    unordered_map<wstring, wstring> &args, const wstring &name)
{
  return stof(require(args, name));
}

float AbstractAnalysisFactory::getFloat(unordered_map<wstring, wstring> &args,
                                        const wstring &name, float defaultVal)
{
  wstring s = args.erase(name);
  return s == L"" ? defaultVal : stof(s);
}

wchar_t
AbstractAnalysisFactory::requireChar(unordered_map<wstring, wstring> &args,
                                     const wstring &name)
{
  return require(args, name)[0];
}

wchar_t AbstractAnalysisFactory::getChar(unordered_map<wstring, wstring> &args,
                                         const wstring &name,
                                         wchar_t defaultValue)
{
  wstring s = args.erase(name);
  if (s == L"") {
    return defaultValue;
  } else {
    if (s.length() != 1) {
      throw invalid_argument(name + L" should be a char. \"" + s +
                             L"\" is invalid");
    } else {
      return s[0];
    }
  }
}

const shared_ptr<java::util::regex::Pattern>
    AbstractAnalysisFactory::ITEM_PATTERN =
        java::util::regex::Pattern::compile(L"[^,\\s]+");

shared_ptr<Set<wstring>>
AbstractAnalysisFactory::getSet(unordered_map<wstring, wstring> &args,
                                const wstring &name)
{
  wstring s = args.erase(name);
  if (s == L"") {
    return nullptr;
  } else {
    shared_ptr<Set<wstring>> set = nullptr;
    shared_ptr<Matcher> matcher = ITEM_PATTERN->matcher(s);
    if (matcher->find()) {
      set = unordered_set<>();
      set->add(matcher->group(0));
      while (matcher->find()) {
        set->add(matcher->group(0));
      }
    }
    return set;
  }
}

shared_ptr<Pattern>
AbstractAnalysisFactory::getPattern(unordered_map<wstring, wstring> &args,
                                    const wstring &name)
{
  try {
    return Pattern::compile(require(args, name));
  } catch (const PatternSyntaxException &e) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Configuration Error:
    // '" + name + "' can not be parsed in " + this.getClass().getSimpleName(),
    // e);
    throw invalid_argument(L"Configuration Error: '" + name +
                           L"' can not be parsed in " +
                           this->getClass().getSimpleName());
  }
}

shared_ptr<CharArraySet>
AbstractAnalysisFactory::getWordSet(shared_ptr<ResourceLoader> loader,
                                    const wstring &wordFiles,
                                    bool ignoreCase) 
{
  deque<wstring> files = splitFileNames(wordFiles);
  shared_ptr<CharArraySet> words = nullptr;
  if (files.size() > 0) {
    // default stopwords deque has 35 or so words, but maybe don't make it that
    // big to start
    words = make_shared<CharArraySet>(files.size() * 10, ignoreCase);
    for (auto file : files) {
      deque<wstring> wlist = getLines(loader, file.trim());
      words->addAll(StopFilter::makeStopSet(wlist, ignoreCase));
    }
  }
  return words;
}

deque<wstring>
AbstractAnalysisFactory::getLines(shared_ptr<ResourceLoader> loader,
                                  const wstring &resource) 
{
  return WordlistLoader::getLines(loader->openResource(resource),
                                  StandardCharsets::UTF_8);
}

shared_ptr<CharArraySet>
AbstractAnalysisFactory::getSnowballWordSet(shared_ptr<ResourceLoader> loader,
                                            const wstring &wordFiles,
                                            bool ignoreCase) 
{
  deque<wstring> files = splitFileNames(wordFiles);
  shared_ptr<CharArraySet> words = nullptr;
  if (files.size() > 0) {
    // default stopwords deque has 35 or so words, but maybe don't make it that
    // big to start
    words = make_shared<CharArraySet>(files.size() * 10, ignoreCase);
    for (auto file : files) {
      shared_ptr<InputStream> stream = nullptr;
      shared_ptr<Reader> reader = nullptr;
      try {
        stream = loader->openResource(file.trim());
        shared_ptr<CharsetDecoder> decoder =
            StandardCharsets::UTF_8::newDecoder()
                .onMalformedInput(CodingErrorAction::REPORT)
                .onUnmappableCharacter(CodingErrorAction::REPORT);
        reader = make_shared<InputStreamReader>(stream, decoder);
        WordlistLoader::getSnowballWordSet(reader, words);
      }
      // C++ TODO: There is no native C++ equivalent to the exception 'finally'
      // clause:
      finally {
        IOUtils::closeWhileHandlingException({reader, stream});
      }
    }
  }
  return words;
}

deque<wstring>
AbstractAnalysisFactory::splitFileNames(const wstring &fileNames)
{
  return splitAt(L',', fileNames);
}

deque<wstring> AbstractAnalysisFactory::splitAt(wchar_t separator,
                                                 const wstring &deque)
{
  if (deque == L"") {
    return Collections::emptyList();
  }

  deque<wstring> result = deque<wstring>();
  for (wstring item :
       deque.split(L"(?<!\\\\)[" + StringHelper::toString(separator) + L"]")) {
    result.push_back(item.replaceAll(
        L"\\\\(?=[" + StringHelper::toString(separator) + L"])", L""));
  }

  return result;
}

const wstring AbstractAnalysisFactory::CLASS_NAME = L"class";

wstring AbstractAnalysisFactory::getClassArg()
{
  if (nullptr != originalArgs) {
    wstring className = originalArgs[CLASS_NAME];
    if (L"" != className) {
      return className;
    }
  }
  return getClassName();
}

bool AbstractAnalysisFactory::isExplicitLuceneMatchVersion()
{
  return isExplicitLuceneMatchVersion_;
}

void AbstractAnalysisFactory::setExplicitLuceneMatchVersion(
    bool isExplicitLuceneMatchVersion)
{
  this->isExplicitLuceneMatchVersion_ = isExplicitLuceneMatchVersion;
}
} // namespace org::apache::lucene::analysis::util