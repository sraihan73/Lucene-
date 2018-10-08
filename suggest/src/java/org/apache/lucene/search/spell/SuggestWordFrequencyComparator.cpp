using namespace std;

#include "SuggestWordFrequencyComparator.h"

namespace org::apache::lucene::search::spell
{

SuggestWordFrequencyComparator::SuggestWordFrequencyComparator() {}

int SuggestWordFrequencyComparator::compare(shared_ptr<SuggestWord> first,
                                            shared_ptr<SuggestWord> second)
{
  // first criteria: the frequency
  if (first->freq > second->freq) {
    return 1;
  }
  if (first->freq < second->freq) {
    return -1;
  }

  // second criteria (if first criteria is equal): the score
  if (first->score > second->score) {
    return 1;
  }
  if (first->score < second->score) {
    return -1;
  }
  // third criteria: term text
  return second->string.compare(first->string);
}
} // namespace org::apache::lucene::search::spell