using namespace std;

#include "FieldsConsumer.h"
#include "../index/Fields.h"
#include "../index/MappedMultiFields.h"
#include "../index/MergeState.h"
#include "../index/MultiFields.h"
#include "../index/ReaderSlice.h"
#include "FieldsProducer.h"

namespace org::apache::lucene::codecs
{
using Fields = org::apache::lucene::index::Fields;
using MappedMultiFields = org::apache::lucene::index::MappedMultiFields;
using MergeState = org::apache::lucene::index::MergeState;
using MultiFields = org::apache::lucene::index::MultiFields;
using ReaderSlice = org::apache::lucene::index::ReaderSlice;

FieldsConsumer::FieldsConsumer() {}

void FieldsConsumer::merge(shared_ptr<MergeState> mergeState) 
{
  const deque<std::shared_ptr<Fields>> fields =
      deque<std::shared_ptr<Fields>>();
  const deque<std::shared_ptr<ReaderSlice>> slices =
      deque<std::shared_ptr<ReaderSlice>>();

  int docBase = 0;

  for (int readerIndex = 0; readerIndex < mergeState->fieldsProducers.size();
       readerIndex++) {
    shared_ptr<FieldsProducer> *const f =
        mergeState->fieldsProducers[readerIndex];

    constexpr int maxDoc = mergeState->maxDocs[readerIndex];
    f->checkIntegrity();
    slices.push_back(make_shared<ReaderSlice>(docBase, maxDoc, readerIndex));
    fields.push_back(f);
    docBase += maxDoc;
  }

  shared_ptr<Fields> mergedFields = make_shared<MappedMultiFields>(
      mergeState,
      make_shared<MultiFields>(fields.toArray(Fields::EMPTY_ARRAY),
                               slices.toArray(ReaderSlice::EMPTY_ARRAY)));
  write(mergedFields);
}
} // namespace org::apache::lucene::codecs