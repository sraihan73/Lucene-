using namespace std;

#include "MultiFields.h"

namespace org::apache::lucene::index
{
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using MergedIterator = org::apache::lucene::util::MergedIterator;

shared_ptr<Fields>
MultiFields::getFields(shared_ptr<IndexReader> reader) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
  switch (leaves.size()) {
  case 1:
    // already an atomic reader / reader with one leave
    return make_shared<LeafReaderFields>(leaves[0]->reader());
  default:
    const deque<std::shared_ptr<Fields>> fields =
        deque<std::shared_ptr<Fields>>(leaves.size());
    const deque<std::shared_ptr<ReaderSlice>> slices =
        deque<std::shared_ptr<ReaderSlice>>(leaves.size());
    for (auto ctx : leaves) {
      shared_ptr<LeafReader> *const r = ctx->reader();
      shared_ptr<Fields> *const f = make_shared<LeafReaderFields>(r);
      fields.push_back(f);
      slices.push_back(make_shared<ReaderSlice>(ctx->docBase, r->maxDoc(),
                                                fields.size() - 1));
    }
    if (fields.size() == 1) {
      return fields[0];
    } else {
      return make_shared<MultiFields>(fields.toArray(Fields::EMPTY_ARRAY),
                                      slices.toArray(ReaderSlice::EMPTY_ARRAY));
    }
  }
}

shared_ptr<Bits> MultiFields::getLiveDocs(shared_ptr<IndexReader> reader)
{
  if (reader->hasDeletions()) {
    const deque<std::shared_ptr<LeafReaderContext>> leaves = reader->leaves();
    constexpr int size = leaves.size();
    assert((size > 0, L"A reader with deletions must have at least one leave"));
    if (size == 1) {
      return leaves[0]->reader().getLiveDocs();
    }
    std::deque<std::shared_ptr<Bits>> liveDocs(size);
    const std::deque<int> starts = std::deque<int>(size + 1);
    for (int i = 0; i < size; i++) {
      // record all liveDocs, even if they are null
      shared_ptr<LeafReaderContext> *const ctx = leaves[i];
      liveDocs[i] = ctx->reader()->getLiveDocs();
      starts[i] = ctx->docBase;
    }
    starts[size] = reader->maxDoc();
    return make_shared<MultiBits>(liveDocs, starts, true);
  } else {
    return nullptr;
  }
}

shared_ptr<Terms> MultiFields::getTerms(shared_ptr<IndexReader> r,
                                        const wstring &field) 
{
  const deque<std::shared_ptr<LeafReaderContext>> leaves = r->leaves();
  if (leaves.size() == 1) {
    return leaves[0]->reader().terms(field);
  }

  const deque<std::shared_ptr<Terms>> termsPerLeaf =
      deque<std::shared_ptr<Terms>>(leaves.size());
  const deque<std::shared_ptr<ReaderSlice>> slicePerLeaf =
      deque<std::shared_ptr<ReaderSlice>>(leaves.size());

  for (int leafIdx = 0; leafIdx < leaves.size(); leafIdx++) {
    shared_ptr<LeafReaderContext> ctx = leaves[leafIdx];
    shared_ptr<Terms> subTerms = ctx->reader()->terms(field);
    if (subTerms != nullptr) {
      termsPerLeaf.push_back(subTerms);
      slicePerLeaf.push_back(
          make_shared<ReaderSlice>(ctx->docBase, r->maxDoc(), leafIdx - 1));
    }
  }

  if (termsPerLeaf.empty()) {
    return nullptr;
  } else {
    return make_shared<MultiTerms>(
        termsPerLeaf.toArray(Terms::EMPTY_ARRAY),
        slicePerLeaf.toArray(ReaderSlice::EMPTY_ARRAY));
  }
}

shared_ptr<PostingsEnum>
MultiFields::getTermDocsEnum(shared_ptr<IndexReader> r, const wstring &field,
                             shared_ptr<BytesRef> term) 
{
  return getTermDocsEnum(r, field, term, PostingsEnum::FREQS);
}

shared_ptr<PostingsEnum>
MultiFields::getTermDocsEnum(shared_ptr<IndexReader> r, const wstring &field,
                             shared_ptr<BytesRef> term,
                             int flags) 
{
  assert(field != L"");
  assert(term != nullptr);
  shared_ptr<Terms> *const terms = getTerms(r, field);
  if (terms != nullptr) {
    shared_ptr<TermsEnum> *const termsEnum = terms->begin();
    if (termsEnum->seekExact(term)) {
      return termsEnum->postings(nullptr, flags);
    }
  }
  return nullptr;
}

shared_ptr<PostingsEnum>
MultiFields::getTermPositionsEnum(shared_ptr<IndexReader> r,
                                  const wstring &field,
                                  shared_ptr<BytesRef> term) 
{
  return getTermPositionsEnum(r, field, term, PostingsEnum::ALL);
}

shared_ptr<PostingsEnum> MultiFields::getTermPositionsEnum(
    shared_ptr<IndexReader> r, const wstring &field, shared_ptr<BytesRef> term,
    int flags) 
{
  assert(field != L"");
  assert(term != nullptr);
  shared_ptr<Terms> *const terms = getTerms(r, field);
  if (terms != nullptr) {
    shared_ptr<TermsEnum> *const termsEnum = terms->begin();
    if (termsEnum->seekExact(term)) {
      return termsEnum->postings(nullptr, flags);
    }
  }
  return nullptr;
}

MultiFields::MultiFields(std::deque<std::shared_ptr<Fields>> &subs,
                         std::deque<std::shared_ptr<ReaderSlice>> &subSlices)
    : subs(subs), subSlices(subSlices)
{
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) @Override public
// java.util.Iterator<std::wstring> iterator()
shared_ptr<Iterator<wstring>> MultiFields::iterator()
{
  std::deque<Iterator<wstring>> subIterators =
      std::deque<std::shared_ptr<Iterator>>(subs.size());
  for (int i = 0; i < subs.size(); i++) {
    subIterators[i] = subs[i]->begin();
  }
  return make_shared<MergedIterator<>>(subIterators);
}

shared_ptr<Terms> MultiFields::terms(const wstring &field) 
{
  shared_ptr<Terms> result = terms_[field];
  if (result != nullptr) {
    return result;
  }

  // Lazy init: first time this field is requested, we
  // create & add to terms:
  const deque<std::shared_ptr<Terms>> subs2 = deque<std::shared_ptr<Terms>>();
  const deque<std::shared_ptr<ReaderSlice>> slices2 =
      deque<std::shared_ptr<ReaderSlice>>();

  // Gather all sub-readers that share this field
  for (int i = 0; i < subs.size(); i++) {
    shared_ptr<Terms> *const terms = subs[i]->terms(field);
    if (terms != nullptr) {
      subs2.push_back(terms);
      slices2.push_back(subSlices[i]);
    }
  }
  if (subs2.empty()) {
    result.reset();
    // don't cache this case with an unbounded cache, since the number of fields
    // that don't exist is unbounded.
  } else {
    result = make_shared<MultiTerms>(subs2.toArray(Terms::EMPTY_ARRAY),
                                     slices2.toArray(ReaderSlice::EMPTY_ARRAY));
    terms_.emplace(field, result);
  }

  return result;
}

int MultiFields::size() { return -1; }

shared_ptr<FieldInfos>
MultiFields::getMergedFieldInfos(shared_ptr<IndexReader> reader)
{
  const wstring softDeletesField =
      reader->leaves()
          .stream()
          .map_obj(
              [&](any l) { l::reader().getFieldInfos().getSoftDeletesField(); })
          .filter(Objects::nonNull)
          .findAny()
          .orElse(nullptr);
  shared_ptr<FieldInfos::Builder> *const builder =
      make_shared<FieldInfos::Builder>(
          make_shared<FieldInfos::FieldNumbers>(softDeletesField));
  for (auto ctx : reader->leaves()) {
    builder->add(ctx->reader()->getFieldInfos());
  }
  return builder->finish();
}

shared_ptr<deque<wstring>>
MultiFields::getIndexedFields(shared_ptr<IndexReader> reader)
{
  shared_ptr<deque<wstring>> *const fields = unordered_set<wstring>();
  for (auto fieldInfo : getMergedFieldInfos(reader)) {
    if (fieldInfo->getIndexOptions() != IndexOptions::NONE) {
      fields->add(fieldInfo->name);
    }
  }
  return fields;
}

MultiFields::LeafReaderFields::LeafReaderFields(
    shared_ptr<LeafReader> leafReader)
    : leafReader(leafReader), indexedFields(deque<>())
{
  for (auto fieldInfo : leafReader->getFieldInfos()) {
    if (fieldInfo->getIndexOptions() != IndexOptions::NONE) {
      indexedFields.push_back(fieldInfo->name);
    }
  }
  sort(indexedFields.begin(), indexedFields.end());
}

shared_ptr<Iterator<wstring>> MultiFields::LeafReaderFields::iterator()
{
  return Collections::unmodifiableList(indexedFields).begin();
}

int MultiFields::LeafReaderFields::size() { return indexedFields.size(); }

shared_ptr<Terms>
MultiFields::LeafReaderFields::terms(const wstring &field) 
{
  return leafReader->terms(field);
}
} // namespace org::apache::lucene::index