using namespace std;

#include "SerialMergeScheduler.h"

namespace org::apache::lucene::index
{

SerialMergeScheduler::SerialMergeScheduler() {}

// C++ WARNING: The following method was originally marked 'synchronized':
void SerialMergeScheduler::merge(shared_ptr<IndexWriter> writer,
                                 MergeTrigger trigger,
                                 bool newMergesFound) 
{
  while (true) {
    shared_ptr<MergePolicy::OneMerge> merge = writer->getNextMerge();
    if (merge == nullptr) {
      break;
    }
    writer->merge(merge);
  }
}

SerialMergeScheduler::~SerialMergeScheduler() {}
} // namespace org::apache::lucene::index