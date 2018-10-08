using namespace std;

#include "Stemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/ArrayUtil.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IntsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/fst/Outputs.h"
#include "Dictionary.h"

namespace org::apache::lucene::analysis::hunspell
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;
using FST = org::apache::lucene::util::fst::FST;
using Outputs = org::apache::lucene::util::fst::Outputs;

Stemmer::Stemmer(shared_ptr<Dictionary> dictionary)
    : dictionary(dictionary),
      affixReader(make_shared<ByteArrayDataInput>(dictionary->affixData)),
      formStep(dictionary->hasStemExceptions ? 2 : 1)
{
  for (int level = 0; level < 3; level++) {
    if (dictionary->prefixes != nullptr) {
      prefixArcs[level] = make_shared<FST::Arc<IntsRef>>();
      prefixReaders[level] = dictionary->prefixes->getBytesReader();
    }
    if (dictionary->suffixes != nullptr) {
      suffixArcs[level] = make_shared<FST::Arc<IntsRef>>();
      suffixReaders[level] = dictionary->suffixes->getBytesReader();
    }
  }
}

deque<std::shared_ptr<CharsRef>> Stemmer::stem(const wstring &word)
{
  return stem(word.toCharArray(), word.length());
}

deque<std::shared_ptr<CharsRef>> Stemmer::stem(std::deque<wchar_t> &word,
                                                int length)
{

  if (dictionary->needsInputCleaning) {
    scratchSegment->setLength(0);
    scratchSegment->append(word, 0, length);
    shared_ptr<std::wstring> cleaned =
        dictionary->cleanInput(scratchSegment, segment);
    scratchBuffer = ArrayUtil::grow(scratchBuffer, cleaned->length());
    length = segment->length();
    segment->getChars(0, length, scratchBuffer, 0);
    word = scratchBuffer;
  }

  int caseType = caseOf(word, length);
  if (caseType == UPPER_CASE) {
    // upper: union exact, title, lower
    caseFoldTitle(word, length);
    caseFoldLower(titleBuffer, length);
    deque<std::shared_ptr<CharsRef>> deque = doStem(word, length, false);
    deque.addAll(doStem(titleBuffer, length, true));
    deque.addAll(doStem(lowerBuffer, length, true));
    return deque;
  } else if (caseType == TITLE_CASE) {
    // title: union exact, lower
    caseFoldLower(word, length);
    deque<std::shared_ptr<CharsRef>> deque = doStem(word, length, false);
    deque.addAll(doStem(lowerBuffer, length, true));
    return deque;
  } else {
    // exact match only
    return doStem(word, length, false);
  }
}

int Stemmer::caseOf(std::deque<wchar_t> &word, int length)
{
  if (dictionary->ignoreCase || length == 0 || !isupper(word[0])) {
    return EXACT_CASE;
  }

  // determine if we are title or lowercase (or something funky, in which it's
  // exact)
  bool seenUpper = false;
  bool seenLower = false;
  for (int i = 1; i < length; i++) {
    bool v = isupper(word[i]);
    seenUpper |= v;
    seenLower |= !v;
  }

  if (!seenLower) {
    return UPPER_CASE;
  } else if (!seenUpper) {
    return TITLE_CASE;
  } else {
    return EXACT_CASE;
  }
}

void Stemmer::caseFoldTitle(std::deque<wchar_t> &word, int length)
{
  titleBuffer = ArrayUtil::grow(titleBuffer, length);
  System::arraycopy(word, 0, titleBuffer, 0, length);
  for (int i = 1; i < length; i++) {
    titleBuffer[i] = dictionary->caseFold(titleBuffer[i]);
  }
}

void Stemmer::caseFoldLower(std::deque<wchar_t> &word, int length)
{
  lowerBuffer = ArrayUtil::grow(lowerBuffer, length);
  System::arraycopy(word, 0, lowerBuffer, 0, length);
  lowerBuffer[0] = dictionary->caseFold(lowerBuffer[0]);
}

deque<std::shared_ptr<CharsRef>> Stemmer::doStem(std::deque<wchar_t> &word,
                                                  int length, bool caseVariant)
{
  deque<std::shared_ptr<CharsRef>> stems = deque<std::shared_ptr<CharsRef>>();
  shared_ptr<IntsRef> forms = dictionary->lookupWord(word, 0, length);
  if (forms != nullptr) {
    for (int i = 0; i < forms->length; i += formStep) {
      bool checkKeepCase = caseVariant && dictionary->keepcase != -1;
      bool checkNeedAffix = dictionary->needaffix != -1;
      bool checkOnlyInCompound = dictionary->onlyincompound != -1;
      if (checkKeepCase || checkNeedAffix || checkOnlyInCompound) {
        dictionary->flagLookup->get(forms->ints[forms->offset + i], scratch);
        std::deque<wchar_t> wordFlags = Dictionary::decodeFlags(scratch);
        // we are looking for a case variant, but this word does not allow it
        if (checkKeepCase &&
            Dictionary::hasFlag(wordFlags,
                                static_cast<wchar_t>(dictionary->keepcase))) {
          continue;
        }
        // we can't add this form, it's a pseudostem requiring an affix
        if (checkNeedAffix &&
            Dictionary::hasFlag(wordFlags,
                                static_cast<wchar_t>(dictionary->needaffix))) {
          continue;
        }
        // we can't add this form, it only belongs inside a compound word
        if (checkOnlyInCompound &&
            Dictionary::hasFlag(
                wordFlags, static_cast<wchar_t>(dictionary->onlyincompound))) {
          continue;
        }
      }
      stems.push_back(newStem(word, length, forms, i));
    }
  }
  try {
    bool v = stems.addAll(stem(word, length, -1, -1, -1, 0, true, true, false,
                               false, caseVariant));
  } catch (const IOException &bogus) {
    throw runtime_error(bogus);
  }
  return stems;
}

deque<std::shared_ptr<CharsRef>>
Stemmer::uniqueStems(std::deque<wchar_t> &word, int length)
{
  deque<std::shared_ptr<CharsRef>> stems = stem(word, length);
  if (stems.size() < 2) {
    return stems;
  }
  shared_ptr<CharArraySet> terms =
      make_shared<CharArraySet>(8, dictionary->ignoreCase);
  deque<std::shared_ptr<CharsRef>> deduped =
      deque<std::shared_ptr<CharsRef>>();
  for (auto s : stems) {
    if (!terms->contains(s)) {
      deduped.push_back(s);
      terms->add(s);
    }
  }
  return deduped;
}

shared_ptr<CharsRef> Stemmer::newStem(std::deque<wchar_t> &buffer, int length,
                                      shared_ptr<IntsRef> forms, int formID)
{
  const wstring exception;
  if (dictionary->hasStemExceptions) {
    int exceptionID = forms->ints[forms->offset + formID + 1];
    if (exceptionID > 0) {
      exception = dictionary->getStemException(exceptionID);
    } else {
      exception = L"";
    }
  } else {
    exception = L"";
  }

  if (dictionary->needsOutputCleaning) {
    scratchSegment->setLength(0);
    if (exception != L"") {
      scratchSegment->append(exception);
    } else {
      scratchSegment->append(buffer, 0, length);
    }
    try {
      Dictionary::applyMappings(dictionary->oconv, scratchSegment);
    } catch (const IOException &bogus) {
      throw runtime_error(bogus);
    }
    std::deque<wchar_t> cleaned(scratchSegment->length());
    scratchSegment->getChars(0, cleaned.size(), cleaned, 0);
    return make_shared<CharsRef>(cleaned, 0, cleaned.size());
  } else {
    if (exception != L"") {
      return make_shared<CharsRef>(exception);
    } else {
      return make_shared<CharsRef>(buffer, 0, length);
    }
  }
}

deque<std::shared_ptr<CharsRef>>
Stemmer::stem(std::deque<wchar_t> &word, int length, int previous,
              int prevFlag, int prefixFlag, int recursionDepth, bool doPrefix,
              bool doSuffix, bool previousWasPrefix, bool circumfix,
              bool caseVariant) 
{

  // TODO: allow this stuff to be reused by tokenfilter
  deque<std::shared_ptr<CharsRef>> stems = deque<std::shared_ptr<CharsRef>>();

  if (doPrefix && dictionary->prefixes != nullptr) {
    shared_ptr<FST<std::shared_ptr<IntsRef>>> fst = dictionary->prefixes;
    shared_ptr<Outputs<std::shared_ptr<IntsRef>>> outputs = fst->outputs;
    shared_ptr<FST::BytesReader> bytesReader = prefixReaders[recursionDepth];
    shared_ptr<FST::Arc<std::shared_ptr<IntsRef>>> arc =
        prefixArcs[recursionDepth];
    fst->getFirstArc(arc);
    shared_ptr<IntsRef> NO_OUTPUT = outputs->getNoOutput();
    shared_ptr<IntsRef> output = NO_OUTPUT;
    int limit = dictionary->fullStrip ? length : length - 1;
    for (int i = 0; i < limit; i++) {
      if (i > 0) {
        int ch = word[i - 1];
        if (fst->findTargetArc(ch, arc, arc, bytesReader) == nullptr) {
          break;
        } else if (arc->output != NO_OUTPUT) {
          output = fst->outputs->add(output, arc->output);
        }
      }
      shared_ptr<IntsRef> prefixes = nullptr;
      if (!arc->isFinal()) {
        continue;
      } else {
        prefixes = fst->outputs->add(output, arc->nextFinalOutput);
      }

      for (int j = 0; j < prefixes->length; j++) {
        int prefix = prefixes->ints[prefixes->offset + j];
        if (prefix == previous) {
          continue;
        }
        affixReader->setPosition(8 * prefix);
        wchar_t flag = static_cast<wchar_t>(affixReader->readShort() & 0xffff);
        wchar_t stripOrd =
            static_cast<wchar_t>(affixReader->readShort() & 0xffff);
        int condition = static_cast<wchar_t>(affixReader->readShort() & 0xffff);
        bool crossProduct = (condition & 1) == 1;
        condition = static_cast<int>(static_cast<unsigned int>(condition) >> 1);
        wchar_t append =
            static_cast<wchar_t>(affixReader->readShort() & 0xffff);

        constexpr bool compatible;
        if (recursionDepth == 0) {
          if (dictionary->onlyincompound == -1) {
            compatible = true;
          } else {
            // check if affix is allowed in a non-compound word
            dictionary->flagLookup->get(append, scratch);
            std::deque<wchar_t> appendFlags = Dictionary::decodeFlags(scratch);
            compatible = !Dictionary::hasFlag(
                appendFlags, static_cast<wchar_t>(dictionary->onlyincompound));
          }
        } else if (crossProduct) {
          // cross check incoming continuation class (flag of previous affix)
          // against deque.
          dictionary->flagLookup->get(append, scratch);
          std::deque<wchar_t> appendFlags = Dictionary::decodeFlags(scratch);
          assert(prevFlag >= 0);
          bool allowed = dictionary->onlyincompound == -1 ||
                         !Dictionary::hasFlag(
                             appendFlags,
                             static_cast<wchar_t>(dictionary->onlyincompound));
          compatible =
              allowed && hasCrossCheckedFlag(static_cast<wchar_t>(prevFlag),
                                             appendFlags, false);
        } else {
          compatible = false;
        }

        if (compatible) {
          int deAffixedStart = i;
          int deAffixedLength = length - deAffixedStart;

          int stripStart = dictionary->stripOffsets[stripOrd];
          int stripEnd = dictionary->stripOffsets[stripOrd + 1];
          int stripLength = stripEnd - stripStart;

          if (!checkCondition(condition, dictionary->stripData, stripStart,
                              stripLength, word, deAffixedStart,
                              deAffixedLength)) {
            continue;
          }

          std::deque<wchar_t> strippedWord(stripLength + deAffixedLength);
          System::arraycopy(dictionary->stripData, stripStart, strippedWord, 0,
                            stripLength);
          System::arraycopy(word, deAffixedStart, strippedWord, stripLength,
                            deAffixedLength);

          deque<std::shared_ptr<CharsRef>> stemList =
              applyAffix(strippedWord, strippedWord.size(), prefix, -1,
                         recursionDepth, true, circumfix, caseVariant);

          stems.insert(stems.end(), stemList.begin(), stemList.end());
        }
      }
    }
  }

  if (doSuffix && dictionary->suffixes != nullptr) {
    shared_ptr<FST<std::shared_ptr<IntsRef>>> fst = dictionary->suffixes;
    shared_ptr<Outputs<std::shared_ptr<IntsRef>>> outputs = fst->outputs;
    shared_ptr<FST::BytesReader> bytesReader = suffixReaders[recursionDepth];
    shared_ptr<FST::Arc<std::shared_ptr<IntsRef>>> arc =
        suffixArcs[recursionDepth];
    fst->getFirstArc(arc);
    shared_ptr<IntsRef> NO_OUTPUT = outputs->getNoOutput();
    shared_ptr<IntsRef> output = NO_OUTPUT;
    int limit = dictionary->fullStrip ? 0 : 1;
    for (int i = length; i >= limit; i--) {
      if (i < length) {
        int ch = word[i];
        if (fst->findTargetArc(ch, arc, arc, bytesReader) == nullptr) {
          break;
        } else if (arc->output != NO_OUTPUT) {
          output = fst->outputs->add(output, arc->output);
        }
      }
      shared_ptr<IntsRef> suffixes = nullptr;
      if (!arc->isFinal()) {
        continue;
      } else {
        suffixes = fst->outputs->add(output, arc->nextFinalOutput);
      }

      for (int j = 0; j < suffixes->length; j++) {
        int suffix = suffixes->ints[suffixes->offset + j];
        if (suffix == previous) {
          continue;
        }
        affixReader->setPosition(8 * suffix);
        wchar_t flag = static_cast<wchar_t>(affixReader->readShort() & 0xffff);
        wchar_t stripOrd =
            static_cast<wchar_t>(affixReader->readShort() & 0xffff);
        int condition = static_cast<wchar_t>(affixReader->readShort() & 0xffff);
        bool crossProduct = (condition & 1) == 1;
        condition = static_cast<int>(static_cast<unsigned int>(condition) >> 1);
        wchar_t append =
            static_cast<wchar_t>(affixReader->readShort() & 0xffff);

        constexpr bool compatible;
        if (recursionDepth == 0) {
          if (dictionary->onlyincompound == -1) {
            compatible = true;
          } else {
            // check if affix is allowed in a non-compound word
            dictionary->flagLookup->get(append, scratch);
            std::deque<wchar_t> appendFlags = Dictionary::decodeFlags(scratch);
            compatible = !Dictionary::hasFlag(
                appendFlags, static_cast<wchar_t>(dictionary->onlyincompound));
          }
        } else if (crossProduct) {
          // cross check incoming continuation class (flag of previous affix)
          // against deque.
          dictionary->flagLookup->get(append, scratch);
          std::deque<wchar_t> appendFlags = Dictionary::decodeFlags(scratch);
          assert(prevFlag >= 0);
          bool allowed = dictionary->onlyincompound == -1 ||
                         !Dictionary::hasFlag(
                             appendFlags,
                             static_cast<wchar_t>(dictionary->onlyincompound));
          compatible =
              allowed && hasCrossCheckedFlag(static_cast<wchar_t>(prevFlag),
                                             appendFlags, previousWasPrefix);
        } else {
          compatible = false;
        }

        if (compatible) {
          int appendLength = length - i;
          int deAffixedLength = length - appendLength;

          int stripStart = dictionary->stripOffsets[stripOrd];
          int stripEnd = dictionary->stripOffsets[stripOrd + 1];
          int stripLength = stripEnd - stripStart;

          if (!checkCondition(condition, word, 0, deAffixedLength,
                              dictionary->stripData, stripStart, stripLength)) {
            continue;
          }

          std::deque<wchar_t> strippedWord(stripLength + deAffixedLength);
          System::arraycopy(word, 0, strippedWord, 0, deAffixedLength);
          System::arraycopy(dictionary->stripData, stripStart, strippedWord,
                            deAffixedLength, stripLength);

          deque<std::shared_ptr<CharsRef>> stemList =
              applyAffix(strippedWord, strippedWord.size(), suffix, prefixFlag,
                         recursionDepth, false, circumfix, caseVariant);

          stems.insert(stems.end(), stemList.begin(), stemList.end());
        }
      }
    }
  }

  return stems;
}

bool Stemmer::checkCondition(int condition, std::deque<wchar_t> &c1, int c1off,
                             int c1len, std::deque<wchar_t> &c2, int c2off,
                             int c2len)
{
  if (condition != 0) {
    shared_ptr<CharacterRunAutomaton> pattern = dictionary->patterns[condition];
    int state = 0;
    for (int i = c1off; i < c1off + c1len; i++) {
      state = pattern->step(state, c1[i]);
      if (state == -1) {
        return false;
      }
    }
    for (int i = c2off; i < c2off + c2len; i++) {
      state = pattern->step(state, c2[i]);
      if (state == -1) {
        return false;
      }
    }
    return pattern->isAccept(state);
  }
  return true;
}

deque<std::shared_ptr<CharsRef>>
Stemmer::applyAffix(std::deque<wchar_t> &strippedWord, int length, int affix,
                    int prefixFlag, int recursionDepth, bool prefix,
                    bool circumfix, bool caseVariant) 
{
  // TODO: just pass this in from before, no need to decode it twice
  affixReader->setPosition(8 * affix);
  wchar_t flag = static_cast<wchar_t>(affixReader->readShort() & 0xffff);
  affixReader->skipBytes(2); // strip
  int condition = static_cast<wchar_t>(affixReader->readShort() & 0xffff);
  bool crossProduct = (condition & 1) == 1;
  condition = static_cast<int>(static_cast<unsigned int>(condition) >> 1);
  wchar_t append = static_cast<wchar_t>(affixReader->readShort() & 0xffff);

  deque<std::shared_ptr<CharsRef>> stems = deque<std::shared_ptr<CharsRef>>();

  shared_ptr<IntsRef> forms = dictionary->lookupWord(strippedWord, 0, length);
  if (forms != nullptr) {
    for (int i = 0; i < forms->length; i += formStep) {
      dictionary->flagLookup->get(forms->ints[forms->offset + i], scratch);
      std::deque<wchar_t> wordFlags = Dictionary::decodeFlags(scratch);
      if (Dictionary::hasFlag(wordFlags, flag)) {
        // confusing: in this one exception, we already chained the first prefix
        // against the second, so it doesnt need to be checked against the word
        bool chainedPrefix =
            dictionary->complexPrefixes && recursionDepth == 1 && prefix;
        if (chainedPrefix == false && prefixFlag >= 0 &&
            !Dictionary::hasFlag(wordFlags, static_cast<wchar_t>(prefixFlag))) {
          // see if we can chain prefix thru the suffix continuation class (only
          // if it has any!)
          dictionary->flagLookup->get(append, scratch);
          std::deque<wchar_t> appendFlags = Dictionary::decodeFlags(scratch);
          if (!hasCrossCheckedFlag(static_cast<wchar_t>(prefixFlag),
                                   appendFlags, false)) {
            continue;
          }
        }

        // if circumfix was previously set by a prefix, we must check this
        // suffix, to ensure it has it, and vice versa
        if (dictionary->circumfix != -1) {
          dictionary->flagLookup->get(append, scratch);
          std::deque<wchar_t> appendFlags = Dictionary::decodeFlags(scratch);
          bool suffixCircumfix = Dictionary::hasFlag(
              appendFlags, static_cast<wchar_t>(dictionary->circumfix));
          if (circumfix != suffixCircumfix) {
            continue;
          }
        }

        // we are looking for a case variant, but this word does not allow it
        if (caseVariant && dictionary->keepcase != -1 &&
            Dictionary::hasFlag(wordFlags,
                                static_cast<wchar_t>(dictionary->keepcase))) {
          continue;
        }
        // we aren't decompounding (yet)
        if (dictionary->onlyincompound != -1 &&
            Dictionary::hasFlag(
                wordFlags, static_cast<wchar_t>(dictionary->onlyincompound))) {
          continue;
        }
        stems.push_back(newStem(strippedWord, length, forms, i));
      }
    }
  }

  // if a circumfix flag is defined in the dictionary, and we are a prefix, we
  // need to check if we have that flag
  if (dictionary->circumfix != -1 && !circumfix && prefix) {
    dictionary->flagLookup->get(append, scratch);
    std::deque<wchar_t> appendFlags = Dictionary::decodeFlags(scratch);
    circumfix = Dictionary::hasFlag(
        appendFlags, static_cast<wchar_t>(dictionary->circumfix));
  }

  if (crossProduct) {
    if (recursionDepth == 0) {
      if (prefix) {
        // we took away the first prefix.
        // COMPLEXPREFIXES = true:  combine with a second prefix and another
        // suffix COMPLEXPREFIXES = false: combine with a suffix
        stems.addAll(
            stem(strippedWord, length, affix, flag, flag, ++recursionDepth,
                 dictionary->complexPrefixes && dictionary->twoStageAffix, true,
                 true, circumfix, caseVariant));
      } else if (dictionary->complexPrefixes == false &&
                 dictionary->twoStageAffix) {
        // we took away a suffix.
        // COMPLEXPREFIXES = true: we don't recurse! only one suffix allowed
        // COMPLEXPREFIXES = false: combine with another suffix
        stems.addAll(stem(strippedWord, length, affix, flag, prefixFlag,
                          ++recursionDepth, false, true, false, circumfix,
                          caseVariant));
      }
    } else if (recursionDepth == 1) {
      if (prefix && dictionary->complexPrefixes) {
        // we took away the second prefix: go look for another suffix
        stems.addAll(stem(strippedWord, length, affix, flag, flag,
                          ++recursionDepth, false, true, true, circumfix,
                          caseVariant));
      } else if (prefix == false && dictionary->complexPrefixes == false &&
                 dictionary->twoStageAffix) {
        // we took away a prefix, then a suffix: go look for another suffix
        stems.addAll(stem(strippedWord, length, affix, flag, prefixFlag,
                          ++recursionDepth, false, true, false, circumfix,
                          caseVariant));
      }
    }
  }

  return stems;
}

bool Stemmer::hasCrossCheckedFlag(wchar_t flag, std::deque<wchar_t> &flags,
                                  bool matchEmpty)
{
  return (flags.empty() && matchEmpty) ||
         Arrays::binarySearch(flags, flag) >= 0;
}
} // namespace org::apache::lucene::analysis::hunspell