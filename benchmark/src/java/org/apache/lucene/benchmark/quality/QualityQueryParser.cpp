using namespace std;

#include "QualityQueryParser.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "QualityQuery.h"

namespace org::apache::lucene::benchmark::quality
{
using ParseException =
    org::apache::lucene::queryparser::classic::ParseException;
using Query = org::apache::lucene::search::Query;
} // namespace org::apache::lucene::benchmark::quality