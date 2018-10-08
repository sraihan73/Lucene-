using namespace std;

#include "TopGroupsCollector.h"

namespace org::apache::lucene::search::grouping
{
using Sort = org::apache::lucene::search::Sort;
using TopDocs = org::apache::lucene::search::TopDocs;
using TopDocsCollector = org::apache::lucene::search::TopDocsCollector;
using TopFieldCollector = org::apache::lucene::search::TopFieldCollector;
using TopScoreDocCollector = org::apache::lucene::search::TopScoreDocCollector;
} // namespace org::apache::lucene::search::grouping