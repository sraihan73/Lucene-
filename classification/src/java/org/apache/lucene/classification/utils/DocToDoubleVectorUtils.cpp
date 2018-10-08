using namespace std;

#include "DocToDoubleVectorUtils.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::classification::utils
{
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

DocToDoubleVectorUtils::DocToDoubleVectorUtils()
{
  // no public constructors
}

std::deque<optional<double>>
DocToDoubleVectorUtils::toSparseLocalFreqDoubleArray(
    shared_ptr<Terms> docTerms, shared_ptr<Terms> fieldTerms) 
{
  shared_ptr<TermsEnum> fieldTermsEnum = fieldTerms->begin();
  std::deque<optional<double>> freqVector;
  if (docTerms != nullptr && fieldTerms->size() > -1) {
    freqVector =
        std::deque<optional<double>>(static_cast<int>(fieldTerms->size()));
    int i = 0;
    shared_ptr<TermsEnum> docTermsEnum = docTerms->begin();
    shared_ptr<BytesRef> term;
    while ((term = fieldTermsEnum->next()) != nullptr) {
      TermsEnum::SeekStatus seekStatus = docTermsEnum->seekCeil(term);
      if (seekStatus.equals(TermsEnum::SeekStatus::END)) {
        docTermsEnum = docTerms->begin();
      }
      if (seekStatus.equals(TermsEnum::SeekStatus::FOUND)) {
        int64_t termFreqLocal =
            docTermsEnum->totalTermFreq(); // the total number of occurrences of
                                           // this term in the given document
        freqVector[i] = static_cast<int64_t>(termFreqLocal).doubleValue();
      } else {
        freqVector[i] = 0;
      }
      i++;
    }
  }
  return freqVector;
}

std::deque<optional<double>>
DocToDoubleVectorUtils::toDenseLocalFreqDoubleArray(
    shared_ptr<Terms> docTerms) 
{
  std::deque<optional<double>> freqVector;
  if (docTerms != nullptr) {
    freqVector =
        std::deque<optional<double>>(static_cast<int>(docTerms->size()));
    int i = 0;
    shared_ptr<TermsEnum> docTermsEnum = docTerms->begin();

    while (docTermsEnum->next() != nullptr) {
      int64_t termFreqLocal =
          docTermsEnum->totalTermFreq(); // the total number of occurrences of
                                         // this term in the given document
      freqVector[i] = static_cast<int64_t>(termFreqLocal).doubleValue();
      i++;
    }
  }
  return freqVector;
}
} // namespace org::apache::lucene::classification::utils