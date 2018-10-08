using namespace std;

#include "CompletionsTermsReader.h"

namespace org::apache::lucene::search::suggest::document
{
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;

CompletionsTermsReader::CompletionsTermsReader(shared_ptr<IndexInput> dictIn,
                                               int64_t offset,
                                               int64_t minWeight,
                                               int64_t maxWeight,
                                               char type) 
    : minWeight(minWeight), maxWeight(maxWeight), type(type), dictIn(dictIn),
      offset(offset)
{
  assert(minWeight <= maxWeight);
  assert(offset >= 0LL && offset < dictIn->length());
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<NRTSuggester> CompletionsTermsReader::suggester() 
{
  if (suggester_ == nullptr) {
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (org.apache.lucene.store.IndexInput
    // dictClone = dictIn.clone())
    {
      org::apache::lucene::store::IndexInput dictClone = dictIn->clone();
      dictClone->seek(offset);
      suggester_ = NRTSuggester::load(dictClone);
    }
  }
  return suggester_;
}

int64_t CompletionsTermsReader::ramBytesUsed()
{
  return (suggester_ != nullptr) ? suggester_->ramBytesUsed() : 0;
}

shared_ptr<deque<std::shared_ptr<Accountable>>>
CompletionsTermsReader::getChildResources()
{
  return Collections::emptyList();
}
} // namespace org::apache::lucene::search::suggest::document