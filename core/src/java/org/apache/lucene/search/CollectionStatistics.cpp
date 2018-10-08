using namespace std;

#include "CollectionStatistics.h"

namespace org::apache::lucene::search
{
using IndexReader = org::apache::lucene::index::IndexReader;
using Terms = org::apache::lucene::index::Terms;

CollectionStatistics::CollectionStatistics(const wstring &field,
                                           int64_t maxDoc, int64_t docCount,
                                           int64_t sumTotalTermFreq,
                                           int64_t sumDocFreq)
    : field(field), maxDoc(maxDoc), docCount(docCount),
      sumTotalTermFreq(sumTotalTermFreq), sumDocFreq(sumDocFreq)
{
  assert(maxDoc >= 0);
  assert(docCount >= -1 &&
         docCount <= maxDoc); // #docs with field must be <= #docs
  assert(sumDocFreq == -1 ||
         sumDocFreq >= docCount); // #postings must be >= #docs with field
  assert(sumTotalTermFreq == -1 ||
         sumTotalTermFreq >= sumDocFreq); // #positions must be >= #postings
}

wstring CollectionStatistics::field() { return field_; }

int64_t CollectionStatistics::maxDoc() { return maxDoc_; }

int64_t CollectionStatistics::docCount() { return docCount_; }

int64_t CollectionStatistics::sumTotalTermFreq() { return sumTotalTermFreq_; }

int64_t CollectionStatistics::sumDocFreq() { return sumDocFreq_; }
} // namespace org::apache::lucene::search