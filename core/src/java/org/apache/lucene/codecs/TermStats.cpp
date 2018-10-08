using namespace std;

#include "TermStats.h"

namespace org::apache::lucene::codecs
{
using TermsEnum = org::apache::lucene::index::TermsEnum;

TermStats::TermStats(int docFreq, int64_t totalTermFreq)
    : docFreq(docFreq), totalTermFreq(totalTermFreq)
{
}
} // namespace org::apache::lucene::codecs