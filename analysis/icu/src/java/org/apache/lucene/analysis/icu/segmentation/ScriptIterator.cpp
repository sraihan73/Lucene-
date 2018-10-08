using namespace std;

#include "ScriptIterator.h"

namespace org::apache::lucene::analysis::icu::segmentation
{
using com::ibm::icu::lang::UCharacter;
using com::ibm::icu::lang::UScript;
using com::ibm::icu::lang::UCharacterEnums::ECharacterCategory;
using com::ibm::icu::text::UTF16;

ScriptIterator::ScriptIterator(bool combineCJ) : combineCJ(combineCJ) {}

int ScriptIterator::getScriptStart() { return scriptStart; }

int ScriptIterator::getScriptLimit() { return scriptLimit; }

int ScriptIterator::getScriptCode() { return scriptCode; }

bool ScriptIterator::next()
{
  if (scriptLimit >= limit) {
    return false;
  }

  scriptCode = UScript::COMMON;
  scriptStart = scriptLimit;

  while (index < limit) {
    constexpr int ch = UTF16->charAt(text, start, limit, index - start);
    constexpr int sc = getScript(ch);

    /*
     * From UTR #24: Implementations that determine the boundaries between
     * characters of given scripts should never break between a non-spacing
     * mark and its base character. Thus for boundary determinations and
     * similar sorts of processing, a non-spacing mark — whatever its script
     * value — should inherit the script value of its base character.
     */
    if (isSameScript(scriptCode, sc) ||
        UCharacter::getType(ch) == ECharacterCategory::NON_SPACING_MARK) {
      index += UTF16::getCharCount(ch);

      /*
       * Inherited or Common becomes the script code of the surrounding text.
       */
      if (scriptCode <= UScript::INHERITED && sc > UScript::INHERITED) {
        scriptCode = sc;
      }

    } else {
      break;
    }
  }

  scriptLimit = index;
  return true;
}

bool ScriptIterator::isSameScript(int scriptOne, int scriptTwo)
{
  return scriptOne <= UScript::INHERITED || scriptTwo <= UScript::INHERITED ||
         scriptOne == scriptTwo;
}

void ScriptIterator::setText(std::deque<wchar_t> &text, int start, int length)
{
  this->text = text;
  this->start = start;
  this->index = start;
  this->limit = start + length;
  this->scriptStart = start;
  this->scriptLimit = start;
  this->scriptCode = UScript::INVALID_CODE;
}

std::deque<int> const ScriptIterator::basicLatin = std::deque<int>(128);

ScriptIterator::StaticConstructor::StaticConstructor()
{
  for (int i = 0; i < basicLatin.size(); i++) {
    basicLatin[i] = UScript::getScript(i);
  }
}

ScriptIterator::StaticConstructor ScriptIterator::staticConstructor;

int ScriptIterator::getScript(int codepoint)
{
  if (0 <= codepoint && codepoint < basicLatin.size()) {
    return basicLatin[codepoint];
  } else {
    int script = UScript::getScript(codepoint);
    if (combineCJ) {
      if (script == UScript::HAN || script == UScript::HIRAGANA ||
          script == UScript::KATAKANA) {
        return UScript::JAPANESE;
      } else if (codepoint >= 0xFF10 && codepoint <= 0xFF19) {
        // when using CJK dictionary breaking, don't let full width numbers go
        // to it, otherwise they are treated as punctuation. we currently have
        // no cleaner way to fix this!
        return UScript::LATIN;
      } else {
        return script;
      }
    } else {
      return script;
    }
  }
}
} // namespace org::apache::lucene::analysis::icu::segmentation