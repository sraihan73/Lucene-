using namespace std;

#include "FreqProxTermsWriter.h"

namespace org::apache::lucene::index
{
using FieldsConsumer = org::apache::lucene::codecs::FieldsConsumer;
using CollectionUtil = org::apache::lucene::util::CollectionUtil;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOUtils = org::apache::lucene::util::IOUtils;

FreqProxTermsWriter::FreqProxTermsWriter(
    shared_ptr<DocumentsWriterPerThread> docWriter,
    shared_ptr<TermsHash> termVectors)
    : TermsHash(docWriter, true, termVectors)
{
}

void FreqProxTermsWriter::applyDeletes(
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Fields> fields) 
{
  // Process any pending Term deletes for this newly
  // flushed segment:
  if (state->segUpdates != nullptr &&
      state->segUpdates->deleteTerms.size() > 0) {
    unordered_map<std::shared_ptr<Term>, int> &segDeletes =
        state->segUpdates->deleteTerms;
    deque<std::shared_ptr<Term>> deleteTerms =
        deque<std::shared_ptr<Term>>(segDeletes.keySet());
    sort(deleteTerms.begin(), deleteTerms.end());
    wstring lastField = L"";
    shared_ptr<TermsEnum> termsEnum = nullptr;
    shared_ptr<PostingsEnum> postingsEnum = nullptr;
    for (auto deleteTerm : deleteTerms) {
      if (deleteTerm->field() == lastField == false) {
        lastField = deleteTerm->field();
        shared_ptr<Terms> terms = fields->terms(lastField);
        if (terms != nullptr) {
          termsEnum = terms->begin();
        } else {
          termsEnum.reset();
        }
      }

      if (termsEnum != nullptr && termsEnum->seekExact(deleteTerm->bytes())) {
        postingsEnum = termsEnum->postings(postingsEnum, 0);
        int delDocLimit = segDeletes[deleteTerm];
        assert(delDocLimit < PostingsEnum::NO_MORE_DOCS);
        while (true) {
          int doc = postingsEnum->nextDoc();
          if (doc < delDocLimit) {
            if (state->liveDocs == nullptr) {
              state->liveDocs =
                  make_shared<FixedBitSet>(state->segmentInfo->maxDoc());
              state->liveDocs->set(0, state->segmentInfo->maxDoc());
            }
            if (state->liveDocs->get(doc)) {
              state->delCountOnFlush++;
              state->liveDocs->clear(doc);
            }
          } else {
            break;
          }
        }
      }
    }
  }
}

void FreqProxTermsWriter::flush(
    unordered_map<wstring, std::shared_ptr<TermsHashPerField>> &fieldsToFlush,
    shared_ptr<SegmentWriteState> state,
    shared_ptr<Sorter::DocMap> sortMap) 
{
  TermsHash::flush(fieldsToFlush, state, sortMap);

  // Gather all fields that saw any postings:
  deque<std::shared_ptr<FreqProxTermsWriterPerField>> allFields =
      deque<std::shared_ptr<FreqProxTermsWriterPerField>>();

  for (auto f : fieldsToFlush) {
    shared_ptr<FreqProxTermsWriterPerField> *const perField =
        std::static_pointer_cast<FreqProxTermsWriterPerField>(f->second);
    if (perField->bytesHash->size() > 0) {
      perField->sortPostings();
      assert(perField->fieldInfo->getIndexOptions() != IndexOptions::NONE);
      allFields.push_back(perField);
    }
  }

  // Sort by field name
  CollectionUtil::introSort(allFields);

  shared_ptr<Fields> fields = make_shared<FreqProxFields>(allFields);
  applyDeletes(state, fields);
  if (sortMap != nullptr) {
    fields = make_shared<SortingLeafReader::SortingFields>(
        fields, state->fieldInfos, sortMap);
  }

  shared_ptr<FieldsConsumer> consumer =
      state->segmentInfo->getCodec()->postingsFormat()->fieldsConsumer(state);
  bool success = false;
  try {
    consumer->write(fields);
    success = true;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({consumer});
    } else {
      IOUtils::closeWhileHandlingException({consumer});
    }
  }
}

shared_ptr<TermsHashPerField>
FreqProxTermsWriter::addField(shared_ptr<FieldInvertState> invertState,
                              shared_ptr<FieldInfo> fieldInfo)
{
  return make_shared<FreqProxTermsWriterPerField>(
      invertState, shared_from_this(), fieldInfo,
      nextTermsHash->addField(invertState, fieldInfo));
}
} // namespace org::apache::lucene::index