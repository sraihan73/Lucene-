using namespace std;

#include "FieldInfo.h"

namespace org::apache::lucene::index
{

FieldInfo::FieldInfo(const wstring &name, int number, bool storeTermVector,
                     bool omitNorms, bool storePayloads,
                     IndexOptions indexOptions, DocValuesType docValues,
                     int64_t dvGen,
                     unordered_map<wstring, wstring> &attributes,
                     int pointDimensionCount, int pointNumBytes,
                     bool softDeletesField)
    : name(Objects::requireNonNull(name)), number(number),
      attributes(Objects::requireNonNull(attributes)),
      softDeletesField(softDeletesField)
{
  this->docValuesType = Objects::requireNonNull(
      docValues, L"DocValuesType must not be null (field: \"" + name + L"\")");
  this->indexOptions = Objects::requireNonNull(
      indexOptions,
      L"IndexOptions must not be null (field: \"" + name + L"\")");
  if (indexOptions != IndexOptions::NONE) {
    this->storeTermVector = storeTermVector;
    this->storePayloads = storePayloads;
    this->omitNorms = omitNorms;
  } else { // for non-indexed fields, leave defaults
    this->storeTermVector = false;
    this->storePayloads = false;
    this->omitNorms = false;
  }
  this->dvGen = dvGen;
  this->pointDimensionCount = pointDimensionCount;
  this->pointNumBytes = pointNumBytes;
  assert(checkConsistency());
}

bool FieldInfo::checkConsistency()
{
  if (indexOptions != IndexOptions::NONE) {
    // Cannot store payloads unless positions are indexed:
    if (indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) <
            0 &&
        storePayloads) {
      throw make_shared<IllegalStateException>(
          L"indexed field '" + name +
          L"' cannot have payloads without positions");
    }
  } else {
    if (storeTermVector) {
      throw make_shared<IllegalStateException>(L"non-indexed field '" + name +
                                               L"' cannot store term vectors");
    }
    if (storePayloads) {
      throw make_shared<IllegalStateException>(L"non-indexed field '" + name +
                                               L"' cannot store payloads");
    }
    if (omitNorms) {
      throw make_shared<IllegalStateException>(L"non-indexed field '" + name +
                                               L"' cannot omit norms");
    }
  }

  if (pointDimensionCount < 0) {
    throw make_shared<IllegalStateException>(
        L"pointDimensionCount must be >= 0; got " +
        to_wstring(pointDimensionCount));
  }

  if (pointNumBytes < 0) {
    throw make_shared<IllegalStateException>(
        L"pointNumBytes must be >= 0; got " + to_wstring(pointNumBytes));
  }

  if (pointDimensionCount != 0 && pointNumBytes == 0) {
    throw make_shared<IllegalStateException>(
        L"pointNumBytes must be > 0 when pointDimensionCount=" +
        to_wstring(pointDimensionCount));
  }

  if (pointNumBytes != 0 && pointDimensionCount == 0) {
    throw make_shared<IllegalStateException>(
        L"pointDimensionCount must be > 0 when pointNumBytes=" +
        to_wstring(pointNumBytes));
  }

  if (dvGen != -1 && docValuesType == DocValuesType::NONE) {
    throw make_shared<IllegalStateException>(
        L"field '" + name +
        L"' cannot have a docvalues update generation without having "
        L"docvalues");
  }

  return true;
}

void FieldInfo::update(bool storeTermVector, bool omitNorms, bool storePayloads,
                       IndexOptions indexOptions, int dimensionCount,
                       int dimensionNumBytes)
{
  if (indexOptions == nullptr) {
    throw make_shared<NullPointerException>(
        L"IndexOptions must not be null (field: \"" + name + L"\")");
  }
  // System.out.println("FI.update field=" + name + " indexed=" + indexed + "
  // omitNorms=" + omitNorms + " this.omitNorms=" + this.omitNorms);
  if (this->indexOptions != indexOptions) {
    if (this->indexOptions == IndexOptions::NONE) {
      this->indexOptions = indexOptions;
    } else if (indexOptions != IndexOptions::NONE) {
      // downgrade
      this->indexOptions = this->indexOptions.compareTo(indexOptions) < 0
                               ? this->indexOptions
                               : indexOptions;
    }
  }

  if (this->pointDimensionCount == 0 && dimensionCount != 0) {
    this->pointDimensionCount = dimensionCount;
    this->pointNumBytes = dimensionNumBytes;
  } else if (dimensionCount != 0 &&
             (this->pointDimensionCount != dimensionCount ||
              this->pointNumBytes != dimensionNumBytes)) {
    throw invalid_argument(
        L"cannot change field \"" + name + L"\" from points dimensionCount=" +
        to_wstring(this->pointDimensionCount) + L", numBytes=" +
        to_wstring(this->pointNumBytes) + L" to inconsistent dimensionCount=" +
        to_wstring(dimensionCount) + L", numBytes=" +
        to_wstring(dimensionNumBytes));
  }

  if (this->indexOptions !=
      IndexOptions::NONE) { // if updated field data is not for indexing, leave
                            // the updates out
    this->storeTermVector |= storeTermVector; // once deque, always deque
    this->storePayloads |= storePayloads;

    // Awkward: only drop norms if incoming update is indexed:
    if (indexOptions != IndexOptions::NONE && this->omitNorms != omitNorms) {
      this->omitNorms = true; // if one require omitNorms at least once, it
                              // remains off for life
    }
  }
  if (this->indexOptions == IndexOptions::NONE ||
      this->indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) <
          0) {
    // cannot store payloads if we don't store positions:
    this->storePayloads = false;
  }
  assert(checkConsistency());
}

void FieldInfo::setPointDimensions(int count, int numBytes)
{
  if (count <= 0) {
    throw invalid_argument(L"point dimension count must be >= 0; got " +
                           to_wstring(count) + L" for field=\"" + name + L"\"");
  }
  if (count > PointValues::MAX_DIMENSIONS) {
    throw invalid_argument(
        L"point dimension count must be < PointValues.MAX_DIMENSIONS (= " +
        to_wstring(PointValues::MAX_DIMENSIONS) + L"); got " +
        to_wstring(count) + L" for field=\"" + name + L"\"");
  }
  if (numBytes <= 0) {
    throw invalid_argument(L"point numBytes must be >= 0; got " +
                           to_wstring(numBytes) + L" for field=\"" + name +
                           L"\"");
  }
  if (numBytes > PointValues::MAX_NUM_BYTES) {
    throw invalid_argument(
        L"point numBytes must be <= PointValues.MAX_NUM_BYTES (= " +
        to_wstring(PointValues::MAX_NUM_BYTES) + L"); got " +
        to_wstring(numBytes) + L" for field=\"" + name + L"\"");
  }
  if (pointDimensionCount != 0 && pointDimensionCount != count) {
    throw invalid_argument(L"cannot change point dimension count from " +
                           to_wstring(pointDimensionCount) + L" to " +
                           to_wstring(count) + L" for field=\"" + name + L"\"");
  }
  if (pointNumBytes != 0 && pointNumBytes != numBytes) {
    throw invalid_argument(
        L"cannot change point numBytes from " + to_wstring(pointNumBytes) +
        L" to " + to_wstring(numBytes) + L" for field=\"" + name + L"\"");
  }

  pointDimensionCount = count;
  pointNumBytes = numBytes;

  assert(checkConsistency());
}

int FieldInfo::getPointDimensionCount() { return pointDimensionCount; }

int FieldInfo::getPointNumBytes() { return pointNumBytes; }

void FieldInfo::setDocValuesType(DocValuesType type)
{
  if (type == nullptr) {
    throw make_shared<NullPointerException>(
        L"DocValuesType must not be null (field: \"" + name + L"\")");
  }
  if (docValuesType != DocValuesType::NONE && type != DocValuesType::NONE &&
      docValuesType != type) {
    throw invalid_argument(L"cannot change DocValues type from " +
                           docValuesType + L" to " + type + L" for field \"" +
                           name + L"\"");
  }
  docValuesType = type;
  assert(checkConsistency());
}

IndexOptions FieldInfo::getIndexOptions() { return indexOptions; }

void FieldInfo::setIndexOptions(IndexOptions newIndexOptions)
{
  if (indexOptions != newIndexOptions) {
    if (indexOptions == IndexOptions::NONE) {
      indexOptions = newIndexOptions;
    } else if (newIndexOptions != IndexOptions::NONE) {
      // downgrade
      indexOptions = indexOptions.compareTo(newIndexOptions) < 0
                         ? indexOptions
                         : newIndexOptions;
    }
  }

  if (indexOptions == IndexOptions::NONE ||
      indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) < 0) {
    // cannot store payloads if we don't store positions:
    storePayloads = false;
  }
}

DocValuesType FieldInfo::getDocValuesType() { return docValuesType; }

void FieldInfo::setDocValuesGen(int64_t dvGen)
{
  this->dvGen = dvGen;
  assert(checkConsistency());
}

int64_t FieldInfo::getDocValuesGen() { return dvGen; }

void FieldInfo::setStoreTermVectors()
{
  storeTermVector = true;
  assert(checkConsistency());
}

void FieldInfo::setStorePayloads()
{
  if (indexOptions != IndexOptions::NONE &&
      indexOptions.compareTo(IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0) {
    storePayloads = true;
  }
  assert(checkConsistency());
}

bool FieldInfo::omitsNorms() { return omitNorms; }

void FieldInfo::setOmitsNorms()
{
  if (indexOptions == IndexOptions::NONE) {
    throw make_shared<IllegalStateException>(
        L"cannot omit norms: this field is not indexed");
  }
  omitNorms = true;
}

bool FieldInfo::hasNorms()
{
  return indexOptions != IndexOptions::NONE && omitNorms == false;
}

bool FieldInfo::hasPayloads() { return storePayloads; }

bool FieldInfo::hasVectors() { return storeTermVector; }

wstring FieldInfo::getAttribute(const wstring &key) { return attributes_[key]; }

wstring FieldInfo::putAttribute(const wstring &key, const wstring &value)
{
  return attributes_.emplace(key, value);
}

unordered_map<wstring, wstring> FieldInfo::attributes() { return attributes_; }

bool FieldInfo::isSoftDeletesField() { return softDeletesField; }
} // namespace org::apache::lucene::index