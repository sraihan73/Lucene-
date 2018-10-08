using namespace std;

#include "LongToEnglishContentSource.h"
#include "DocData.h"
#include "NoMoreDataException.h"

namespace org::apache::lucene::benchmark::byTask::feeds
{
using com::ibm::icu::text::RuleBasedNumberFormat;

LongToEnglishContentSource::~LongToEnglishContentSource() {}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<DocData> LongToEnglishContentSource::getNextDocData(
    shared_ptr<DocData> docData) 
{
  docData->clear();
  // store the current counter to avoid synchronization later on
  int64_t curCounter;
  // C++ TODO: Multithread locking on 'this' is not converted to native C++:
  synchronized(shared_from_this())
  {
    curCounter = counter;
    if (counter == numeric_limits<int64_t>::max()) {
      counter = numeric_limits<int64_t>::min(); // loop around
    } else {
      ++counter;
    }
  }

  docData->setBody(rnbf->format(curCounter));
  docData->setName(L"doc_" + to_wstring(curCounter));
  docData->setTitle(L"title_" + to_wstring(curCounter));
  docData->setDate(Date());
  return docData;
}

void LongToEnglishContentSource::resetInputs() 
{
  counter = numeric_limits<int64_t>::min() + 10;
}
} // namespace org::apache::lucene::benchmark::byTask::feeds