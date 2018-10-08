using namespace std;

#include "TooManyBasicQueries.h"

namespace org::apache::lucene::queryparser::surround::query
{

TooManyBasicQueries::TooManyBasicQueries(int maxBasicQueries)
    : java::io::IOException(L"Exceeded maximum of " + maxBasicQueries +
                            L" basic queries.")
{
}
} // namespace org::apache::lucene::queryparser::surround::query