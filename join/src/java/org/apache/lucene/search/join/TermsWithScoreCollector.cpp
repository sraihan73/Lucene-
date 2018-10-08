using namespace std;

#include "TermsWithScoreCollector.h"

namespace org::apache::lucene::search::join
{
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Scorer = org::apache::lucene::search::Scorer;
using ArrayUtil = org::apache::lucene::util::ArrayUtil;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;

TermsWithScoreCollector<DV>::SV::SV(
    Function<std::shared_ptr<BinaryDocValues>> docValuesCall,
    ScoreMode scoreMode)
    : TermsWithScoreCollector<org::apache::lucene::index::BinaryDocValues>(
          docValuesCall, scoreMode)
{
}

void TermsWithScoreCollector<DV>::SV::collect(int doc) 
{
  shared_ptr<BytesRef> value;
  if (docValues->advanceExact(doc)) {
    value = docValues->binaryValue();
  } else {
    value = make_shared<BytesRef>(BytesRef::EMPTY_BYTES);
  }
  int ord = collectedTerms->add(value);
  if (ord < 0) {
    ord = -ord - 1;
  } else {
    if (ord >= scoreSums.size()) {
      int begin = scoreSums.size();
      scoreSums = ArrayUtil::grow(scoreSums);
      if (scoreMode == ScoreMode::Min) {
        Arrays::fill(scoreSums, begin, scoreSums.size(),
                     numeric_limits<float>::infinity());
      } else if (scoreMode == ScoreMode::Max) {
        Arrays::fill(scoreSums, begin, scoreSums.size(),
                     -numeric_limits<float>::infinity());
      }
    }
  }

  float current = scorer->score();
  float existing = scoreSums[ord];
  if (Float::compare(existing, 0.0f) == 0) {
    scoreSums[ord] = current;
  } else {
    switch (scoreMode) {
    case org::apache::lucene::search::join::ScoreMode::Total:
      scoreSums[ord] = scoreSums[ord] + current;
      break;
    case org::apache::lucene::search::join::ScoreMode::Min:
      if (current < existing) {
        scoreSums[ord] = current;
      }
      break;
    case org::apache::lucene::search::join::ScoreMode::Max:
      if (current > existing) {
        scoreSums[ord] = current;
      }
      break;
    default:
      throw make_shared<AssertionError>(L"unexpected: " + scoreMode);
    }
  }
}

TermsWithScoreCollector<DV>::SV::Avg::Avg(
    Function<std::shared_ptr<BinaryDocValues>> docValuesCall)
    : SV(docValuesCall, ScoreMode::Avg)
{
}

void TermsWithScoreCollector<DV>::SV::Avg::collect(int doc) 
{
  shared_ptr<BytesRef> value;
  if (docValues->advanceExact(doc)) {
    value = docValues->binaryValue();
  } else {
    value = make_shared<BytesRef>(BytesRef::EMPTY_BYTES);
  }
  int ord = collectedTerms->add(value);
  if (ord < 0) {
    ord = -ord - 1;
  } else {
    if (ord >= scoreSums.size()) {
      scoreSums = ArrayUtil::grow(scoreSums);
      scoreCounts = ArrayUtil::grow(scoreCounts);
    }
  }

  float current = scorer->score();
  float existing = scoreSums[ord];
  if (Float::compare(existing, 0.0f) == 0) {
    scoreSums[ord] = current;
    scoreCounts[ord] = 1;
  } else {
    scoreSums[ord] = scoreSums[ord] + current;
    scoreCounts[ord]++;
  }
}

std::deque<float> TermsWithScoreCollector<DV>::SV::Avg::getScoresPerTerm()
{
  if (scoreCounts.size() > 0) {
    for (int i = 0; i < scoreCounts.size(); i++) {
      scoreSums[i] = scoreSums[i] / scoreCounts[i];
    }
    scoreCounts.clear();
  }
  return scoreSums;
}

TermsWithScoreCollector<DV>::MV::MV(
    Function<std::shared_ptr<SortedSetDocValues>> docValuesCall,
    ScoreMode scoreMode)
    : TermsWithScoreCollector<org::apache::lucene::index::SortedSetDocValues>(
          docValuesCall, scoreMode)
{
}

void TermsWithScoreCollector<DV>::MV::collect(int doc) 
{
  if (docValues->advanceExact(doc)) {
    int64_t ord;
    while ((ord = docValues->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
      int termID = collectedTerms->add(docValues->lookupOrd(ord));
      if (termID < 0) {
        termID = -termID - 1;
      } else {
        if (termID >= scoreSums.size()) {
          int begin = scoreSums.size();
          scoreSums = ArrayUtil::grow(scoreSums);
          if (scoreMode == ScoreMode::Min) {
            Arrays::fill(scoreSums, begin, scoreSums.size(),
                         numeric_limits<float>::infinity());
          } else if (scoreMode == ScoreMode::Max) {
            Arrays::fill(scoreSums, begin, scoreSums.size(),
                         -numeric_limits<float>::infinity());
          }
        }
      }

      switch (scoreMode) {
      case org::apache::lucene::search::join::ScoreMode::Total:
        scoreSums[termID] += scorer->score();
        break;
      case org::apache::lucene::search::join::ScoreMode::Min:
        scoreSums[termID] = min(scoreSums[termID], scorer->score());
        break;
      case org::apache::lucene::search::join::ScoreMode::Max:
        scoreSums[termID] = max(scoreSums[termID], scorer->score());
        break;
      default:
        throw make_shared<AssertionError>(L"unexpected: " + scoreMode);
      }
    }
  }
}

TermsWithScoreCollector<DV>::MV::Avg::Avg(
    Function<std::shared_ptr<SortedSetDocValues>> docValuesCall)
    : MV(docValuesCall, ScoreMode::Avg)
{
}

void TermsWithScoreCollector<DV>::MV::Avg::collect(int doc) 
{
  if (docValues->advanceExact(doc)) {
    int64_t ord;
    while ((ord = docValues->nextOrd()) != SortedSetDocValues::NO_MORE_ORDS) {
      int termID = collectedTerms->add(docValues->lookupOrd(ord));
      if (termID < 0) {
        termID = -termID - 1;
      } else {
        if (termID >= scoreSums.size()) {
          scoreSums = ArrayUtil::grow(scoreSums);
          scoreCounts = ArrayUtil::grow(scoreCounts);
        }
      }

      scoreSums[termID] += scorer->score();
      scoreCounts[termID]++;
    }
  }
}

std::deque<float> TermsWithScoreCollector<DV>::MV::Avg::getScoresPerTerm()
{
  if (scoreCounts.size() > 0) {
    for (int i = 0; i < scoreCounts.size(); i++) {
      scoreSums[i] = scoreSums[i] / scoreCounts[i];
    }
    scoreCounts.clear();
  }
  return scoreSums;
}
} // namespace org::apache::lucene::search::join