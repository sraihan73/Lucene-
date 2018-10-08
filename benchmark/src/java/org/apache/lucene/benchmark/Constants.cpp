using namespace std;

#include "Constants.h"

namespace org::apache::lucene::benchmark
{

std::deque<optional<bool>> Constants::BOOLEANS = {Boolean::FALSE,
                                                   Boolean::TRUE};
}