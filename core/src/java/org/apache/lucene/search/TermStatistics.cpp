using namespace std;

#include "TermStatistics.h"

namespace org::apache::lucene::search
{
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

TermStatistics::TermStatistics(shared_ptr<BytesRef> term, int64_t docFreq,
                               int64_t totalTermFreq)
    : term(term), docFreq(docFreq), totalTermFreq(totalTermFreq)
{
  assert(docFreq >= 0);
  assert(totalTermFreq == -1 ||
         totalTermFreq >= docFreq); // #positions must be >= #postings
}

shared_ptr<BytesRef> TermStatistics::term() { return term_; }

int64_t TermStatistics::docFreq() { return docFreq_; }

int64_t TermStatistics::totalTermFreq() { return totalTermFreq_; }
} // namespace org::apache::lucene::search