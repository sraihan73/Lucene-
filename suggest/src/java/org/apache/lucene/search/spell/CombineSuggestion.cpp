using namespace std;

#include "CombineSuggestion.h"

namespace org::apache::lucene::search::spell
{

CombineSuggestion::CombineSuggestion(shared_ptr<SuggestWord> suggestion,
                                     std::deque<int> &originalTermIndexes)
    : originalTermIndexes(originalTermIndexes), suggestion(suggestion)
{
}
} // namespace org::apache::lucene::search::spell