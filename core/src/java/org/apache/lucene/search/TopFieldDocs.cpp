using namespace std;

#include "TopFieldDocs.h"

namespace org::apache::lucene::search
{

TopFieldDocs::TopFieldDocs(int64_t totalHits,
                           std::deque<std::shared_ptr<ScoreDoc>> &scoreDocs,
                           std::deque<std::shared_ptr<SortField>> &fields,
                           float maxScore)
    : TopDocs(totalHits, scoreDocs, maxScore)
{
  this->fields = fields;
}
} // namespace org::apache::lucene::search