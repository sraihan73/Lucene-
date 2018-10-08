using namespace std;

#include "FieldType.h"

namespace org::apache::lucene::document
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using DocValuesType = org::apache::lucene::index::DocValuesType;
using IndexOptions = org::apache::lucene::index::IndexOptions;
using IndexableFieldType = org::apache::lucene::index::IndexableFieldType;
using PointValues = org::apache::lucene::index::PointValues;

FieldType::FieldType(shared_ptr<IndexableFieldType> ref)
{
  this->stored_ = ref->stored();
  this->tokenized_ = ref->tokenized();
  this->storeTermVectors_ = ref->storeTermVectors();
  this->storeTermVectorOffsets_ = ref->storeTermVectorOffsets();
  this->storeTermVectorPositions_ = ref->storeTermVectorPositions();
  this->storeTermVectorPayloads_ = ref->storeTermVectorPayloads();
  this->omitNorms_ = ref->omitNorms();
  this->indexOptions_ = ref->indexOptions();
  this->docValuesType_ = ref->docValuesType();
  this->dimensionCount = ref->pointDimensionCount();
  this->dimensionNumBytes = ref->pointNumBytes();
  // Do not copy frozen!
}

FieldType::FieldType() {}

void FieldType::checkIfFrozen()
{
  if (frozen) {
    throw make_shared<IllegalStateException>(
        L"this FieldType is already frozen and cannot be changed");
  }
}

void FieldType::freeze() { this->frozen = true; }

bool FieldType::stored() { return this->stored_; }

void FieldType::setStored(bool value)
{
  checkIfFrozen();
  this->stored_ = value;
}

bool FieldType::tokenized() { return this->tokenized_; }

void FieldType::setTokenized(bool value)
{
  checkIfFrozen();
  this->tokenized_ = value;
}

bool FieldType::storeTermVectors() { return this->storeTermVectors_; }

void FieldType::setStoreTermVectors(bool value)
{
  checkIfFrozen();
  this->storeTermVectors_ = value;
}

bool FieldType::storeTermVectorOffsets()
{
  return this->storeTermVectorOffsets_;
}

void FieldType::setStoreTermVectorOffsets(bool value)
{
  checkIfFrozen();
  this->storeTermVectorOffsets_ = value;
}

bool FieldType::storeTermVectorPositions()
{
  return this->storeTermVectorPositions_;
}

void FieldType::setStoreTermVectorPositions(bool value)
{
  checkIfFrozen();
  this->storeTermVectorPositions_ = value;
}

bool FieldType::storeTermVectorPayloads()
{
  return this->storeTermVectorPayloads_;
}

void FieldType::setStoreTermVectorPayloads(bool value)
{
  checkIfFrozen();
  this->storeTermVectorPayloads_ = value;
}

bool FieldType::omitNorms() { return this->omitNorms_; }

void FieldType::setOmitNorms(bool value)
{
  checkIfFrozen();
  this->omitNorms_ = value;
}

IndexOptions FieldType::indexOptions() { return this->indexOptions_; }

void FieldType::setIndexOptions(IndexOptions value)
{
  checkIfFrozen();
  if (value == nullptr) {
    throw make_shared<NullPointerException>(L"IndexOptions must not be null");
  }
  this->indexOptions_ = value;
}

void FieldType::setDimensions(int dimensionCount, int dimensionNumBytes)
{
  if (dimensionCount < 0) {
    throw invalid_argument(L"dimensionCount must be >= 0; got " +
                           to_wstring(dimensionCount));
  }
  if (dimensionCount > PointValues::MAX_DIMENSIONS) {
    throw invalid_argument(L"dimensionCount must be <= " +
                           to_wstring(PointValues::MAX_DIMENSIONS) + L"; got " +
                           to_wstring(dimensionCount));
  }
  if (dimensionNumBytes < 0) {
    throw invalid_argument(L"dimensionNumBytes must be >= 0; got " +
                           to_wstring(dimensionNumBytes));
  }
  if (dimensionNumBytes > PointValues::MAX_NUM_BYTES) {
    throw invalid_argument(L"dimensionNumBytes must be <= " +
                           to_wstring(PointValues::MAX_NUM_BYTES) + L"; got " +
                           to_wstring(dimensionNumBytes));
  }
  if (dimensionCount == 0) {
    if (dimensionNumBytes != 0) {
      throw invalid_argument(
          L"when dimensionCount is 0, dimensionNumBytes must 0; got " +
          to_wstring(dimensionNumBytes));
    }
  } else if (dimensionNumBytes == 0) {
    if (dimensionCount != 0) {
      throw invalid_argument(
          L"when dimensionNumBytes is 0, dimensionCount must 0; got " +
          to_wstring(dimensionCount));
    }
  }

  this->dimensionCount = dimensionCount;
  this->dimensionNumBytes = dimensionNumBytes;
}

int FieldType::pointDimensionCount() { return dimensionCount; }

int FieldType::pointNumBytes() { return dimensionNumBytes; }

wstring FieldType::toString()
{
  shared_ptr<StringBuilder> result = make_shared<StringBuilder>();
  if (stored()) {
    result->append(L"stored");
  }
  if (indexOptions_ != IndexOptions::NONE) {
    if (result->length() > 0) {
      result->append(L",");
    }
    result->append(L"indexed");
    if (tokenized()) {
      result->append(L",tokenized");
    }
    if (storeTermVectors()) {
      result->append(L",termVector");
    }
    if (storeTermVectorOffsets()) {
      result->append(L",termVectorOffsets");
    }
    if (storeTermVectorPositions()) {
      result->append(L",termVectorPosition");
    }
    if (storeTermVectorPayloads()) {
      result->append(L",termVectorPayloads");
    }
    if (omitNorms()) {
      result->append(L",omitNorms");
    }
    if (indexOptions_ != IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) {
      result->append(L",indexOptions=");
      result->append(indexOptions_);
    }
  }
  if (dimensionCount != 0) {
    if (result->length() > 0) {
      result->append(L",");
    }
    result->append(L"pointDimensionCount=");
    result->append(dimensionCount);
    result->append(L",pointNumBytes=");
    result->append(dimensionNumBytes);
  }
  if (docValuesType_ != DocValuesType::NONE) {
    if (result->length() > 0) {
      result->append(L",");
    }
    result->append(L"docValuesType=");
    result->append(docValuesType_);
  }

  return result->toString();
}

DocValuesType FieldType::docValuesType() { return docValuesType_; }

void FieldType::setDocValuesType(DocValuesType type)
{
  checkIfFrozen();
  if (type == nullptr) {
    throw make_shared<NullPointerException>(L"DocValuesType must not be null");
  }
  docValuesType_ = type;
}

int FieldType::hashCode()
{
  constexpr int prime = 31;
  int result = 1;
  result = prime * result + dimensionCount;
  result = prime * result + dimensionNumBytes;
  result = prime * result +
           ((docValuesType_ == nullptr) ? 0 : docValuesType_.hashCode());
  result = prime * result + indexOptions_.hashCode();
  result = prime * result + (omitNorms_ ? 1231 : 1237);
  result = prime * result + (storeTermVectorOffsets_ ? 1231 : 1237);
  result = prime * result + (storeTermVectorPayloads_ ? 1231 : 1237);
  result = prime * result + (storeTermVectorPositions_ ? 1231 : 1237);
  result = prime * result + (storeTermVectors_ ? 1231 : 1237);
  result = prime * result + (stored_ ? 1231 : 1237);
  result = prime * result + (tokenized_ ? 1231 : 1237);
  return result;
}

bool FieldType::equals(any obj)
{
  if (shared_from_this() == obj) {
    return true;
  }
  if (obj == nullptr) {
    return false;
  }
  if (getClass() != obj.type()) {
    return false;
  }
  shared_ptr<FieldType> other = any_cast<std::shared_ptr<FieldType>>(obj);
  if (dimensionCount != other->dimensionCount) {
    return false;
  }
  if (dimensionNumBytes != other->dimensionNumBytes) {
    return false;
  }
  if (docValuesType_ != other->docValuesType_) {
    return false;
  }
  if (indexOptions_ != other->indexOptions_) {
    return false;
  }
  if (omitNorms_ != other->omitNorms_) {
    return false;
  }
  if (storeTermVectorOffsets_ != other->storeTermVectorOffsets_) {
    return false;
  }
  if (storeTermVectorPayloads_ != other->storeTermVectorPayloads_) {
    return false;
  }
  if (storeTermVectorPositions_ != other->storeTermVectorPositions_) {
    return false;
  }
  if (storeTermVectors_ != other->storeTermVectors_) {
    return false;
  }
  if (stored_ != other->stored_) {
    return false;
  }
  if (tokenized_ != other->tokenized_) {
    return false;
  }
  return true;
}
} // namespace org::apache::lucene::document