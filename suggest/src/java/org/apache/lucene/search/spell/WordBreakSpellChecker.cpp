using namespace std;

#include "WordBreakSpellChecker.h"

namespace org::apache::lucene::search::spell
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Term = org::apache::lucene::index::Term;
using SuggestMode = org::apache::lucene::search::spell::SuggestMode;
const shared_ptr<org::apache::lucene::index::Term>
    WordBreakSpellChecker::SEPARATOR_TERM =
        make_shared<org::apache::lucene::index::Term>(L"", L"");

WordBreakSpellChecker::WordBreakSpellChecker() {}

std::deque<std::deque<std::shared_ptr<SuggestWord>>>
WordBreakSpellChecker::suggestWordBreaks(
    shared_ptr<Term> term, int maxSuggestions, shared_ptr<IndexReader> ir,
    SuggestMode suggestMode,
    BreakSuggestionSortMethod sortMethod) 
{
  if (maxSuggestions < 1) {
    return deque<deque<SuggestWord>>(0);
  }
  if (suggestMode == nullptr) {
    suggestMode = SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX;
  }
  if (sortMethod == nullptr) {
    sortMethod = BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY;
  }

  int queueInitialCapacity = maxSuggestions > 10 ? 10 : maxSuggestions;
  shared_ptr<Comparator<std::shared_ptr<SuggestWordArrayWrapper>>>
      queueComparator =
          sortMethod ==
                  BreakSuggestionSortMethod::NUM_CHANGES_THEN_MAX_FREQUENCY
              ? make_shared<LengthThenMaxFreqComparator>()
              : make_shared<LengthThenSumFreqComparator>();
  deque<std::shared_ptr<SuggestWordArrayWrapper>> suggestions =
      make_shared<PriorityQueue<std::shared_ptr<SuggestWordArrayWrapper>>>(
          queueInitialCapacity, queueComparator);

  int origFreq = ir->docFreq(term);
  if (origFreq > 0 && suggestMode == SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX) {
    return std::deque<std::deque<std::shared_ptr<SuggestWord>>>(0);
  }

  int useMinSuggestionFrequency = minSuggestionFrequency;
  if (suggestMode == SuggestMode::SUGGEST_MORE_POPULAR) {
    useMinSuggestionFrequency = (origFreq == 0 ? 1 : origFreq);
  }

  generateBreakUpSuggestions(
      term, ir, 1, maxSuggestions, useMinSuggestionFrequency,
      std::deque<std::shared_ptr<SuggestWord>>(0), suggestions, 0, sortMethod);

  std::deque<std::deque<std::shared_ptr<SuggestWord>>> suggestionArray(
      suggestions.size());
  for (int i = suggestions.size() - 1; i >= 0; i--) {
    suggestionArray[i] = suggestions.pop_front().suggestWords;
  }

  return suggestionArray;
}

std::deque<std::shared_ptr<CombineSuggestion>>
WordBreakSpellChecker::suggestWordCombinations(
    std::deque<std::shared_ptr<Term>> &terms, int maxSuggestions,
    shared_ptr<IndexReader> ir, SuggestMode suggestMode) 
{
  if (maxSuggestions < 1) {
    return std::deque<std::shared_ptr<CombineSuggestion>>(0);
  }

  std::deque<int> origFreqs;
  if (suggestMode != SuggestMode::SUGGEST_ALWAYS) {
    origFreqs = std::deque<int>(terms.size());
    for (int i = 0; i < terms.size(); i++) {
      origFreqs[i] = ir->docFreq(terms[i]);
    }
  }

  int queueInitialCapacity = maxSuggestions > 10 ? 10 : maxSuggestions;
  shared_ptr<Comparator<std::shared_ptr<CombineSuggestionWrapper>>>
      queueComparator = make_shared<CombinationsThenFreqComparator>();
  deque<std::shared_ptr<CombineSuggestionWrapper>> suggestions =
      make_shared<PriorityQueue<std::shared_ptr<CombineSuggestionWrapper>>>(
          queueInitialCapacity, queueComparator);

  int thisTimeEvaluations = 0;
  for (int i = 0; i < terms.size() - 1; i++) {
    if (terms[i]->equals(SEPARATOR_TERM)) {
      continue;
    }
    wstring leftTermText = terms[i]->text();
    int leftTermLength = leftTermText.codePointCount(0, leftTermText.length());
    if (leftTermLength > maxCombineWordLength) {
      continue;
    }
    int maxFreq = 0;
    int minFreq = numeric_limits<int>::max();
    if (origFreqs.size() > 0) {
      maxFreq = origFreqs[i];
      minFreq = origFreqs[i];
    }
    wstring combinedTermText = leftTermText;
    int combinedLength = leftTermLength;
    for (int j = i + 1; j < terms.size() && j - i <= maxChanges; j++) {
      if (terms[j]->equals(SEPARATOR_TERM)) {
        break;
      }
      wstring rightTermText = terms[j]->text();
      int rightTermLength =
          rightTermText.codePointCount(0, rightTermText.length());
      combinedTermText += rightTermText;
      combinedLength += rightTermLength;
      if (combinedLength > maxCombineWordLength) {
        break;
      }

      if (origFreqs.size() > 0) {
        maxFreq = max(maxFreq, origFreqs[j]);
        minFreq = min(minFreq, origFreqs[j]);
      }

      shared_ptr<Term> combinedTerm =
          make_shared<Term>(terms[0]->field(), combinedTermText);
      int combinedTermFreq = ir->docFreq(combinedTerm);

      if (suggestMode != SuggestMode::SUGGEST_MORE_POPULAR ||
          combinedTermFreq >= maxFreq) {
        if (suggestMode != SuggestMode::SUGGEST_WHEN_NOT_IN_INDEX ||
            minFreq == 0) {
          if (combinedTermFreq >= minSuggestionFrequency) {
            std::deque<int> origIndexes(j - i + 1);
            origIndexes[0] = i;
            for (int k = 1; k < origIndexes.size(); k++) {
              origIndexes[k] = i + k;
            }
            shared_ptr<SuggestWord> word = make_shared<SuggestWord>();
            word->freq = combinedTermFreq;
            word->score = origIndexes.size() - 1;
            word->string = combinedTerm->text();
            shared_ptr<CombineSuggestionWrapper> suggestion =
                make_shared<CombineSuggestionWrapper>(
                    make_shared<CombineSuggestion>(word, origIndexes),
                    (origIndexes.size() - 1));
            suggestions.push_back(suggestion);
            if (suggestions.size() > maxSuggestions) {
              suggestions.pop_front();
            }
          }
        }
      }
      thisTimeEvaluations++;
      if (thisTimeEvaluations == maxEvaluations) {
        break;
      }
    }
  }
  std::deque<std::shared_ptr<CombineSuggestion>> combineSuggestions(
      suggestions.size());
  for (int i = suggestions.size() - 1; i >= 0; i--) {
    combineSuggestions[i] = suggestions.pop_front().combineSuggestion;
  }
  return combineSuggestions;
}

int WordBreakSpellChecker::generateBreakUpSuggestions(
    shared_ptr<Term> term, shared_ptr<IndexReader> ir, int numberBreaks,
    int maxSuggestions, int useMinSuggestionFrequency,
    std::deque<std::shared_ptr<SuggestWord>> &prefix,
    deque<std::shared_ptr<SuggestWordArrayWrapper>> &suggestions,
    int totalEvaluations,
    BreakSuggestionSortMethod sortMethod) 
{
  wstring termText = term->text();
  int termLength = termText.codePointCount(0, termText.length());
  int useMinBreakWordLength = minBreakWordLength;
  if (useMinBreakWordLength < 1) {
    useMinBreakWordLength = 1;
  }
  if (termLength < (useMinBreakWordLength * 2)) {
    return 0;
  }

  int thisTimeEvaluations = 0;
  for (int i = useMinBreakWordLength; i <= (termLength - useMinBreakWordLength);
       i++) {
    int end = termText.offsetByCodePoints(0, i);
    wstring leftText = termText.substr(0, end);
    wstring rightText = termText.substr(end);
    shared_ptr<SuggestWord> leftWord =
        generateSuggestWord(ir, term->field(), leftText);

    if (leftWord->freq >= useMinSuggestionFrequency) {
      shared_ptr<SuggestWord> rightWord =
          generateSuggestWord(ir, term->field(), rightText);
      if (rightWord->freq >= useMinSuggestionFrequency) {
        shared_ptr<SuggestWordArrayWrapper> suggestion =
            make_shared<SuggestWordArrayWrapper>(
                newSuggestion(prefix, leftWord, rightWord));
        suggestions.push_back(suggestion);
        if (suggestions.size() > maxSuggestions) {
          suggestions.pop_front();
        }
      }
      int newNumberBreaks = numberBreaks + 1;
      if (newNumberBreaks <= maxChanges) {
        int evaluations = generateBreakUpSuggestions(
            make_shared<Term>(term->field(), rightWord->string), ir,
            newNumberBreaks, maxSuggestions, useMinSuggestionFrequency,
            newPrefix(prefix, leftWord), suggestions, totalEvaluations,
            sortMethod);
        totalEvaluations += evaluations;
      }
    }

    thisTimeEvaluations++;
    totalEvaluations++;
    if (totalEvaluations >= maxEvaluations) {
      break;
    }
  }
  return thisTimeEvaluations;
}

std::deque<std::shared_ptr<SuggestWord>> WordBreakSpellChecker::newPrefix(
    std::deque<std::shared_ptr<SuggestWord>> &oldPrefix,
    shared_ptr<SuggestWord> append)
{
  std::deque<std::shared_ptr<SuggestWord>> newPrefix(oldPrefix.size() + 1);
  System::arraycopy(oldPrefix, 0, newPrefix, 0, oldPrefix.size());
  newPrefix[newPrefix.size() - 1] = append;
  return newPrefix;
}

std::deque<std::shared_ptr<SuggestWord>> WordBreakSpellChecker::newSuggestion(
    std::deque<std::shared_ptr<SuggestWord>> &prefix,
    shared_ptr<SuggestWord> append1, shared_ptr<SuggestWord> append2)
{
  std::deque<std::shared_ptr<SuggestWord>> newSuggestion(prefix.size() + 2);
  int score = prefix.size() + 1;
  for (int i = 0; i < prefix.size(); i++) {
    shared_ptr<SuggestWord> word = make_shared<SuggestWord>();
    word->string = prefix[i]->string;
    word->freq = prefix[i]->freq;
    word->score = score;
    newSuggestion[i] = word;
  }
  append1->score = score;
  append2->score = score;
  newSuggestion[newSuggestion.size() - 2] = append1;
  newSuggestion[newSuggestion.size() - 1] = append2;
  return newSuggestion;
}

shared_ptr<SuggestWord> WordBreakSpellChecker::generateSuggestWord(
    shared_ptr<IndexReader> ir, const wstring &fieldname,
    const wstring &text) 
{
  shared_ptr<Term> term = make_shared<Term>(fieldname, text);
  int freq = ir->docFreq(term);
  shared_ptr<SuggestWord> word = make_shared<SuggestWord>();
  word->freq = freq;
  word->score = 1;
  word->string = text;
  return word;
}

int WordBreakSpellChecker::getMinSuggestionFrequency()
{
  return minSuggestionFrequency;
}

int WordBreakSpellChecker::getMaxCombineWordLength()
{
  return maxCombineWordLength;
}

int WordBreakSpellChecker::getMinBreakWordLength()
{
  return minBreakWordLength;
}

int WordBreakSpellChecker::getMaxChanges() { return maxChanges; }

int WordBreakSpellChecker::getMaxEvaluations() { return maxEvaluations; }

void WordBreakSpellChecker::setMinSuggestionFrequency(
    int minSuggestionFrequency)
{
  this->minSuggestionFrequency = minSuggestionFrequency;
}

void WordBreakSpellChecker::setMaxCombineWordLength(int maxCombineWordLength)
{
  this->maxCombineWordLength = maxCombineWordLength;
}

void WordBreakSpellChecker::setMinBreakWordLength(int minBreakWordLength)
{
  this->minBreakWordLength = minBreakWordLength;
}

void WordBreakSpellChecker::setMaxChanges(int maxChanges)
{
  this->maxChanges = maxChanges;
}

void WordBreakSpellChecker::setMaxEvaluations(int maxEvaluations)
{
  this->maxEvaluations = maxEvaluations;
}

int WordBreakSpellChecker::LengthThenMaxFreqComparator::compare(
    shared_ptr<SuggestWordArrayWrapper> o1,
    shared_ptr<SuggestWordArrayWrapper> o2)
{
  if (o1->suggestWords.size() != o2->suggestWords.size()) {
    return o2->suggestWords.size() - o1->suggestWords.size();
  }
  if (o1->freqMax != o2->freqMax) {
    return o1->freqMax - o2->freqMax;
  }
  return 0;
}

int WordBreakSpellChecker::LengthThenSumFreqComparator::compare(
    shared_ptr<SuggestWordArrayWrapper> o1,
    shared_ptr<SuggestWordArrayWrapper> o2)
{
  if (o1->suggestWords.size() != o2->suggestWords.size()) {
    return o2->suggestWords.size() - o1->suggestWords.size();
  }
  if (o1->freqSum != o2->freqSum) {
    return o1->freqSum - o2->freqSum;
  }
  return 0;
}

int WordBreakSpellChecker::CombinationsThenFreqComparator::compare(
    shared_ptr<CombineSuggestionWrapper> o1,
    shared_ptr<CombineSuggestionWrapper> o2)
{
  if (o1->numCombinations != o2->numCombinations) {
    return o2->numCombinations - o1->numCombinations;
  }
  if (o1->combineSuggestion->suggestion->freq !=
      o2->combineSuggestion->suggestion->freq) {
    return o1->combineSuggestion->suggestion->freq -
           o2->combineSuggestion->suggestion->freq;
  }
  return 0;
}

WordBreakSpellChecker::SuggestWordArrayWrapper::SuggestWordArrayWrapper(
    std::deque<std::shared_ptr<SuggestWord>> &suggestWords)
    : suggestWords(suggestWords), freqMax(aFreqMax), freqSum(aFreqSum)
{
  int aFreqSum = 0;
  int aFreqMax = 0;
  for (auto sw : suggestWords) {
    aFreqSum += sw->freq;
    aFreqMax = max(aFreqMax, sw->freq);
  }
}

WordBreakSpellChecker::CombineSuggestionWrapper::CombineSuggestionWrapper(
    shared_ptr<CombineSuggestion> combineSuggestion, int numCombinations)
    : combineSuggestion(combineSuggestion), numCombinations(numCombinations)
{
}
} // namespace org::apache::lucene::search::spell