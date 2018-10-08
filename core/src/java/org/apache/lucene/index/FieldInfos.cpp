using namespace std;

#include "FieldInfos.h"

namespace org::apache::lucene::index
{
using ArrayUtil = org::apache::lucene::util::ArrayUtil;

FieldInfos::FieldInfos(std::deque<std::shared_ptr<FieldInfo>> &infos)
    : hasFreq(hasFreq_), hasProx(hasProx_), hasPayloads(hasPayloads_),
      hasOffsets(hasOffsets_), hasVectors(hasVectors_), hasNorms(hasNorms_),
      hasDocValues(hasDocValues_), hasPointValues(hasPointValues_),
      byNumber(std::deque<std::shared_ptr<FieldInfo>>(size)),
      values(Collections::unmodifiableCollection(Arrays::asList(
          valuesTemp::toArray(std::deque<std::shared_ptr<FieldInfo>>(0)))))
{
  bool hasVectors = false;
  bool hasProx = false;
  bool hasPayloads = false;
  bool hasOffsets = false;
  bool hasFreq = false;
  bool hasNorms = false;
  bool hasDocValues = false;
  bool hasPointValues = false;
  wstring softDeletesField = L"";

  int size =
      0; // number of elements in byNumberTemp, number of used array slots
  std::deque<std::shared_ptr<FieldInfo>> byNumberTemp(
      10); // initial array capacity of 10
  for (auto info : infos) {
    if (info->number < 0) {
      throw invalid_argument(L"illegal field number: " +
                             to_wstring(info->number) + L" for field " +
                             info->name);
    }
    size = info->number >= size ? info->number + 1 : size;
    if (info->number >= byNumberTemp.size()) { // grow array
      byNumberTemp = ArrayUtil::grow(byNumberTemp, info->number + 1);
    }
    shared_ptr<FieldInfo> previous = byNumberTemp[info->number];
    if (previous != nullptr) {
      throw invalid_argument(L"duplicate field numbers: " + previous->name +
                             L" and " + info->name + L" have: " +
                             to_wstring(info->number));
    }
    byNumberTemp[info->number] = info;

    previous = byName.emplace(info->name, info);
    if (previous != nullptr) {
      throw invalid_argument(
          L"duplicate field names: " + to_wstring(previous->number) + L" and " +
          to_wstring(info->number) + L" have: " + info->name);
    }

    hasVectors |= info->hasVectors();
    hasProx |= info->getIndexOptions().compareTo(
                   IndexOptions::DOCS_AND_FREQS_AND_POSITIONS) >= 0;
    hasFreq |= info->getIndexOptions() != IndexOptions::DOCS;
    hasOffsets |=
        info->getIndexOptions().compareTo(
            IndexOptions::DOCS_AND_FREQS_AND_POSITIONS_AND_OFFSETS) >= 0;
    hasNorms |= info->hasNorms();
    hasDocValues |= info->getDocValuesType() != DocValuesType::NONE;
    hasPayloads |= info->hasPayloads();
    hasPointValues |= (info->getPointDimensionCount() != 0);
    if (info->isSoftDeletesField()) {
      if (softDeletesField != L"" && softDeletesField == info->name == false) {
        throw invalid_argument(L"multiple soft-deletes fields [" + info->name +
                               L", " + softDeletesField + L"]");
      }
      softDeletesField = info->name;
    }
  }

  this->softDeletesField = softDeletesField;

  deque<std::shared_ptr<FieldInfo>> valuesTemp =
      deque<std::shared_ptr<FieldInfo>>();
  for (int i = 0; i < size; i++) {
    byNumber[i] = byNumberTemp[i];
    if (byNumberTemp[i] != nullptr) {
      valuesTemp.push_back(byNumberTemp[i]);
    }
  }
}

bool FieldInfos::hasFreq() { return hasFreq_; }

bool FieldInfos::hasProx() { return hasProx_; }

bool FieldInfos::hasPayloads() { return hasPayloads_; }

bool FieldInfos::hasOffsets() { return hasOffsets_; }

bool FieldInfos::hasVectors() { return hasVectors_; }

bool FieldInfos::hasNorms() { return hasNorms_; }

bool FieldInfos::hasDocValues() { return hasDocValues_; }

bool FieldInfos::hasPointValues() { return hasPointValues_; }

wstring FieldInfos::getSoftDeletesField() { return softDeletesField; }

int FieldInfos::size() { return byName.size(); }

shared_ptr<Iterator<std::shared_ptr<FieldInfo>>> FieldInfos::iterator()
{
  return values->begin();
}

shared_ptr<FieldInfo> FieldInfos::fieldInfo(const wstring &fieldName)
{
  return byName[fieldName];
}

shared_ptr<FieldInfo> FieldInfos::fieldInfo(int fieldNumber)
{
  if (fieldNumber < 0) {
    throw invalid_argument(L"Illegal field number: " + to_wstring(fieldNumber));
  }
  if (fieldNumber >= byNumber.size()) {
    return nullptr;
  }
  return byNumber[fieldNumber];
}

FieldInfos::FieldDimensions::FieldDimensions(int dimensionCount,
                                             int dimensionNumBytes)
    : dimensionCount(dimensionCount), dimensionNumBytes(dimensionNumBytes)
{
}

FieldInfos::FieldNumbers::FieldNumbers(const wstring &softDeletesFieldName)
    : numberToName(unordered_map<>()), nameToNumber(unordered_map<>()),
      docValuesType(unordered_map<>()), dimensions(unordered_map<>()),
      softDeletesFieldName(softDeletesFieldName)
{
}

// C++ WARNING: The following method was originally marked 'synchronized':
int FieldInfos::FieldNumbers::addOrGet(const wstring &fieldName,
                                       int preferredFieldNumber,
                                       DocValuesType dvType, int dimensionCount,
                                       int dimensionNumBytes,
                                       bool isSoftDeletesField)
{
  if (dvType != DocValuesType::NONE) {
    DocValuesType currentDVType = docValuesType[fieldName];
    if (currentDVType == nullptr) {
      docValuesType.emplace(fieldName, dvType);
    } else if (currentDVType != DocValuesType::NONE &&
               currentDVType != dvType) {
      throw invalid_argument(L"cannot change DocValues type from " +
                             currentDVType + L" to " + dvType +
                             L" for field \"" + fieldName + L"\"");
    }
  }
  if (dimensionCount != 0) {
    shared_ptr<FieldDimensions> dims = dimensions[fieldName];
    if (dims != nullptr) {
      if (dims->dimensionCount != dimensionCount) {
        throw invalid_argument(L"cannot change point dimension count from " +
                               to_wstring(dims->dimensionCount) + L" to " +
                               to_wstring(dimensionCount) + L" for field=\"" +
                               fieldName + L"\"");
      }
      if (dims->dimensionNumBytes != dimensionNumBytes) {
        throw invalid_argument(L"cannot change point numBytes from " +
                               to_wstring(dims->dimensionNumBytes) + L" to " +
                               to_wstring(dimensionNumBytes) +
                               L" for field=\"" + fieldName + L"\"");
      }
    } else {
      dimensions.emplace(fieldName, make_shared<FieldDimensions>(
                                        dimensionCount, dimensionNumBytes));
    }
  }
  optional<int> fieldNumber = nameToNumber[fieldName];
  if (!fieldNumber) {
    const optional<int> preferredBoxed =
        static_cast<Integer>(preferredFieldNumber);
    if (preferredFieldNumber != -1 &&
        numberToName.find(preferredBoxed) == numberToName.end()) {
      // cool - we can use this number globally
      fieldNumber = preferredBoxed;
    } else {
      // find a new FieldNumber
      while (numberToName.find(++lowestUnassignedFieldNumber) !=
             numberToName.end()) {
        // might not be up to date - lets do the work once needed
      }
      fieldNumber = lowestUnassignedFieldNumber;
    }
    assert(fieldNumber >= 0);
    numberToName.emplace(fieldNumber, fieldName);
    nameToNumber.emplace(fieldName, fieldNumber);
  }

  if (isSoftDeletesField) {
    if (softDeletesFieldName == L"") {
      throw invalid_argument(L"this index has [" + fieldName +
                             L"] as soft-deletes already but soft-deletes "
                             L"field is not configured in IWC");
    } else if (fieldName == softDeletesFieldName == false) {
      throw invalid_argument(L"cannot configure [" + softDeletesFieldName +
                             L"] as soft-deletes; this index uses [" +
                             fieldName + L"] as soft-deletes already");
    }
  } else if (fieldName == softDeletesFieldName) {
    throw invalid_argument(L"cannot configure [" + softDeletesFieldName +
                           L"] as soft-deletes; this index uses [" + fieldName +
                           L"] as non-soft-deletes already");
  }

  return fieldNumber.value();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FieldInfos::FieldNumbers::verifyConsistent(optional<int> &number,
                                                const wstring &name,
                                                DocValuesType dvType)
{
  if (name == numberToName[number] == false) {
    throw invalid_argument(L"field number " + number +
                           L" is already mapped to field name \"" +
                           numberToName[number] + L"\", not \"" + name + L"\"");
  }
  if (number == nameToNumber[name] == false) {
    throw invalid_argument(
        L"field name \"" + name + L"\" is already mapped to field number \"" +
        to_wstring(nameToNumber[name]) + L"\", not \"" + number + L"\"");
  }
  DocValuesType currentDVType = docValuesType[name];
  if (dvType != DocValuesType::NONE && currentDVType != nullptr &&
      currentDVType != DocValuesType::NONE && dvType != currentDVType) {
    throw invalid_argument(L"cannot change DocValues type from " +
                           currentDVType + L" to " + dvType + L" for field \"" +
                           name + L"\"");
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FieldInfos::FieldNumbers::verifyConsistentDimensions(optional<int> &number,
                                                          const wstring &name,
                                                          int dimensionCount,
                                                          int dimensionNumBytes)
{
  if (name == numberToName[number] == false) {
    throw invalid_argument(L"field number " + number +
                           L" is already mapped to field name \"" +
                           numberToName[number] + L"\", not \"" + name + L"\"");
  }
  if (number == nameToNumber[name] == false) {
    throw invalid_argument(
        L"field name \"" + name + L"\" is already mapped to field number \"" +
        to_wstring(nameToNumber[name]) + L"\", not \"" + number + L"\"");
  }
  shared_ptr<FieldDimensions> dim = dimensions[name];
  if (dim != nullptr) {
    if (dim->dimensionCount != dimensionCount) {
      throw invalid_argument(L"cannot change point dimension count from " +
                             to_wstring(dim->dimensionCount) + L" to " +
                             to_wstring(dimensionCount) + L" for field=\"" +
                             name + L"\"");
    }
    if (dim->dimensionNumBytes != dimensionNumBytes) {
      throw invalid_argument(L"cannot change point numBytes from " +
                             to_wstring(dim->dimensionNumBytes) + L" to " +
                             to_wstring(dimensionNumBytes) + L" for field=\"" +
                             name + L"\"");
    }
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
bool FieldInfos::FieldNumbers::contains(const wstring &fieldName,
                                        DocValuesType dvType)
{
  // used by IndexWriter.updateNumericDocValue
  if (nameToNumber.find(fieldName) == nameToNumber.end()) {
    return false;
  } else {
    // only return true if the field has the same dvType as the requested one
    return dvType == docValuesType[fieldName];
  }
}

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<Set<wstring>> FieldInfos::FieldNumbers::getFieldNames()
{
  return Collections::unmodifiableSet(unordered_set<>(nameToNumber.keySet()));
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FieldInfos::FieldNumbers::clear()
{
  numberToName.clear();
  nameToNumber.clear();
  docValuesType.clear();
  dimensions.clear();
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FieldInfos::FieldNumbers::setDocValuesType(int number, const wstring &name,
                                                DocValuesType dvType)
{
  verifyConsistent(number, name, dvType);
  docValuesType.emplace(name, dvType);
}

// C++ WARNING: The following method was originally marked 'synchronized':
void FieldInfos::FieldNumbers::setDimensions(int number, const wstring &name,
                                             int dimensionCount,
                                             int dimensionNumBytes)
{
  if (dimensionNumBytes > PointValues::MAX_NUM_BYTES) {
    throw invalid_argument(
        L"dimension numBytes must be <= PointValues.MAX_NUM_BYTES (= " +
        to_wstring(PointValues::MAX_NUM_BYTES) + L"); got " +
        to_wstring(dimensionNumBytes) + L" for field=\"" + name + L"\"");
  }
  if (dimensionCount > PointValues::MAX_DIMENSIONS) {
    throw invalid_argument(
        L"pointDimensionCount must be <= PointValues.MAX_DIMENSIONS (= " +
        to_wstring(PointValues::MAX_DIMENSIONS) + L"); got " +
        to_wstring(dimensionCount) + L" for field=\"" + name + L"\"");
  }
  verifyConsistentDimensions(number, name, dimensionCount, dimensionNumBytes);
  dimensions.emplace(
      name, make_shared<FieldDimensions>(dimensionCount, dimensionNumBytes));
}

FieldInfos::Builder::Builder(shared_ptr<FieldNumbers> globalFieldNumbers)
    : globalFieldNumbers(globalFieldNumbers)
{
  assert(globalFieldNumbers != nullptr);
}

void FieldInfos::Builder::add(shared_ptr<FieldInfos> other)
{
  assert(assertNotFinished());
  for (auto fieldInfo : other) {
    add(fieldInfo);
  }
}

shared_ptr<FieldInfo> FieldInfos::Builder::getOrAdd(const wstring &name)
{
  shared_ptr<FieldInfo> fi = fieldInfo(name);
  if (fi == nullptr) {
    assert(assertNotFinished());
    // This field wasn't yet added to this in-RAM
    // segment's FieldInfo, so now we get a global
    // number for this field.  If the field was seen
    // before then we'll get the same name and number,
    // else we'll allocate a new one:
    constexpr bool isSoftDeletesField =
        name == globalFieldNumbers->softDeletesFieldName;
    constexpr int fieldNumber = globalFieldNumbers->addOrGet(
        name, -1, DocValuesType::NONE, 0, 0, isSoftDeletesField);
    fi = make_shared<FieldInfo>(name, fieldNumber, false, false, false,
                                IndexOptions::NONE, DocValuesType::NONE, -1,
                                unordered_map<>(), 0, 0, isSoftDeletesField);
    assert(byName.find(fi->name) == byName.end());
    globalFieldNumbers->verifyConsistent(static_cast<Integer>(fi->number),
                                         fi->name, DocValuesType::NONE);
    byName.emplace(fi->name, fi);
  }

  return fi;
}

shared_ptr<FieldInfo> FieldInfos::Builder::addOrUpdateInternal(
    const wstring &name, int preferredFieldNumber, bool storeTermVector,
    bool omitNorms, bool storePayloads, IndexOptions indexOptions,
    DocValuesType docValues, int64_t dvGen, int dimensionCount,
    int dimensionNumBytes, bool isSoftDeletesField)
{
  assert(assertNotFinished());
  if (docValues == nullptr) {
    throw make_shared<NullPointerException>(L"DocValuesType must not be null");
  }
  shared_ptr<FieldInfo> fi = fieldInfo(name);
  if (fi == nullptr) {
    // This field wasn't yet added to this in-RAM
    // segment's FieldInfo, so now we get a global
    // number for this field.  If the field was seen
    // before then we'll get the same name and number,
    // else we'll allocate a new one:
    constexpr int fieldNumber = globalFieldNumbers->addOrGet(
        name, preferredFieldNumber, docValues, dimensionCount,
        dimensionNumBytes, isSoftDeletesField);
    fi = make_shared<FieldInfo>(name, fieldNumber, storeTermVector, omitNorms,
                                storePayloads, indexOptions, docValues, dvGen,
                                unordered_map<>(), dimensionCount,
                                dimensionNumBytes, isSoftDeletesField);
    assert(byName.find(fi->name) == byName.end());
    globalFieldNumbers->verifyConsistent(static_cast<Integer>(fi->number),
                                         fi->name, fi->getDocValuesType());
    byName.emplace(fi->name, fi);
  } else {
    fi->update(storeTermVector, omitNorms, storePayloads, indexOptions,
               dimensionCount, dimensionNumBytes);

    if (docValues != DocValuesType::NONE) {
      // Only pay the synchronization cost if fi does not already have a DVType
      bool updateGlobal = fi->getDocValuesType() == DocValuesType::NONE;
      if (updateGlobal) {
        // Must also update docValuesType map_obj so it's
        // aware of this field's DocValuesType.  This will throw
        // IllegalArgumentException if an illegal type change was attempted.
        globalFieldNumbers->setDocValuesType(fi->number, name, docValues);
      }

      fi->setDocValuesType(
          docValues); // this will also perform the consistency check.
      fi->setDocValuesGen(dvGen);
    }
  }
  return fi;
}

shared_ptr<FieldInfo> FieldInfos::Builder::add(shared_ptr<FieldInfo> fi)
{
  return add(fi, -1);
}

shared_ptr<FieldInfo> FieldInfos::Builder::add(shared_ptr<FieldInfo> fi,
                                               int64_t dvGen)
{
  // IMPORTANT - reuse the field number if possible for consistent field numbers
  // across segments
  return addOrUpdateInternal(fi->name, fi->number, fi->hasVectors(),
                             fi->omitsNorms(), fi->hasPayloads(),
                             fi->getIndexOptions(), fi->getDocValuesType(),
                             dvGen, fi->getPointDimensionCount(),
                             fi->getPointNumBytes(), fi->isSoftDeletesField());
}

shared_ptr<FieldInfo> FieldInfos::Builder::fieldInfo(const wstring &fieldName)
{
  return byName[fieldName];
}

bool FieldInfos::Builder::assertNotFinished()
{
  if (finished) {
    throw make_shared<IllegalStateException>(
        L"FieldInfos.Builder was already finished; cannot add new fields");
  }
  return true;
}

shared_ptr<FieldInfos> FieldInfos::Builder::finish()
{
  finished = true;
  return make_shared<FieldInfos>(byName.values().toArray(
      std::deque<std::shared_ptr<FieldInfo>>(byName.size())));
}
} // namespace org::apache::lucene::index