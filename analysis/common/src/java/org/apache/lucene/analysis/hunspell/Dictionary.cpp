using namespace std;

#include "Dictionary.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/codecs/CodecUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataOutput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/IOContext.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/IndexOutput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRefHash.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/OfflineSorter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/RamUsageEstimator.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/RegExp.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Builder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/CharSequenceOutputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/IntSequenceOutputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Util.h"
#include "ISO8859_14Decoder.h"

namespace org::apache::lucene::analysis::hunspell
{
using CodecUtil = org::apache::lucene::codecs::CodecUtil;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using CharsRef = org::apache::lucene::util::CharsRef;
using IOUtils = org::apache::lucene::util::IOUtils;
using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using ByteSequencesReader =
    org::apache::lucene::util::OfflineSorter::ByteSequencesReader;
using ByteSequencesWriter =
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using RegExp = org::apache::lucene::util::automaton::RegExp;
using Builder = org::apache::lucene::util::fst::Builder;
using CharSequenceOutputs = org::apache::lucene::util::fst::CharSequenceOutputs;
using FST = org::apache::lucene::util::fst::FST;
using IntSequenceOutputs = org::apache::lucene::util::fst::IntSequenceOutputs;
using Outputs = org::apache::lucene::util::fst::Outputs;
using Util = org::apache::lucene::util::fst::Util;
std::deque<wchar_t> const Dictionary::NOFLAGS = std::deque<wchar_t>(0);
const wstring Dictionary::ALIAS_KEY = L"AF";
const wstring Dictionary::MORPH_ALIAS_KEY = L"AM";
const wstring Dictionary::PREFIX_KEY = L"PFX";
const wstring Dictionary::SUFFIX_KEY = L"SFX";
const wstring Dictionary::FLAG_KEY = L"FLAG";
const wstring Dictionary::COMPLEXPREFIXES_KEY = L"COMPLEXPREFIXES";
const wstring Dictionary::CIRCUMFIX_KEY = L"CIRCUMFIX";
const wstring Dictionary::IGNORE_KEY = L"IGNORE";
const wstring Dictionary::ICONV_KEY = L"ICONV";
const wstring Dictionary::OCONV_KEY = L"OCONV";
const wstring Dictionary::FULLSTRIP_KEY = L"FULLSTRIP";
const wstring Dictionary::LANG_KEY = L"LANG";
const wstring Dictionary::KEEPCASE_KEY = L"KEEPCASE";
const wstring Dictionary::NEEDAFFIX_KEY = L"NEEDAFFIX";
const wstring Dictionary::PSEUDOROOT_KEY = L"PSEUDOROOT";
const wstring Dictionary::ONLYINCOMPOUND_KEY = L"ONLYINCOMPOUND";
const wstring Dictionary::NUM_FLAG_TYPE = L"num";
const wstring Dictionary::UTF8_FLAG_TYPE = L"UTF-8";
const wstring Dictionary::LONG_FLAG_TYPE = L"long";
const wstring Dictionary::PREFIX_CONDITION_REGEX_PATTERN = L"%s.*";
const wstring Dictionary::SUFFIX_CONDITION_REGEX_PATTERN = L".*%s";

Dictionary::Dictionary(shared_ptr<Directory> tempDir,
                       const wstring &tempFileNamePrefix,
                       shared_ptr<InputStream> affix,
                       shared_ptr<InputStream> dictionary) throw(IOException,
                                                                 ParseException)
    : Dictionary(tempDir, tempFileNamePrefix, affix,
                 Collections::singletonList(dictionary), false)
{
}

Dictionary::Dictionary(shared_ptr<Directory> tempDir,
                       const wstring &tempFileNamePrefix,
                       shared_ptr<InputStream> affix,
                       deque<std::shared_ptr<InputStream>> &dictionaries,
                       bool ignoreCase) 
{
  this->ignoreCase = ignoreCase;
  this->needsInputCleaning = ignoreCase;
  this->needsOutputCleaning = false;        // set if we have an OCONV
  flagLookup->add(make_shared<BytesRef>()); // no flags -> ord 0

  shared_ptr<Path> aff = Files::createTempFile(tempPath, L"affix", L"aff");
  shared_ptr<OutputStream> out =
      make_shared<BufferedOutputStream>(Files::newOutputStream(aff));
  shared_ptr<InputStream> aff1 = nullptr;
  shared_ptr<InputStream> aff2 = nullptr;
  bool success = false;
  try {
    // copy contents of affix stream to temp file
    const std::deque<char> buffer = std::deque<char>(1024 * 8);
    int len;
    while ((len = affix->read(buffer)) > 0) {
      out->write(buffer, 0, len);
    }
    out->close();

    // pass 1: get encoding
    aff1 = make_shared<BufferedInputStream>(Files::newInputStream(aff));
    wstring encoding = getDictionaryEncoding(aff1);

    // pass 2: parse affixes
    shared_ptr<CharsetDecoder> decoder = getJavaEncoding(encoding);
    aff2 = make_shared<BufferedInputStream>(Files::newInputStream(aff));
    readAffixFile(aff2, decoder);

    // read dictionary entries
    shared_ptr<IntSequenceOutputs> o = IntSequenceOutputs::getSingleton();
    shared_ptr<Builder<std::shared_ptr<IntsRef>>> b =
        make_shared<Builder<std::shared_ptr<IntsRef>>>(FST::INPUT_TYPE::BYTE4,
                                                       o);
    readDictionaryFiles(tempDir, tempFileNamePrefix, dictionaries, decoder, b);
    words = b->finish();
    aliases.clear();      // no longer needed
    morphAliases.clear(); // no longer needed
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({out, aff1, aff2});
    if (success) {
      Files::delete (aff);
    } else {
      IOUtils::deleteFilesIgnoringExceptions({aff});
    }
  }
}

shared_ptr<IntsRef> Dictionary::lookupWord(std::deque<wchar_t> &word,
                                           int offset, int length)
{
  return lookup(words, word, offset, length);
}

shared_ptr<IntsRef> Dictionary::lookupPrefix(std::deque<wchar_t> &word,
                                             int offset, int length)
{
  return lookup(prefixes, word, offset, length);
}

shared_ptr<IntsRef> Dictionary::lookupSuffix(std::deque<wchar_t> &word,
                                             int offset, int length)
{
  return lookup(suffixes, word, offset, length);
}

shared_ptr<IntsRef>
Dictionary::lookup(shared_ptr<FST<std::shared_ptr<IntsRef>>> fst,
                   std::deque<wchar_t> &word, int offset, int length)
{
  if (fst == nullptr) {
    return nullptr;
  }
  shared_ptr<FST::BytesReader> *const bytesReader = fst->getBytesReader();
  shared_ptr<FST::Arc<std::shared_ptr<IntsRef>>> *const arc =
      fst->getFirstArc(make_shared<FST::Arc<std::shared_ptr<IntsRef>>>());
  // Accumulate output as we go
  shared_ptr<IntsRef> *const NO_OUTPUT = fst->outputs->getNoOutput();
  shared_ptr<IntsRef> output = NO_OUTPUT;

  int l = offset + length;
  try {
    for (int i = offset, cp = 0; i < l; i += Character::charCount(cp)) {
      cp = Character::codePointAt(word, i, l);
      if (fst->findTargetArc(cp, arc, arc, bytesReader) == nullptr) {
        return nullptr;
      } else if (arc->output != NO_OUTPUT) {
        output = fst->outputs->add(output, arc->output);
      }
    }
    if (fst->findTargetArc(FST::END_LABEL, arc, arc, bytesReader) == nullptr) {
      return nullptr;
    } else if (arc->output != NO_OUTPUT) {
      return fst->outputs->add(output, arc->output);
    } else {
      return output;
    }
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
}

void Dictionary::readAffixFile(
    shared_ptr<InputStream> affixStream,
    shared_ptr<CharsetDecoder> decoder) 
{
  map_obj<wstring, deque<int>> prefixes = map_obj<wstring, deque<int>>();
  map_obj<wstring, deque<int>> suffixes = map_obj<wstring, deque<int>>();
  unordered_map<wstring, int> seenPatterns = unordered_map<wstring, int>();

  // zero condition -> 0 ord
  seenPatterns.emplace(L".*", 0);
  patterns.push_back(nullptr);

  // zero strip -> 0 ord
  unordered_map<wstring, int> seenStrips =
      make_shared<LinkedHashMap<wstring, int>>();
  seenStrips.emplace(L"", 0);

  shared_ptr<LineNumberReader> reader = make_shared<LineNumberReader>(
      make_shared<InputStreamReader>(affixStream, decoder));
  wstring line = L"";
  while ((line = reader->readLine()) != L"") {
    // ignore any BOM marker on first line
    if (reader->getLineNumber() == 1 &&
        StringHelper::startsWith(line, L"\uFEFF")) {
      line = line.substr(1);
    }
    if (StringHelper::startsWith(line, ALIAS_KEY)) {
      parseAlias(line);
    } else if (StringHelper::startsWith(line, MORPH_ALIAS_KEY)) {
      parseMorphAlias(line);
    } else if (StringHelper::startsWith(line, PREFIX_KEY)) {
      parseAffix(prefixes, line, reader, PREFIX_CONDITION_REGEX_PATTERN,
                 seenPatterns, seenStrips);
    } else if (StringHelper::startsWith(line, SUFFIX_KEY)) {
      parseAffix(suffixes, line, reader, SUFFIX_CONDITION_REGEX_PATTERN,
                 seenPatterns, seenStrips);
    } else if (StringHelper::startsWith(line, FLAG_KEY)) {
      // Assume that the FLAG line comes before any prefix or suffixes
      // Store the strategy so it can be used when parsing the dic file
      flagParsingStrategy = getFlagParsingStrategy(line);
    } else if (line == COMPLEXPREFIXES_KEY) {
      complexPrefixes = true; // 2-stage prefix+1-stage suffix instead of
                              // 2-stage suffix+1-stage prefix
    } else if (StringHelper::startsWith(line, CIRCUMFIX_KEY)) {
      std::deque<wstring> parts = line.split(L"\\s+");
      if (parts.size() != 2) {
        throw make_shared<ParseException>(L"Illegal CIRCUMFIX declaration",
                                          reader->getLineNumber());
      }
      circumfix = flagParsingStrategy->parseFlag(parts[1]);
    } else if (StringHelper::startsWith(line, KEEPCASE_KEY)) {
      std::deque<wstring> parts = line.split(L"\\s+");
      if (parts.size() != 2) {
        throw make_shared<ParseException>(L"Illegal KEEPCASE declaration",
                                          reader->getLineNumber());
      }
      keepcase = flagParsingStrategy->parseFlag(parts[1]);
    } else if (StringHelper::startsWith(line, NEEDAFFIX_KEY) ||
               StringHelper::startsWith(line, PSEUDOROOT_KEY)) {
      std::deque<wstring> parts = line.split(L"\\s+");
      if (parts.size() != 2) {
        throw make_shared<ParseException>(L"Illegal NEEDAFFIX declaration",
                                          reader->getLineNumber());
      }
      needaffix = flagParsingStrategy->parseFlag(parts[1]);
    } else if (StringHelper::startsWith(line, ONLYINCOMPOUND_KEY)) {
      std::deque<wstring> parts = line.split(L"\\s+");
      if (parts.size() != 2) {
        throw make_shared<ParseException>(L"Illegal ONLYINCOMPOUND declaration",
                                          reader->getLineNumber());
      }
      onlyincompound = flagParsingStrategy->parseFlag(parts[1]);
    } else if (StringHelper::startsWith(line, IGNORE_KEY)) {
      std::deque<wstring> parts = line.split(L"\\s+");
      if (parts.size() != 2) {
        throw make_shared<ParseException>(L"Illegal IGNORE declaration",
                                          reader->getLineNumber());
      }
      ignore = parts[1].toCharArray();
      Arrays::sort(ignore);
      needsInputCleaning = true;
    } else if (StringHelper::startsWith(line, ICONV_KEY) ||
               StringHelper::startsWith(line, OCONV_KEY)) {
      std::deque<wstring> parts = line.split(L"\\s+");
      wstring type = parts[0];
      if (parts.size() != 2) {
        throw make_shared<ParseException>(L"Illegal " + type + L" declaration",
                                          reader->getLineNumber());
      }
      int num = stoi(parts[1]);
      shared_ptr<FST<std::shared_ptr<CharsRef>>> res =
          parseConversions(reader, num);
      if (type == L"ICONV") {
        iconv = res;
        needsInputCleaning |= iconv != nullptr;
      } else {
        oconv = res;
        needsOutputCleaning |= oconv != nullptr;
      }
    } else if (StringHelper::startsWith(line, FULLSTRIP_KEY)) {
      fullStrip = true;
    } else if (StringHelper::startsWith(line, LANG_KEY)) {
      language = line.substr(LANG_KEY.length())->trim();
      alternateCasing = L"tr_TR" == language || L"az_AZ" == language;
    }
  }

  this->prefixes = affixFST(prefixes);
  this->suffixes = affixFST(suffixes);

  int totalChars = 0;
  for (auto strip : seenStrips) {
    totalChars += strip.first->length();
  }
  stripData = std::deque<wchar_t>(totalChars);
  stripOffsets = std::deque<int>(seenStrips.size() + 1);
  int currentOffset = 0;
  int currentIndex = 0;
  for (auto strip : seenStrips) {
    stripOffsets[currentIndex++] = currentOffset;
    strip.first::getChars(0, strip.first->length(), stripData, currentOffset);
    currentOffset += strip.first->length();
  }
  assert(currentIndex == seenStrips.size());
  stripOffsets[currentIndex] = currentOffset;
}

shared_ptr<FST<std::shared_ptr<IntsRef>>>
Dictionary::affixFST(map_obj<wstring, deque<int>> &affixes) 
{
  shared_ptr<IntSequenceOutputs> outputs = IntSequenceOutputs::getSingleton();
  shared_ptr<Builder<std::shared_ptr<IntsRef>>> builder =
      make_shared<Builder<std::shared_ptr<IntsRef>>>(FST::INPUT_TYPE::BYTE4,
                                                     outputs);
  shared_ptr<IntsRefBuilder> scratch = make_shared<IntsRefBuilder>();
  for (auto entry : affixes) {
    Util::toUTF32(entry.first, scratch);
    deque<int> entries = entry.second;
    shared_ptr<IntsRef> output = make_shared<IntsRef>(entries.size());
    for (shared_ptr<> : : optional<int> c : entries) {
      output->ints[output->length++] = c;
    }
    builder->add(scratch->get(), output);
  }
  return builder->finish();
}

wstring Dictionary::escapeDash(const wstring &re)
{
  // we have to be careful, even though dash doesn't have a special meaning,
  // some dictionaries already escape it (e.g. pt_PT), so we don't want to
  // nullify it
  shared_ptr<StringBuilder> escaped = make_shared<StringBuilder>();
  for (int i = 0; i < re.length(); i++) {
    wchar_t c = re[i];
    if (c == L'-') {
      escaped->append(L"\\-");
    } else {
      escaped->append(c);
      if (c == L'\\' && i + 1 < re.length()) {
        escaped->append(re[i + 1]);
        i++;
      }
    }
  }
  return escaped->toString();
}

void Dictionary::parseAffix(
    map_obj<wstring, deque<int>> &affixes, const wstring &header,
    shared_ptr<LineNumberReader> reader, const wstring &conditionPattern,
    unordered_map<wstring, int> &seenPatterns,
    unordered_map<wstring, int> &seenStrips) 
{

  shared_ptr<BytesRefBuilder> scratch = make_shared<BytesRefBuilder>();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  std::deque<wstring> args = header.split(L"\\s+");

  bool crossProduct = args[2] == L"Y";
  bool isSuffix = conditionPattern == SUFFIX_CONDITION_REGEX_PATTERN;

  int numLines = stoi(args[3]);
  affixData = ArrayUtil::grow(affixData, (currentAffix << 3) + (numLines << 3));
  shared_ptr<ByteArrayDataOutput> affixWriter =
      make_shared<ByteArrayDataOutput>(affixData, currentAffix << 3,
                                       numLines << 3);

  for (int i = 0; i < numLines; i++) {
    assert(affixWriter->getPosition() == currentAffix << 3);
    wstring line = reader->readLine();
    std::deque<wstring> ruleArgs = line.split(L"\\s+");

    // from the manpage: PFX flag stripping prefix [condition
    // [morphological_fields...]] condition is optional
    if (ruleArgs.size() < 4) {
      throw make_shared<ParseException>(
          L"The affix file contains a rule with less than four elements: " +
              line,
          reader->getLineNumber());
    }

    wchar_t flag = flagParsingStrategy->parseFlag(ruleArgs[1]);
    wstring strip = ruleArgs[2] == L"0" ? L"" : ruleArgs[2];
    wstring affixArg = ruleArgs[3];
    std::deque<wchar_t> appendFlags;

    // first: parse continuation classes out of affix
    int flagSep = (int)affixArg.rfind(L'/');
    if (flagSep != -1) {
      wstring flagPart = affixArg.substr(flagSep + 1);
      affixArg = affixArg.substr(0, flagSep);

      if (aliasCount > 0) {
        flagPart = getAliasValue(stoi(flagPart));
      }

      appendFlags = flagParsingStrategy->parseFlags(flagPart);
      Arrays::sort(appendFlags);
      twoStageAffix = true;
    }
    // zero affix -> empty string
    if (L"0" == affixArg) {
      affixArg = L"";
    }

    wstring condition = ruleArgs.size() > 4 ? ruleArgs[4] : L".";
    // at least the gascon affix file has this issue
    if (StringHelper::startsWith(condition, L"[") &&
        condition.find(L']') == wstring::npos) {
      condition = condition + L"]";
    }
    // "dash hasn't got special meaning" (we must escape it)
    if (condition.find(L'-') != wstring::npos) {
      condition = escapeDash(condition);
    }

    const wstring regex;
    if (L"." == condition) {
      regex = L".*"; // Zero condition is indicated by dot
    } else if (condition == strip) {
      regex =
          L".*"; // TODO: optimize this better:
                 // if we remove 'strip' from condition, we don't have to append
                 // 'strip' to check it...! but this is complicated...
    } else {
      regex = wstring::format(Locale::ROOT, conditionPattern, condition);
    }

    // deduplicate patterns
    optional<int> patternIndex = seenPatterns[regex];
    if (!patternIndex) {
      patternIndex = patterns.size();
      if (patternIndex > numeric_limits<short>::max()) {
        throw make_shared<UnsupportedOperationException>(
            L"Too many patterns, please report this to dev@lucene.apache.org");
      }
      seenPatterns.emplace(regex, patternIndex);
      shared_ptr<CharacterRunAutomaton> pattern =
          make_shared<CharacterRunAutomaton>(
              (make_shared<RegExp>(regex, RegExp::NONE))->toAutomaton());
      patterns.push_back(pattern);
    }

    optional<int> stripOrd = seenStrips[strip];
    if (!stripOrd) {
      stripOrd = seenStrips.size();
      seenStrips.emplace(strip, stripOrd);
      if (stripOrd > numeric_limits<wchar_t>::max()) {
        throw make_shared<UnsupportedOperationException>(
            L"Too many unique strips, please report this to "
            L"dev@lucene.apache.org");
      }
    }

    if (appendFlags.empty()) {
      appendFlags = NOFLAGS;
    }

    encodeFlags(scratch, appendFlags);
    int appendFlagsOrd = flagLookup->add(scratch->get());
    if (appendFlagsOrd < 0) {
      // already exists in our hash
      appendFlagsOrd = (-appendFlagsOrd) - 1;
    } else if (appendFlagsOrd > numeric_limits<short>::max()) {
      // this limit is probably flexible, but it's a good sanity check too
      throw make_shared<UnsupportedOperationException>(
          L"Too many unique append flags, please report this to "
          L"dev@lucene.apache.org");
    }

    affixWriter->writeShort(static_cast<short>(flag));
    affixWriter->writeShort(static_cast<short>(stripOrd.value()));
    // encode crossProduct into patternIndex
    int patternOrd = patternIndex.value() << 1 | (crossProduct ? 1 : 0);
    affixWriter->writeShort(static_cast<short>(patternOrd));
    affixWriter->writeShort(static_cast<short>(appendFlagsOrd));

    if (needsInputCleaning) {
      shared_ptr<std::wstring> cleaned = cleanInput(affixArg, sb);
      // C++ TODO: There is no native C++ equivalent to 'toString':
      affixArg = cleaned->toString();
    }

    if (isSuffix) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      affixArg = (make_shared<StringBuilder>(affixArg))->reverse()->toString();
    }

    deque<int> deque = affixes[affixArg];
    if (deque.empty()) {
      deque = deque<>();
      affixes.emplace(affixArg, deque);
    }
    deque.push_back(currentAffix);
    currentAffix++;
  }
}

shared_ptr<FST<std::shared_ptr<CharsRef>>>
Dictionary::parseConversions(shared_ptr<LineNumberReader> reader,
                             int num) 
{
  unordered_map<wstring, wstring> mappings = map_obj<wstring, wstring>();

  for (int i = 0; i < num; i++) {
    wstring line = reader->readLine();
    std::deque<wstring> parts = line.split(L"\\s+");
    if (parts.size() != 3) {
      throw make_shared<ParseException>(L"invalid syntax: " + line,
                                        reader->getLineNumber());
    }
    if (mappings.emplace(parts[1], parts[2]) != nullptr) {
      throw make_shared<IllegalStateException>(
          L"duplicate mapping specified for: " + parts[1]);
    }
  }

  shared_ptr<Outputs<std::shared_ptr<CharsRef>>> outputs =
      CharSequenceOutputs::getSingleton();
  shared_ptr<Builder<std::shared_ptr<CharsRef>>> builder =
      make_shared<Builder<std::shared_ptr<CharsRef>>>(FST::INPUT_TYPE::BYTE2,
                                                      outputs);
  shared_ptr<IntsRefBuilder> scratchInts = make_shared<IntsRefBuilder>();
  for (auto entry : mappings) {
    Util::toUTF16(entry.first, scratchInts);
    builder->add(scratchInts->get(), make_shared<CharsRef>(entry.second));
  }

  return builder->finish();
}

const shared_ptr<java::util::regex::Pattern> Dictionary::ENCODING_PATTERN =
    java::util::regex::Pattern::compile(L"^(\u00EF\u00BB\u00BF)?SET\\s+");

wstring Dictionary::getDictionaryEncoding(shared_ptr<InputStream> affix) throw(
    IOException, ParseException)
{
  shared_ptr<StringBuilder> *const encoding = make_shared<StringBuilder>();
  for (;;) {
    encoding->setLength(0);
    int ch;
    while ((ch = affix->read()) >= 0) {
      if (ch == L'\n') {
        break;
      }
      if (ch != L'\r') {
        encoding->append(static_cast<wchar_t>(ch));
      }
    }
    if (encoding->length() == 0 || encoding->charAt(0) == L'#' ||
        encoding->toString()->trim()->length() == 0) {
      if (ch < 0) {
        throw make_shared<ParseException>(L"Unexpected end of affix file.", 0);
      }
      continue;
    }
    shared_ptr<Matcher> matcher = ENCODING_PATTERN->matcher(encoding);
    if (matcher->find()) {
      int last = matcher->end();
      return encoding->substr(last)->trim();
    }
  }
}

const unordered_map<wstring, wstring> Dictionary::CHARSET_ALIASES;

Dictionary::StaticConstructor::StaticConstructor()
{
  unordered_map<wstring, wstring> m = unordered_map<wstring, wstring>();
  m.emplace(L"microsoft-cp1251", L"windows-1251");
  m.emplace(L"TIS620-2533", L"TIS-620");
  CHARSET_ALIASES = m;
}

Dictionary::StaticConstructor Dictionary::staticConstructor;

shared_ptr<CharsetDecoder> Dictionary::getJavaEncoding(const wstring &encoding)
{
  if (L"ISO8859-14" == encoding) {
    return make_shared<ISO8859_14Decoder>();
  }
  wstring canon = CHARSET_ALIASES[encoding];
  if (canon != L"") {
    encoding = canon;
  }
  shared_ptr<Charset> charset = Charset::forName(encoding);
  return charset->newDecoder().onMalformedInput(CodingErrorAction::REPLACE);
}

shared_ptr<FlagParsingStrategy>
Dictionary::getFlagParsingStrategy(const wstring &flagLine)
{
  std::deque<wstring> parts = flagLine.split(L"\\s+");
  if (parts.size() != 2) {
    throw invalid_argument(L"Illegal FLAG specification: " + flagLine);
  }
  wstring flagType = parts[1];

  if (NUM_FLAG_TYPE == flagType) {
    return make_shared<NumFlagParsingStrategy>();
  } else if (UTF8_FLAG_TYPE == flagType) {
    return make_shared<SimpleFlagParsingStrategy>();
  } else if (LONG_FLAG_TYPE == flagType) {
    return make_shared<DoubleASCIIFlagParsingStrategy>();
  }

  throw invalid_argument(L"Unknown flag type: " + flagType);
}

wstring Dictionary::unescapeEntry(const wstring &entry)
{
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int end = morphBoundary(entry);
  for (int i = 0; i < end; i++) {
    wchar_t ch = entry[i];
    if (ch == L'\\' && i + 1 < entry.length()) {
      sb->append(entry[i + 1]);
      i++;
    } else if (ch == L'/') {
      sb->append(FLAG_SEPARATOR);
    } else if (ch == MORPH_SEPARATOR || ch == FLAG_SEPARATOR) {
      // BINARY EXECUTABLES EMBEDDED IN ZULU DICTIONARIES!!!!!!!
    } else {
      sb->append(ch);
    }
  }
  sb->append(MORPH_SEPARATOR);
  if (end < entry.length()) {
    for (int i = end; i < entry.length(); i++) {
      wchar_t c = entry[i];
      if (c == FLAG_SEPARATOR || c == MORPH_SEPARATOR) {
        // BINARY EXECUTABLES EMBEDDED IN ZULU DICTIONARIES!!!!!!!
      } else {
        sb->append(c);
      }
    }
  }
  return sb->toString();
}

int Dictionary::morphBoundary(const wstring &line)
{
  int end = indexOfSpaceOrTab(line, 0);
  if (end == -1) {
    return line.length();
  }
  while (end >= 0 && end < line.length()) {
    if (line[end] == L'\t' ||
        end + 3 < line.length() && isalpha(line[end + 1]) &&
            isalpha(line[end + 2]) && line[end + 3] == L':') {
      break;
    }
    end = indexOfSpaceOrTab(line, end + 1);
  }
  if (end == -1) {
    return line.length();
  }
  return end;
}

int Dictionary::indexOfSpaceOrTab(const wstring &text, int start)
{
  int pos1 = (int)text.find(L'\t', start);
  int pos2 = (int)text.find(L' ', start);
  if (pos1 >= 0 && pos2 >= 0) {
    return min(pos1, pos2);
  } else {
    return max(pos1, pos2);
  }
}

void Dictionary::readDictionaryFiles(
    shared_ptr<Directory> tempDir, const wstring &tempFileNamePrefix,
    deque<std::shared_ptr<InputStream>> &dictionaries,
    shared_ptr<CharsetDecoder> decoder,
    shared_ptr<Builder<std::shared_ptr<IntsRef>>> words) 
{
  shared_ptr<BytesRefBuilder> flagsScratch = make_shared<BytesRefBuilder>();
  shared_ptr<IntsRefBuilder> scratchInts = make_shared<IntsRefBuilder>();

  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();

  shared_ptr<IndexOutput> unsorted =
      tempDir->createTempOutput(tempFileNamePrefix, L"dat", IOContext::DEFAULT);
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.util.OfflineSorter.ByteSequencesWriter writer = new
  // org.apache.lucene.util.OfflineSorter.ByteSequencesWriter(unsorted))
  {
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter writer =
        org::apache::lucene::util::OfflineSorter::ByteSequencesWriter(unsorted);
    for (auto dictionary : dictionaries) {
      shared_ptr<BufferedReader> lines = make_shared<BufferedReader>(
          make_shared<InputStreamReader>(dictionary, decoder));
      wstring line = lines->readLine(); // first line is number of entries
                                        // (approximately, sometimes)

      while ((line = lines->readLine()) != L"") {
        // wild and unpredictable code comment rules
        if (line.isEmpty() || line[0] == L'/' || line[0] == L'#' ||
            line[0] == L'\t') {
          continue;
        }
        line = unescapeEntry(line);
        // if we havent seen any stem exceptions, try to parse one
        if (hasStemExceptions == false) {
          int morphStart = (int)line.find(MORPH_SEPARATOR);
          if (morphStart >= 0 && morphStart < line.length()) {
            hasStemExceptions =
                parseStemException(line.substr(morphStart + 1)) != L"";
          }
        }
        if (needsInputCleaning) {
          int flagSep = (int)line.find(FLAG_SEPARATOR);
          if (flagSep == -1) {
            flagSep = (int)line.find(MORPH_SEPARATOR);
          }
          if (flagSep == -1) {
            shared_ptr<std::wstring> cleansed = cleanInput(line, sb);
            // C++ TODO: There is no native C++ equivalent to 'toString':
            writer->write(
                cleansed->toString()->getBytes(StandardCharsets::UTF_8));
          } else {
            wstring text = line.substr(0, flagSep);
            shared_ptr<std::wstring> cleansed = cleanInput(text, sb);
            if (cleansed != sb) {
              sb->setLength(0);
              sb->append(cleansed);
            }
            sb->append(line.substr(flagSep));
            writer->write(sb->toString().getBytes(StandardCharsets::UTF_8));
          }
        } else {
          writer->write(line.getBytes(StandardCharsets::UTF_8));
        }
      }
    }
    CodecUtil::writeFooter(unsorted);
  }

  shared_ptr<OfflineSorter> sorter = make_shared<OfflineSorter>(
      tempDir, tempFileNamePrefix,
      make_shared<ComparatorAnonymousInnerClass>(shared_from_this()));

  wstring sorted;
  bool success = false;
  try {
    sorted = sorter->sort(unsorted->getName());
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      tempDir->deleteFile(unsorted->getName());
    } else {
      IOUtils::deleteFilesIgnoringExceptions(tempDir, {unsorted->getName()});
    }
  }

  bool success2 = false;

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try
  // (org.apache.lucene.util.OfflineSorter.ByteSequencesReader reader = new
  // org.apache.lucene.util.OfflineSorter.ByteSequencesReader(tempDir.openChecksumInput(sorted,
  // org.apache.lucene.store.IOContext.READONCE), sorted))
  {
    org::apache::lucene::util::OfflineSorter::ByteSequencesReader reader =
        org::apache::lucene::util::OfflineSorter::ByteSequencesReader(
            tempDir->openChecksumInput(
                sorted, org::apache::lucene::store::IOContext::READONCE),
            sorted);
    try {

      // TODO: the flags themselves can be double-chars (long) or also numeric
      // either way the trick is to encode them as char... but they must be
      // parsed differently

      wstring currentEntry = L"";
      shared_ptr<IntsRefBuilder> currentOrds = make_shared<IntsRefBuilder>();

      while (true) {
        shared_ptr<BytesRef> scratch = reader->next();
        if (scratch == nullptr) {
          break;
        }

        wstring line = scratch->utf8ToString();
        wstring entry;
        std::deque<wchar_t> wordForm;
        int end;

        int flagSep = (int)line.find(FLAG_SEPARATOR);
        if (flagSep == -1) {
          wordForm = NOFLAGS;
          end = (int)line.find(MORPH_SEPARATOR);
          entry = line.substr(0, end);
        } else {
          end = (int)line.find(MORPH_SEPARATOR);
          wstring flagPart = line.substr(flagSep + 1, end - (flagSep + 1));
          if (aliasCount > 0) {
            flagPart = getAliasValue(stoi(flagPart));
          }

          wordForm = flagParsingStrategy->parseFlags(flagPart);
          Arrays::sort(wordForm);
          entry = line.substr(0, flagSep);
        }
        // we possibly have morphological data
        int stemExceptionID = 0;
        if (hasStemExceptions && end + 1 < line.length()) {
          wstring stemException = parseStemException(line.substr(end + 1));
          if (stemException != L"") {
            if (stemExceptionCount == stemExceptions.size()) {
              int newSize =
                  ArrayUtil::oversize(stemExceptionCount + 1,
                                      RamUsageEstimator::NUM_BYTES_OBJECT_REF);
              stemExceptions = Arrays::copyOf(stemExceptions, newSize);
            }
            stemExceptionID =
                stemExceptionCount +
                1; // we use '0' to indicate no exception for the form
            stemExceptions[stemExceptionCount++] = stemException;
          }
        }

        int cmp = currentEntry == L"" ? 1 : entry.compare(currentEntry);
        if (cmp < 0) {
          throw invalid_argument(L"out of order: " + entry + L" < " +
                                 currentEntry);
        } else {
          encodeFlags(flagsScratch, wordForm);
          int ord = flagLookup->add(flagsScratch->get());
          if (ord < 0) {
            // already exists in our hash
            ord = (-ord) - 1;
          }
          // finalize current entry, and switch "current" if necessary
          if (cmp > 0 && currentEntry != L"") {
            Util::toUTF32(currentEntry, scratchInts);
            words->add(scratchInts->get(), currentOrds->get());
          }
          // swap current
          if (cmp > 0 || currentEntry == L"") {
            currentEntry = entry;
            currentOrds = make_shared<IntsRefBuilder>(); // must be this way
          }
          if (hasStemExceptions) {
            currentOrds->append(ord);
            currentOrds->append(stemExceptionID);
          } else {
            currentOrds->append(ord);
          }
        }
      }

      // finalize last entry
      Util::toUTF32(currentEntry, scratchInts);
      words->add(scratchInts->get(), currentOrds->get());
      success2 = true;
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (success2) {
        tempDir->deleteFile(sorted);
      } else {
        IOUtils::deleteFilesIgnoringExceptions(tempDir, {sorted});
      }
    }
  }
}

Dictionary::ComparatorAnonymousInnerClass::ComparatorAnonymousInnerClass(
    shared_ptr<Dictionary> outerInstance)
{
  this->outerInstance = outerInstance;
  scratch1 = make_shared<BytesRef>();
  scratch2 = make_shared<BytesRef>();
}

int Dictionary::ComparatorAnonymousInnerClass::compare(shared_ptr<BytesRef> o1,
                                                       shared_ptr<BytesRef> o2)
{
  scratch1->bytes = o1->bytes;
  scratch1->offset = o1->offset;
  scratch1->length = o1->length;

  for (int i = scratch1->length - 1; i >= 0; i--) {
    if (scratch1::bytes[scratch1::offset + i] ==
            outerInstance->FLAG_SEPARATOR ||
        scratch1::bytes[scratch1::offset + i] ==
            outerInstance->MORPH_SEPARATOR) {
      scratch1->length = i;
      break;
    }
  }

  scratch2->bytes = o2->bytes;
  scratch2->offset = o2->offset;
  scratch2->length = o2->length;

  for (int i = scratch2->length - 1; i >= 0; i--) {
    if (scratch2::bytes[scratch2::offset + i] ==
            outerInstance->FLAG_SEPARATOR ||
        scratch2::bytes[scratch2::offset + i] ==
            outerInstance->MORPH_SEPARATOR) {
      scratch2->length = i;
      break;
    }
  }

  int cmp = scratch1->compareTo(scratch2);
  if (cmp == 0) {
    // tie break on whole row
    return o1->compareTo(o2);
  } else {
    return cmp;
  }
}

std::deque<wchar_t> Dictionary::decodeFlags(shared_ptr<BytesRef> b)
{
  if (b->length == 0) {
    return CharsRef::EMPTY_CHARS;
  }
  int len = static_cast<int>(static_cast<unsigned int>(b->length) >> 1);
  std::deque<wchar_t> flags(len);
  int upto = 0;
  int end = b->offset + b->length;
  for (int i = b->offset; i < end; i += 2) {
    flags[upto++] =
        static_cast<wchar_t>((b->bytes[i] << 8) | (b->bytes[i + 1] & 0xff));
  }
  return flags;
}

void Dictionary::encodeFlags(shared_ptr<BytesRefBuilder> b,
                             std::deque<wchar_t> &flags)
{
  int len = flags.size() << 1;
  b->grow(len);
  b->clear();
  for (int i = 0; i < flags.size(); i++) {
    int flag = flags[i];
    b->append(static_cast<char>((flag >> 8) & 0xff));
    b->append(static_cast<char>(flag & 0xff));
  }
}

void Dictionary::parseAlias(const wstring &line)
{
  std::deque<wstring> ruleArgs = line.split(L"\\s+");
  if (aliases.empty()) {
    // first line should be the aliases count
    constexpr int count = stoi(ruleArgs[1]);
    aliases = std::deque<wstring>(count);
  } else {
    // an alias can map_obj to no flags
    wstring aliasValue = ruleArgs.size() == 1 ? L"" : ruleArgs[1];
    aliases[aliasCount++] = aliasValue;
  }
}

wstring Dictionary::getAliasValue(int id)
{
  try {
    return aliases[id - 1];
  } catch (const out_of_range &ex) {
    // C++ TODO: This exception's constructor requires only one argument:
    // ORIGINAL LINE: throw new IllegalArgumentException("Bad flag alias
    // number:" + id, ex);
    throw invalid_argument(L"Bad flag alias number:" + to_wstring(id));
  }
}

wstring Dictionary::getStemException(int id) { return stemExceptions[id - 1]; }

void Dictionary::parseMorphAlias(const wstring &line)
{
  if (morphAliases.empty()) {
    // first line should be the aliases count
    constexpr int count = stoi(line.substr(3));
    morphAliases = std::deque<wstring>(count);
  } else {
    wstring arg = line.substr(2); // leave the space
    morphAliases[morphAliasCount++] = arg;
  }
}

wstring Dictionary::parseStemException(const wstring &morphData)
{
  // first see if it's an alias
  if (morphAliasCount > 0) {
    try {
      int alias = static_cast<Integer>(StringHelper::trim(morphData));
      morphData = morphAliases[alias - 1];
    } catch (const NumberFormatException &e) {
      // fine
    }
  }
  // try to parse morph entry
  int index = (int)morphData.find(L" st:");
  if (index < 0) {
    index = (int)morphData.find(L"\tst:");
  }
  if (index >= 0) {
    int endIndex = indexOfSpaceOrTab(morphData, index + 1);
    if (endIndex < 0) {
      endIndex = morphData.length();
    }
    return morphData.substr(index + 4, endIndex - (index + 4));
  }
  return L"";
}

wchar_t Dictionary::FlagParsingStrategy::parseFlag(const wstring &rawFlag)
{
  std::deque<wchar_t> flags = parseFlags(rawFlag);
  if (flags.size() != 1) {
    throw invalid_argument(L"expected only one flag, got: " + rawFlag);
  }
  return flags[0];
}

std::deque<wchar_t>
Dictionary::SimpleFlagParsingStrategy::parseFlags(const wstring &rawFlags)
{
  return rawFlags.toCharArray();
}

std::deque<wchar_t>
Dictionary::NumFlagParsingStrategy::parseFlags(const wstring &rawFlags)
{
  std::deque<wstring> rawFlagParts = StringHelper::trim(rawFlags)->split(L",");
  std::deque<wchar_t> flags(rawFlagParts.size());
  int upto = 0;

  for (int i = 0; i < rawFlagParts.size(); i++) {
    // note, removing the trailing X/leading I for nepali... what is the rule
    // here?!
    wstring replacement = rawFlagParts[i].replaceAll(L"[^0-9]", L"");
    // note, ignoring empty flags (this happens in danish, for example)
    if (replacement.isEmpty()) {
      continue;
    }
    flags[upto++] = static_cast<wchar_t>(stoi(replacement));
  }

  if (upto < flags.size()) {
    flags = Arrays::copyOf(flags, upto);
  }
  return flags;
}

std::deque<wchar_t>
Dictionary::DoubleASCIIFlagParsingStrategy::parseFlags(const wstring &rawFlags)
{
  if (rawFlags.length() == 0) {
    return std::deque<wchar_t>(0);
  }

  shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
  if (rawFlags.length() % 2 == 1) {
    throw invalid_argument(
        L"Invalid flags (should be even number of characters): " + rawFlags);
  }
  for (int i = 0; i < rawFlags.length(); i += 2) {
    wchar_t f1 = rawFlags[i];
    wchar_t f2 = rawFlags[i + 1];
    if (f1 >= 256 || f2 >= 256) {
      throw invalid_argument(
          L"Invalid flags (LONG flags must be double ASCII): " + rawFlags);
    }
    wchar_t combined = static_cast<wchar_t>(f1 << 8 | f2);
    builder->append(combined);
  }

  std::deque<wchar_t> flags(builder->length());
  builder->getChars(0, builder->length(), flags, 0);
  return flags;
}

bool Dictionary::hasFlag(std::deque<wchar_t> &flags, wchar_t flag)
{
  return Arrays::binarySearch(flags, flag) >= 0;
}

shared_ptr<std::wstring> Dictionary::cleanInput(shared_ptr<std::wstring> input,
                                                shared_ptr<StringBuilder> reuse)
{
  reuse->setLength(0);

  for (int i = 0; i < input->length(); i++) {
    wchar_t ch = input->charAt(i);

    if (ignore.size() > 0 && Arrays::binarySearch(ignore, ch) >= 0) {
      continue;
    }

    if (ignoreCase && iconv == nullptr) {
      // if we have no input conversion mappings, do this on-the-fly
      ch = caseFold(ch);
    }

    reuse->append(ch);
  }

  if (iconv != nullptr) {
    try {
      applyMappings(iconv, reuse);
    } catch (const IOException &bogus) {
      throw runtime_error(bogus);
    }
    if (ignoreCase) {
      for (int i = 0; i < reuse->length(); i++) {
        reuse->setCharAt(i, caseFold(reuse->charAt(i)));
      }
    }
  }

  return reuse;
}

wchar_t Dictionary::caseFold(wchar_t c)
{
  if (alternateCasing) {
    if (c == L'I') {
      return L'ı';
    } else if (c == L'İ') {
      return L'i';
    } else {
      return towlower(c);
    }
  } else {
    return towlower(c);
  }
}

void Dictionary::applyMappings(shared_ptr<FST<std::shared_ptr<CharsRef>>> fst,
                               shared_ptr<StringBuilder> sb) 
{
  shared_ptr<FST::BytesReader> *const bytesReader = fst->getBytesReader();
  shared_ptr<FST::Arc<std::shared_ptr<CharsRef>>> *const firstArc =
      fst->getFirstArc(make_shared<FST::Arc<std::shared_ptr<CharsRef>>>());
  shared_ptr<CharsRef> *const NO_OUTPUT = fst->outputs->getNoOutput();

  // temporary stuff
  shared_ptr<FST::Arc<std::shared_ptr<CharsRef>>> *const arc =
      make_shared<FST::Arc<std::shared_ptr<CharsRef>>>();
  int longestMatch;
  shared_ptr<CharsRef> longestOutput;

  for (int i = 0; i < sb->length(); i++) {
    arc->copyFrom(firstArc);
    shared_ptr<CharsRef> output = NO_OUTPUT;
    longestMatch = -1;
    longestOutput.reset();

    for (int j = i; j < sb->length(); j++) {
      wchar_t ch = sb->charAt(j);
      if (fst->findTargetArc(ch, arc, arc, bytesReader) == nullptr) {
        break;
      } else {
        output = fst->outputs->add(output, arc->output);
      }
      if (arc->isFinal()) {
        longestOutput = fst->outputs->add(output, arc->nextFinalOutput);
        longestMatch = j;
      }
    }

    if (longestMatch >= 0) {
      sb->remove(i, longestMatch + 1);
      sb->insert(i, longestOutput);
      i += (longestOutput->length_ - 1);
    }
  }
}

bool Dictionary::getIgnoreCase() { return ignoreCase; }

shared_ptr<java::nio::file::Path> Dictionary::DEFAULT_TEMP_DIR;

void Dictionary::setDefaultTempDir(shared_ptr<Path> tempDir)
{
  DEFAULT_TEMP_DIR = tempDir;
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Path> Dictionary::getDefaultTempDir() 
{
  if (DEFAULT_TEMP_DIR == nullptr) {
    // Lazy init
    wstring tempDirPath = System::getProperty(L"java.io.tmpdir");
    if (tempDirPath == L"") {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(
          L"Java has no temporary folder property (java.io.tmpdir)?");
    }
    shared_ptr<Path> tempDirectory = Paths->get(tempDirPath);
    if (Files::isWritable(tempDirectory) == false) {
      // C++ TODO: The following line could not be converted:
      throw java.io.IOException(
          L"Java's temporary folder not present or writeable?: " +
          tempDirectory.toAbsolutePath());
    }
    DEFAULT_TEMP_DIR = tempDirectory;
  }

  return DEFAULT_TEMP_DIR;
}
} // namespace org::apache::lucene::analysis::hunspell