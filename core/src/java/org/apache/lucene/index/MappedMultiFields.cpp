using namespace std;

#include "MappedMultiFields.h"

namespace org::apache::lucene::index
{
//    import static org.apache.lucene.index.FilterLeafReader.FilterFields;
//    import static org.apache.lucene.index.FilterLeafReader.FilterTerms;
//    import static org.apache.lucene.index.FilterLeafReader.FilterTermsEnum;

MappedMultiFields::MappedMultiFields(shared_ptr<MergeState> mergeState,
                                     shared_ptr<MultiFields> multiFields)
    : FilterFields(multiFields), mergeState(mergeState)
{
}

shared_ptr<Terms>
MappedMultiFields::terms(const wstring &field) 
{
  shared_ptr<MultiTerms> terms =
      std::static_pointer_cast<MultiTerms>(in_::terms(field));
  if (terms == nullptr) {
    return nullptr;
  } else {
    return make_shared<MappedMultiTerms>(field, mergeState, terms);
  }
}

MappedMultiFields::MappedMultiTerms::MappedMultiTerms(
    const wstring &field, shared_ptr<MergeState> mergeState,
    shared_ptr<MultiTerms> multiTerms)
    : FilterTerms(multiTerms), mergeState(mergeState), field(field)
{
}

shared_ptr<TermsEnum>
MappedMultiFields::MappedMultiTerms::iterator() 
{
  shared_ptr<TermsEnum> iterator = in_::begin();
  if (iterator == TermsEnum::EMPTY) {
    // LUCENE-6826
    return TermsEnum::EMPTY;
  } else {
    return make_shared<MappedMultiTermsEnum>(
        field, mergeState, std::static_pointer_cast<MultiTermsEnum>(iterator));
  }
}

int64_t MappedMultiFields::MappedMultiTerms::size() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t
MappedMultiFields::MappedMultiTerms::getSumTotalTermFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t
MappedMultiFields::MappedMultiTerms::getSumDocFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

int MappedMultiFields::MappedMultiTerms::getDocCount() 
{
  throw make_shared<UnsupportedOperationException>();
}

MappedMultiFields::MappedMultiTermsEnum::MappedMultiTermsEnum(
    const wstring &field, shared_ptr<MergeState> mergeState,
    shared_ptr<MultiTermsEnum> multiTermsEnum)
    : FilterTermsEnum(multiTermsEnum), mergeState(mergeState), field(field)
{
}

int MappedMultiFields::MappedMultiTermsEnum::docFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t
MappedMultiFields::MappedMultiTermsEnum::totalTermFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

shared_ptr<PostingsEnum> MappedMultiFields::MappedMultiTermsEnum::postings(
    shared_ptr<PostingsEnum> reuse, int flags) 
{
  shared_ptr<MappingMultiPostingsEnum> mappingDocsAndPositionsEnum;
  if (std::dynamic_pointer_cast<MappingMultiPostingsEnum>(reuse) != nullptr) {
    shared_ptr<MappingMultiPostingsEnum> postings =
        std::static_pointer_cast<MappingMultiPostingsEnum>(reuse);
    if (postings->field.equals(this->field)) {
      mappingDocsAndPositionsEnum = postings;
    } else {
      mappingDocsAndPositionsEnum =
          make_shared<MappingMultiPostingsEnum>(field, mergeState);
    }
  } else {
    mappingDocsAndPositionsEnum =
        make_shared<MappingMultiPostingsEnum>(field, mergeState);
  }

  shared_ptr<MultiPostingsEnum> docsAndPositionsEnum =
      std::static_pointer_cast<MultiPostingsEnum>(in_::postings(
          mappingDocsAndPositionsEnum->multiDocsAndPositionsEnum, flags));
  mappingDocsAndPositionsEnum->reset(docsAndPositionsEnum);
  return mappingDocsAndPositionsEnum;
}
} // namespace org::apache::lucene::index