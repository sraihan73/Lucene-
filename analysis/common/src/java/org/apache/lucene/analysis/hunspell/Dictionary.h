#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <limits>
#include <map_obj>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class IntsRef;
}

namespace org::apache::lucene::util::automaton
{
class CharacterRunAutomaton;
}
namespace org::apache::lucene::util
{
class BytesRefHash;
}
namespace org::apache::lucene::analysis::hunspell
{
class FlagParsingStrategy;
}
namespace org::apache::lucene::util
{
class CharsRef;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class Builder;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::analysis::hunspell
{

using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using CharsRef = org::apache::lucene::util::CharsRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using Builder = org::apache::lucene::util::fst::Builder;
using FST = org::apache::lucene::util::fst::FST;

/**
 * In-memory structure for the dictionary (.dic) and affix (.aff)
 * data of a hunspell dictionary.
 */
class Dictionary : public std::enable_shared_from_this<Dictionary>
{
  GET_CLASS_NAME(Dictionary)

public:
  static std::deque<wchar_t> const NOFLAGS;

private:
  static const std::wstring ALIAS_KEY;
  static const std::wstring MORPH_ALIAS_KEY;
  static const std::wstring PREFIX_KEY;
  static const std::wstring SUFFIX_KEY;
  static const std::wstring FLAG_KEY;
  static const std::wstring COMPLEXPREFIXES_KEY;
  static const std::wstring CIRCUMFIX_KEY;
  static const std::wstring IGNORE_KEY;
  static const std::wstring ICONV_KEY;
  static const std::wstring OCONV_KEY;
  static const std::wstring FULLSTRIP_KEY;
  static const std::wstring LANG_KEY;
  static const std::wstring KEEPCASE_KEY;
  static const std::wstring NEEDAFFIX_KEY;
  static const std::wstring PSEUDOROOT_KEY;
  static const std::wstring ONLYINCOMPOUND_KEY;

  static const std::wstring NUM_FLAG_TYPE;
  static const std::wstring UTF8_FLAG_TYPE;
  static const std::wstring LONG_FLAG_TYPE;

  // TODO: really for suffixes we should reverse the automaton and run them
  // backwards
  static const std::wstring PREFIX_CONDITION_REGEX_PATTERN;
  static const std::wstring SUFFIX_CONDITION_REGEX_PATTERN;

public:
  std::shared_ptr<FST<std::shared_ptr<IntsRef>>> prefixes;
  std::shared_ptr<FST<std::shared_ptr<IntsRef>>> suffixes;

  // all condition checks used by prefixes and suffixes. these are typically
  // re-used across many affix stripping rules. so these are deduplicated, to
  // save RAM.
  std::deque<std::shared_ptr<CharacterRunAutomaton>> patterns =
      std::deque<std::shared_ptr<CharacterRunAutomaton>>();

  // the entries in the .dic file, mapping to their set of flags.
  // the fst output is the ordinal deque for flagLookup
  std::shared_ptr<FST<std::shared_ptr<IntsRef>>> words;
  // the deque of unique flagsets (wordforms). theoretically huge, but
  // practically small (e.g. for polish this is 756), otherwise humans wouldn't
  // be able to deal with it either.
  std::shared_ptr<BytesRefHash> flagLookup = std::make_shared<BytesRefHash>();

  // the deque of unique strip affixes.
  std::deque<wchar_t> stripData;
  std::deque<int> stripOffsets;

  // 8 bytes per affix
  std::deque<char> affixData = std::deque<char>(64);

private:
  int currentAffix = 0;

  std::shared_ptr<FlagParsingStrategy> flagParsingStrategy =
      std::make_shared<SimpleFlagParsingStrategy>(); // Default flag parsing
                                                     // strategy

  // AF entries
  std::deque<std::wstring> aliases;
  int aliasCount = 0;

  // AM entries
  std::deque<std::wstring> morphAliases;
  int morphAliasCount = 0;

  // st: morphological entries (either directly, or aliased from AM)
  std::deque<std::wstring> stemExceptions = std::deque<std::wstring>(8);
  int stemExceptionCount = 0;
  // we set this during sorting, so we know to add an extra FST output.
  // when set, some words have exceptional stems, and the last entry is a
  // pointer to stemExceptions
public:
  bool hasStemExceptions = false;

private:
  const std::shared_ptr<Path> tempPath =
      getDefaultTempDir(); // TODO: make this configurable?

public:
  bool ignoreCase = false;
  bool complexPrefixes = false;
  bool twoStageAffix = false; // if no affixes have continuation classes, no
                              // need to do 2-level affix stripping

  int circumfix = -1;      // circumfix flag, or -1 if one is not defined
  int keepcase = -1;       // keepcase flag, or -1 if one is not defined
  int needaffix = -1;      // needaffix flag, or -1 if one is not defined
  int onlyincompound = -1; // onlyincompound flag, or -1 if one is not defined

  // ignored characters (dictionary, affix, inputs)
private:
  std::deque<wchar_t> ignore;

  // FSTs used for ICONV/OCONV, output ord pointing to replacement text
public:
  std::shared_ptr<FST<std::shared_ptr<CharsRef>>> iconv;
  std::shared_ptr<FST<std::shared_ptr<CharsRef>>> oconv;

  bool needsInputCleaning = false;
  bool needsOutputCleaning = false;

  // true if we can strip suffixes "down to nothing"
  bool fullStrip = false;

  // language declaration of the dictionary
  std::wstring language;
  // true if case algorithms should use alternate (Turkish/Azeri) mapping
  bool alternateCasing = false;

  /**
   * Creates a new Dictionary containing the information read from the provided
   * InputStreams to hunspell affix and dictionary files. You have to close the
   * provided InputStreams yourself.
   *
   * @param tempDir Directory to use for offline sorting
   * @param tempFileNamePrefix prefix to use to generate temp file names
   * @param affix InputStream for reading the hunspell affix file (won't be
   * closed).
   * @param dictionary InputStream for reading the hunspell dictionary file
   * (won't be closed).
   * @throws IOException Can be thrown while reading from the InputStreams
   * @throws ParseException Can be thrown if the content of the files does not
   * meet expected formats
   */
  Dictionary(std::shared_ptr<Directory> tempDir,
             const std::wstring &tempFileNamePrefix,
             std::shared_ptr<InputStream> affix,
             std::shared_ptr<InputStream> dictionary) throw(IOException,
                                                            ParseException);

  /**
   * Creates a new Dictionary containing the information read from the provided
   * InputStreams to hunspell affix and dictionary files. You have to close the
   * provided InputStreams yourself.
   *
   * @param tempDir Directory to use for offline sorting
   * @param tempFileNamePrefix prefix to use to generate temp file names
   * @param affix InputStream for reading the hunspell affix file (won't be
   * closed).
   * @param dictionaries InputStream for reading the hunspell dictionary files
   * (won't be closed).
   * @throws IOException Can be thrown while reading from the InputStreams
   * @throws ParseException Can be thrown if the content of the files does not
   * meet expected formats
   */
  Dictionary(std::shared_ptr<Directory> tempDir,
             const std::wstring &tempFileNamePrefix,
             std::shared_ptr<InputStream> affix,
             std::deque<std::shared_ptr<InputStream>> &dictionaries,
             bool ignoreCase) ;

  /**
   * Looks up Hunspell word forms from the dictionary
   */
  virtual std::shared_ptr<IntsRef> lookupWord(std::deque<wchar_t> &word,
                                              int offset, int length);

  // only for testing
  virtual std::shared_ptr<IntsRef> lookupPrefix(std::deque<wchar_t> &word,
                                                int offset, int length);

  // only for testing
  virtual std::shared_ptr<IntsRef> lookupSuffix(std::deque<wchar_t> &word,
                                                int offset, int length);

  virtual std::shared_ptr<IntsRef>
  lookup(std::shared_ptr<FST<std::shared_ptr<IntsRef>>> fst,
         std::deque<wchar_t> &word, int offset, int length);

  /**
   * Reads the affix file through the provided InputStream, building up the
   * prefix and suffix maps
   *
   * @param affixStream InputStream to read the content of the affix file from
   * @param decoder CharsetDecoder to decode the content of the file
   * @throws IOException Can be thrown while reading from the InputStream
   */
private:
  void
  readAffixFile(std::shared_ptr<InputStream> affixStream,
                std::shared_ptr<CharsetDecoder> decoder) throw(IOException,
                                                               ParseException);

  std::shared_ptr<FST<std::shared_ptr<IntsRef>>> affixFST(
      std::map_obj<std::wstring, std::deque<int>> &affixes) ;

public:
  static std::wstring escapeDash(const std::wstring &re);

  /**
   * Parses a specific affix rule putting the result into the provided affix map_obj
   *
   * @param affixes Map where the result of the parsing will be put
   * @param header Header line of the affix rule
   * @param reader BufferedReader to read the content of the rule from
   * @param conditionPattern {@link std::wstring#format(std::wstring, Object...)} pattern to
   * be used to generate the condition regex pattern
   * @param seenPatterns map_obj from condition -&gt; index of patterns, for
   * deduplication.
   * @throws IOException Can be thrown while reading the rule
   */
private:
  void parseAffix(
      std::map_obj<std::wstring, std::deque<int>> &affixes,
      const std::wstring &header, std::shared_ptr<LineNumberReader> reader,
      const std::wstring &conditionPattern,
      std::unordered_map<std::wstring, int> &seenPatterns,
      std::unordered_map<std::wstring, int> &seenStrips) throw(IOException,
                                                               ParseException);

  std::shared_ptr<FST<std::shared_ptr<CharsRef>>>
  parseConversions(std::shared_ptr<LineNumberReader> reader,
                   int num) ;

  /** pattern accepts optional BOM + SET + any whitespace */
public:
  static const std::shared_ptr<Pattern> ENCODING_PATTERN;

  /**
   * Parses the encoding specified in the affix file readable through the
   * provided InputStream
   *
   * @param affix InputStream for reading the affix file
   * @return Encoding specified in the affix file
   * @throws IOException Can be thrown while reading from the InputStream
   * @throws ParseException Thrown if the first non-empty non-comment line read
   * from the file does not adhere to the format {@code SET <encoding>}
   */
  static std::wstring getDictionaryEncoding(
      std::shared_ptr<InputStream> affix) ;

  static const std::unordered_map<std::wstring, std::wstring> CHARSET_ALIASES;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static Dictionary::StaticConstructor staticConstructor;

  /**
   * Retrieves the CharsetDecoder for the given encoding.  Note, This isn't
   * perfect as I think ISCII-DEVANAGARI and MICROSOFT-CP1251 etc are allowed...
   *
   * @param encoding Encoding to retrieve the CharsetDecoder for
   * @return CharSetDecoder for the given encoding
   */
  std::shared_ptr<CharsetDecoder> getJavaEncoding(const std::wstring &encoding);

  /**
   * Determines the appropriate {@link FlagParsingStrategy} based on the FLAG
   * definition line taken from the affix file
   *
   * @param flagLine Line containing the flag information
   * @return FlagParsingStrategy that handles parsing flags in the way specified
   * in the FLAG definition
   */
public:
  static std::shared_ptr<FlagParsingStrategy>
  getFlagParsingStrategy(const std::wstring &flagLine);

  const wchar_t FLAG_SEPARATOR = 0x1f; // flag separator after escaping
  const wchar_t MORPH_SEPARATOR =
      0x1e; // separator for boundary of entry (may be followed by morph data)

  virtual std::wstring unescapeEntry(const std::wstring &entry);

  static int morphBoundary(const std::wstring &line);

  static int indexOfSpaceOrTab(const std::wstring &text, int start);

  /**
   * Reads the dictionary file through the provided InputStreams, building up
   * the words map_obj
   *
   * @param dictionaries InputStreams to read the dictionary file through
   * @param decoder CharsetDecoder used to decode the contents of the file
   * @throws IOException Can be thrown while reading from the file
   */
private:
  void
  readDictionaryFiles(std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix,
                      std::deque<std::shared_ptr<InputStream>> &dictionaries,
                      std::shared_ptr<CharsetDecoder> decoder,
                      std::shared_ptr<Builder<std::shared_ptr<IntsRef>>>
                          words) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<Dictionary> outerInstance;

  public:
    ComparatorAnonymousInnerClass(std::shared_ptr<Dictionary> outerInstance);

    std::shared_ptr<BytesRef> scratch1;
    std::shared_ptr<BytesRef> scratch2;

    int compare(std::shared_ptr<BytesRef> o1, std::shared_ptr<BytesRef> o2);
  };

public:
  static std::deque<wchar_t> decodeFlags(std::shared_ptr<BytesRef> b);

  static void encodeFlags(std::shared_ptr<BytesRefBuilder> b,
                          std::deque<wchar_t> &flags);

private:
  void parseAlias(const std::wstring &line);

  std::wstring getAliasValue(int id);

public:
  virtual std::wstring getStemException(int id);

private:
  void parseMorphAlias(const std::wstring &line);

  std::wstring parseStemException(const std::wstring &morphData);

  /**
   * Abstraction of the process of parsing flags taken from the affix and dic
   * files
   */
public:
  class FlagParsingStrategy
      : public std::enable_shared_from_this<FlagParsingStrategy>
  {
    GET_CLASS_NAME(FlagParsingStrategy)

    /**
     * Parses the given std::wstring into a single flag
     *
     * @param rawFlag std::wstring to parse into a flag
     * @return Parsed flag
     */
  public:
    virtual wchar_t parseFlag(const std::wstring &rawFlag);

    /**
     * Parses the given std::wstring into multiple flags
     *
     * @param rawFlags std::wstring to parse into flags
     * @return Parsed flags
     */
    virtual std::deque<wchar_t> parseFlags(const std::wstring &rawFlags) = 0;
  };

  /**
   * Simple implementation of {@link FlagParsingStrategy} that treats the chars
   * in each std::wstring as a individual flags. Can be used with both the ASCII and
   * UTF-8 flag types.
   */
private:
  class SimpleFlagParsingStrategy : public FlagParsingStrategy
  {
    GET_CLASS_NAME(SimpleFlagParsingStrategy)
  public:
    std::deque<wchar_t> parseFlags(const std::wstring &rawFlags) override;

  protected:
    std::shared_ptr<SimpleFlagParsingStrategy> shared_from_this()
    {
      return std::static_pointer_cast<SimpleFlagParsingStrategy>(
          FlagParsingStrategy::shared_from_this());
    }
  };

  /**
   * Implementation of {@link FlagParsingStrategy} that assumes each flag is
   * encoded in its numerical form.  In the case of multiple flags, each number
   * is separated by a comma.
   */
private:
  class NumFlagParsingStrategy : public FlagParsingStrategy
  {
    GET_CLASS_NAME(NumFlagParsingStrategy)
  public:
    std::deque<wchar_t> parseFlags(const std::wstring &rawFlags) override;

  protected:
    std::shared_ptr<NumFlagParsingStrategy> shared_from_this()
    {
      return std::static_pointer_cast<NumFlagParsingStrategy>(
          FlagParsingStrategy::shared_from_this());
    }
  };

  /**
   * Implementation of {@link FlagParsingStrategy} that assumes each flag is
   * encoded as two ASCII characters whose codes must be combined into a single
   * character.
   */
private:
  class DoubleASCIIFlagParsingStrategy : public FlagParsingStrategy
  {
    GET_CLASS_NAME(DoubleASCIIFlagParsingStrategy)

  public:
    std::deque<wchar_t> parseFlags(const std::wstring &rawFlags) override;

  protected:
    std::shared_ptr<DoubleASCIIFlagParsingStrategy> shared_from_this()
    {
      return std::static_pointer_cast<DoubleASCIIFlagParsingStrategy>(
          FlagParsingStrategy::shared_from_this());
    }
  };

public:
  static bool hasFlag(std::deque<wchar_t> &flags, wchar_t flag);

  virtual std::shared_ptr<std::wstring>
  cleanInput(std::shared_ptr<std::wstring> input,
             std::shared_ptr<StringBuilder> reuse);

  /** folds single character (according to LANG if present) */
  virtual wchar_t caseFold(wchar_t c);

  // TODO: this could be more efficient!
  static void
  applyMappings(std::shared_ptr<FST<std::shared_ptr<CharsRef>>> fst,
                std::shared_ptr<StringBuilder> sb) ;

  /** Returns true if this dictionary was constructed with the {@code
   * ignoreCase} option */
  virtual bool getIgnoreCase();

private:
  static std::shared_ptr<Path> DEFAULT_TEMP_DIR;

  /** Used by test framework */
public:
  static void setDefaultTempDir(std::shared_ptr<Path> tempDir);

  /**
   * Returns the default temporary directory. By default, java.io.tmpdir. If not
   * accessible or not available, an IOException is thrown
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  static std::shared_ptr<Path> getDefaultTempDir() ;
};

} // namespace org::apache::lucene::analysis::hunspell
