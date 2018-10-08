using namespace std;

#include "DefaultICUTokenizerConfig.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using com::ibm::icu::lang::UScript;
using com::ibm::icu::text::BreakIterator;
using com::ibm::icu::text::RuleBasedBreakIterator;
using com::ibm::icu::util::ULocale;
const wstring DefaultICUTokenizerConfig::WORD_IDEO =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::
             IDEOGRAPHIC];
const wstring DefaultICUTokenizerConfig::WORD_HIRAGANA =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::HIRAGANA];
const wstring DefaultICUTokenizerConfig::WORD_KATAKANA =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::KATAKANA];
const wstring DefaultICUTokenizerConfig::WORD_HANGUL =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::HANGUL];
const wstring DefaultICUTokenizerConfig::WORD_LETTER =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::ALPHANUM];
const wstring DefaultICUTokenizerConfig::WORD_NUMBER =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::NUM];
const wstring DefaultICUTokenizerConfig::WORD_EMOJI =
    org::apache::lucene::analysis::standard::StandardTokenizer::TOKEN_TYPES
        [org::apache::lucene::analysis::standard::StandardTokenizer::EMOJI];
const shared_ptr<com::ibm::icu::text::BreakIterator>
    DefaultICUTokenizerConfig::cjkBreakIterator =
        com::ibm::icu::text::BreakIterator::getWordInstance(
            com::ibm::icu::util::ULocale::ROOT);
const shared_ptr<com::ibm::icu::text::RuleBasedBreakIterator>
    DefaultICUTokenizerConfig::defaultBreakIterator =
        readBreakIterator(L"Default.brk");
const shared_ptr<com::ibm::icu::text::RuleBasedBreakIterator>
    DefaultICUTokenizerConfig::myanmarSyllableIterator =
        readBreakIterator(L"MyanmarSyllable.brk");

DefaultICUTokenizerConfig::DefaultICUTokenizerConfig(bool cjkAsWords,
                                                     bool myanmarAsWords)
    : cjkAsWords(cjkAsWords), myanmarAsWords(myanmarAsWords)
{
}

bool DefaultICUTokenizerConfig::combineCJ() { return cjkAsWords; }

shared_ptr<RuleBasedBreakIterator>
DefaultICUTokenizerConfig::getBreakIterator(int script)
{
  switch (script) {
  case UScript::JAPANESE:
    return std::static_pointer_cast<RuleBasedBreakIterator>(
        cjkBreakIterator->clone());
  case UScript::MYANMAR:
    if (myanmarAsWords) {
      return std::static_pointer_cast<RuleBasedBreakIterator>(
          defaultBreakIterator->clone());
    } else {
      return std::static_pointer_cast<RuleBasedBreakIterator>(
          myanmarSyllableIterator->clone());
    }
  default:
    return std::static_pointer_cast<RuleBasedBreakIterator>(
        defaultBreakIterator->clone());
  }
}

wstring DefaultICUTokenizerConfig::getType(int script, int ruleStatus)
{
  switch (ruleStatus) {
  case RuleBasedBreakIterator::WORD_IDEO:
    return WORD_IDEO;
  case RuleBasedBreakIterator::WORD_KANA:
    return script == UScript::HIRAGANA ? WORD_HIRAGANA : WORD_KATAKANA;
  case RuleBasedBreakIterator::WORD_LETTER:
    return script == UScript::HANGUL ? WORD_HANGUL : WORD_LETTER;
  case RuleBasedBreakIterator::WORD_NUMBER:
    return WORD_NUMBER;
  case EMOJI_SEQUENCE_STATUS:
    return WORD_EMOJI;
  default: // some other custom code
    return L"<OTHER>";
  }
}

shared_ptr<RuleBasedBreakIterator>
DefaultICUTokenizerConfig::readBreakIterator(const wstring &filename)
{
  shared_ptr<InputStream> is =
      DefaultICUTokenizerConfig::typeid->getResourceAsStream(filename);
  try {
    shared_ptr<RuleBasedBreakIterator> bi =
        RuleBasedBreakIterator::getInstanceFromCompiledRules(is);
    is->close();
    return bi;
  } catch (const IOException &e) {
    throw runtime_error(e);
  }
}
} // namespace org::apache::lucene::analysis::icu::segmentation