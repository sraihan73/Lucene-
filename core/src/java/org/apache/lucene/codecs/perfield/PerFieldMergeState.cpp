using namespace std;

#include "PerFieldMergeState.h"

namespace org::apache::lucene::codecs::perfield
{
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using FieldInfos = org::apache::lucene::index::FieldInfos;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using MergeState = org::apache::lucene::index::MergeState;
using Terms = org::apache::lucene::index::Terms;

PerFieldMergeState::PerFieldMergeState(shared_ptr<MergeState> in_)
    : in_(in_), orgMergeFieldInfos(in_->mergeFieldInfos),
      orgFieldInfos(
          std::deque<std::shared_ptr<FieldInfos>>(in_->fieldInfos.size())),
      orgFieldsProducers(std::deque<std::shared_ptr<FieldsProducer>>(
          in_->fieldsProducers.size()))
{

  System::arraycopy(in_->fieldInfos, 0, this->orgFieldInfos, 0,
                    this->orgFieldInfos.size());
  System::arraycopy(in_->fieldsProducers, 0, this->orgFieldsProducers, 0,
                    this->orgFieldsProducers.size());
}

shared_ptr<MergeState>
PerFieldMergeState::apply(shared_ptr<deque<wstring>> fields)
{
  in_->mergeFieldInfos =
      make_shared<FilterFieldInfos>(orgMergeFieldInfos, fields);
  for (int i = 0; i < orgFieldInfos.size(); i++) {
    in_->fieldInfos[i] =
        make_shared<FilterFieldInfos>(orgFieldInfos[i], fields);
  }
  for (int i = 0; i < orgFieldsProducers.size(); i++) {
    in_->fieldsProducers[i] =
        make_shared<FilterFieldsProducer>(orgFieldsProducers[i], fields);
  }
  return in_;
}

shared_ptr<MergeState> PerFieldMergeState::reset()
{
  in_->mergeFieldInfos = orgMergeFieldInfos;
  System::arraycopy(orgFieldInfos, 0, in_->fieldInfos, 0,
                    in_->fieldInfos.size());
  System::arraycopy(orgFieldsProducers, 0, in_->fieldsProducers, 0,
                    in_->fieldsProducers.size());
  return in_;
}

PerFieldMergeState::FilterFieldInfos::FilterFieldInfos(
    shared_ptr<FieldInfos> src, shared_ptr<deque<wstring>> filterFields)
    : org::apache::lucene::index::FieldInfos(toArray(src)),
      filteredNames(unordered_set<>(filterFields)),
      filtered(deque<>(filterFields->size())), filteredHasVectors(hasVectors),
      filteredHasProx(hasProx), filteredHasPayloads(hasPayloads),
      filteredHasOffsets(hasOffsets), filteredHasFreq(hasFreq),
      filteredHasNorms(hasNorms), filteredHasDocValues(hasDocValues),
      filteredHasPointValues(hasPointValues)
{
  // Copy all the input FieldInfo objects since the field numbering must be kept
  // consistent

  bool hasVectors = false;
  bool hasProx = false;
  bool hasPayloads = false;
  bool hasOffsets = false;
  bool hasFreq = false;
  bool hasNorms = false;
  bool hasDocValues = false;
  bool hasPointValues = false;

  for (auto fi : src) {
    if (filterFields->contains(fi->name)) {
      this->filtered.push_back(fi);
      hasVectors |= fi->hasVectors();
      hasProx |= fi->getIndexOptions().compareTo(
                     IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
      hasFreq |= fi->getIndexOptions() != IndexOptions::DOCS;
      hasOffsets |=
          fi->getIndexOptions().compareTo(
              IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
      hasNorms |= fi->hasNorms();
      hasDocValues |= fi->getDocValuesType() != DocValuesType::NONE;
      hasPayloads |= fi->hasPayloads();
      hasPointValues |= (fi->getPointDimensionCount() != 0);
    }
  }
}

std::deque<std::shared_ptr<FieldInfo>>
PerFieldMergeState::FilterFieldInfos::toArray(shared_ptr<FieldInfos> src)
{
  std::deque<std::shared_ptr<FieldInfo>> res(src->size());
  int i = 0;
  for (auto fi : src) {
    res[i++] = fi;
  }
  return res;
}

shared_ptr<Iterator<std::shared_ptr<FieldInfo>>>
PerFieldMergeState::FilterFieldInfos::iterator()
{
  return filtered.begin();
}

bool PerFieldMergeState::FilterFieldInfos::hasFreq() { return filteredHasFreq; }

bool PerFieldMergeState::FilterFieldInfos::hasProx() { return filteredHasProx; }

bool PerFieldMergeState::FilterFieldInfos::hasPayloads()
{
  return filteredHasPayloads;
}

bool PerFieldMergeState::FilterFieldInfos::hasOffsets()
{
  return filteredHasOffsets;
}

bool PerFieldMergeState::FilterFieldInfos::hasVectors()
{
  return filteredHasVectors;
}

bool PerFieldMergeState::FilterFieldInfos::hasNorms()
{
  return filteredHasNorms;
}

bool PerFieldMergeState::FilterFieldInfos::hasDocValues()
{
  return filteredHasDocValues;
}

bool PerFieldMergeState::FilterFieldInfos::hasPointValues()
{
  return filteredHasPointValues;
}

int PerFieldMergeState::FilterFieldInfos::size() { return filtered.size(); }

shared_ptr<FieldInfo>
PerFieldMergeState::FilterFieldInfos::fieldInfo(const wstring &fieldName)
{
  if (!filteredNames->contains(fieldName)) {
    // Throw IAE to be consistent with fieldInfo(int) which throws it as well on
    // invalid numbers
    throw invalid_argument(L"The field named '" + fieldName +
                           L"' is not accessible in the current " +
                           L"merge context, available ones are: " +
                           filteredNames);
  }
  return FieldInfos::fieldInfo(fieldName);
}

shared_ptr<FieldInfo>
PerFieldMergeState::FilterFieldInfos::fieldInfo(int fieldNumber)
{
  shared_ptr<FieldInfo> res = FieldInfos::fieldInfo(fieldNumber);
  if (!filteredNames->contains(res->name)) {
    throw invalid_argument(
        L"The field named '" + res->name + L"' numbered '" +
        to_wstring(fieldNumber) + L"' is not " +
        L"accessible in the current merge context, available ones are: " +
        filteredNames);
  }
  return res;
}

PerFieldMergeState::FilterFieldsProducer::FilterFieldsProducer(
    shared_ptr<FieldsProducer> in_,
    shared_ptr<deque<wstring>> filterFields)
    : in_(in_), filtered(deque<>(filterFields))
{
}

int64_t PerFieldMergeState::FilterFieldsProducer::ramBytesUsed()
{
  return in_->ramBytesUsed();
}

shared_ptr<Iterator<wstring>>
PerFieldMergeState::FilterFieldsProducer::iterator()
{
  return filtered.begin();
}

shared_ptr<Terms> PerFieldMergeState::FilterFieldsProducer::terms(
    const wstring &field) 
{
  if (!find(filtered.begin(), filtered.end(), field) != filtered.end()) {
    throw invalid_argument(L"The field named '" + field +
                           L"' is not accessible in the current " +
                           L"merge context, available ones are: " + filtered);
  }
  return in_->terms(field);
}

int PerFieldMergeState::FilterFieldsProducer::size() { return filtered.size(); }

PerFieldMergeState::FilterFieldsProducer::~FilterFieldsProducer()
{
  in_->close();
}

void PerFieldMergeState::FilterFieldsProducer::checkIntegrity() throw(
    IOException)
{
  in_->checkIntegrity();
}
} // namespace org::apache::lucene::codecs::perfield