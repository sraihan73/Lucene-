using namespace std;

#include "WordDelimiterGraphFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../util/ResourceLoader.h"
#include "WordDelimiterGraphFilter.h"
#include "WordDelimiterIterator.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using namespace org::apache::lucene::analysis::miscellaneous;
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterGraphFilter.*;
//    import static
//    org.apache.lucene.analysis.miscellaneous.WordDelimiterIterator.*;
const wstring WordDelimiterGraphFilterFactory::PROTECTED_TOKENS = L"protected";
const wstring WordDelimiterGraphFilterFactory::TYPES = L"types";

WordDelimiterGraphFilterFactory::WordDelimiterGraphFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::TokenFilterFactory(args),
      wordFiles(get(args, PROTECTED_TOKENS)), types(get(args, TYPES)),
      flags(flags)
{
  int flags = 0;
  if (getInt(args, L"generateWordParts", 1) != 0) {
    flags |= WordDelimiterGraphFilter::GENERATE_WORD_PARTS;
  }
  if (getInt(args, L"generateNumberParts", 1) != 0) {
    flags |= WordDelimiterGraphFilter::GENERATE_NUMBER_PARTS;
  }
  if (getInt(args, L"catenateWords", 0) != 0) {
    flags |= WordDelimiterGraphFilter::CATENATE_WORDS;
  }
  if (getInt(args, L"catenateNumbers", 0) != 0) {
    flags |= WordDelimiterGraphFilter::CATENATE_NUMBERS;
  }
  if (getInt(args, L"catenateAll", 0) != 0) {
    flags |= WordDelimiterGraphFilter::CATENATE_ALL;
  }
  if (getInt(args, L"splitOnCaseChange", 1) != 0) {
    flags |= WordDelimiterGraphFilter::SPLIT_ON_CASE_CHANGE;
  }
  if (getInt(args, L"splitOnNumerics", 1) != 0) {
    flags |= WordDelimiterGraphFilter::SPLIT_ON_NUMERICS;
  }
  if (getInt(args, L"preserveOriginal", 0) != 0) {
    flags |= WordDelimiterGraphFilter::PRESERVE_ORIGINAL;
  }
  if (getInt(args, L"stemEnglishPossessive", 1) != 0) {
    flags |= WordDelimiterGraphFilter::STEM_ENGLISH_POSSESSIVE;
  }
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void WordDelimiterGraphFilterFactory::inform(
    shared_ptr<ResourceLoader> loader) 
{
  if (wordFiles != L"") {
    protectedWords = getWordSet(loader, wordFiles, false);
  }
  if (types != L"") {
    deque<wstring> files = splitFileNames(types);
    deque<wstring> wlist = deque<wstring>();
    for (auto file : files) {
      deque<wstring> lines = getLines(loader, file.trim());
      wlist.insert(wlist.end(), lines.begin(), lines.end());
    }
    typeTable = parseTypes(wlist);
  }
}

shared_ptr<TokenFilter>
WordDelimiterGraphFilterFactory::create(shared_ptr<TokenStream> input)
{
  return make_shared<WordDelimiterGraphFilter>(
      input,
      typeTable.empty() ? WordDelimiterIterator::DEFAULT_WORD_DELIM_TABLE
                        : typeTable,
      flags, protectedWords);
}

shared_ptr<java::util::regex::Pattern>
    WordDelimiterGraphFilterFactory::typePattern =
        java::util::regex::Pattern::compile(L"(.*)\\s*=>\\s*(.*)\\s*$");

std::deque<char>
WordDelimiterGraphFilterFactory::parseTypes(deque<wstring> &rules)
{
  shared_ptr<SortedMap<wchar_t, char>> typeMap = map_obj<wchar_t, char>();
  for (auto rule : rules) {
    shared_ptr<Matcher> m = typePattern->matcher(rule);
    if (!m->find()) {
      throw invalid_argument(L"Invalid Mapping Rule : [" + rule + L"]");
    }
    wstring lhs = parseString(m->group(1)->trim());
    optional<char> rhs = parseType(m->group(2)->trim());
    if (lhs.length() != 1) {
      throw invalid_argument(L"Invalid Mapping Rule : [" + rule +
                             L"]. Only a single character is allowed.");
    }
    if (!rhs) {
      throw invalid_argument(L"Invalid Mapping Rule : [" + rule +
                             L"]. Illegal type.");
    }
    typeMap->put(lhs[0], rhs);
  }

  // ensure the table is always at least as big as DEFAULT_WORD_DELIM_TABLE for
  // performance
  std::deque<char> types(
      max(typeMap->lastKey() + 1,
          WordDelimiterIterator::DEFAULT_WORD_DELIM_TABLE.size()));
  for (int i = 0; i < types.size(); i++) {
    types[i] = WordDelimiterIterator::getType(i);
  }
  for (auto mapping : typeMap) {
    types[mapping.first] = mapping.second;
  }
  return types;
}

optional<char> WordDelimiterGraphFilterFactory::parseType(const wstring &s)
{
  if (s == L"LOWER") {
    return WordDelimiterIterator::LOWER;
  } else if (s == L"UPPER") {
    return WordDelimiterIterator::UPPER;
  } else if (s == L"ALPHA") {
    return WordDelimiterIterator::ALPHA;
  } else if (s == L"DIGIT") {
    return WordDelimiterIterator::DIGIT;
  } else if (s == L"ALPHANUM") {
    return WordDelimiterIterator::ALPHANUM;
  } else if (s == L"SUBWORD_DELIM") {
    return WordDelimiterIterator::SUBWORD_DELIM;
  } else {
    return nullopt;
  }
}

wstring WordDelimiterGraphFilterFactory::parseString(const wstring &s)
{
  int readPos = 0;
  int len = s.length();
  int writePos = 0;
  while (readPos < len) {
    wchar_t c = s[readPos++];
    if (c == L'\\') {
      if (readPos >= len) {
        throw invalid_argument(L"Invalid escaped char in [" + s + L"]");
      }
      c = s[readPos++];
      switch (c) {
      case L'\\':
        c = L'\\';
        break;
      case L'n':
        c = L'\n';
        break;
      case L't':
        c = L'\t';
        break;
      case L'r':
        c = L'\r';
        break;
      case L'b':
        c = L'\b';
        break;
      case L'f':
        c = L'\f';
        break;
      case L'u':
        if (readPos + 3 >= len) {
          throw invalid_argument(L"Invalid escaped char in [" + s + L"]");
        }
        // C++ TODO: Only single-argument parse and valueOf methods are
        // converted: ORIGINAL LINE: c =
        // (char)Integer.parseInt(s.substring(readPos, 4), 16);
        c = static_cast<wchar_t>(Integer::valueOf(s.substr(readPos, 4), 16));
        readPos += 4;
        break;
      }
    }
    out[writePos++] = c;
  }
  return wstring(out, 0, writePos);
}
} // namespace org::apache::lucene::analysis::miscellaneous