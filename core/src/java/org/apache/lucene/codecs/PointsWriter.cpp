using namespace std;

#include "PointsWriter.h"
#include "../index/FieldInfo.h"
#include "../index/MergeState.h"

namespace org::apache::lucene::codecs
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using MergeState = org::apache::lucene::index::MergeState;
using PointValues = org::apache::lucene::index::PointValues;

PointsWriter::PointsWriter() {}

void PointsWriter::mergeOneField(
    shared_ptr<MergeState> mergeState,
    shared_ptr<FieldInfo> fieldInfo) 
{
  int64_t maxPointCount = 0;
  int docCount = 0;
  for (int i = 0; i < mergeState->pointsReaders.size(); i++) {
    shared_ptr<PointsReader> pointsReader = mergeState->pointsReaders[i];
    if (pointsReader != nullptr) {
      shared_ptr<FieldInfo> readerFieldInfo =
          mergeState->fieldInfos[i]->fieldInfo(fieldInfo->name);
      if (readerFieldInfo != nullptr &&
          readerFieldInfo->getPointDimensionCount() > 0) {
        shared_ptr<PointValues> values =
            pointsReader->getValues(fieldInfo->name);
        if (values != nullptr) {
          maxPointCount += values->size();
          docCount += values->getDocCount();
        }
      }
    }
  }
  constexpr int64_t finalMaxPointCount = maxPointCount;
  constexpr int finalDocCount = docCount;
  writeField(fieldInfo, make_shared<PointsReaderAnonymousInnerClass>(
                            shared_from_this(), mergeState, fieldInfo,
                            finalMaxPointCount, finalDocCount));
}

PointsWriter::PointsReaderAnonymousInnerClass::PointsReaderAnonymousInnerClass(
    shared_ptr<PointsWriter> outerInstance, shared_ptr<MergeState> mergeState,
    shared_ptr<FieldInfo> fieldInfo, int64_t finalMaxPointCount,
    int finalDocCount)
{
  this->outerInstance = outerInstance;
  this->mergeState = mergeState;
  this->fieldInfo = fieldInfo;
  this->finalMaxPointCount = finalMaxPointCount;
  this->finalDocCount = finalDocCount;
}

int64_t PointsWriter::PointsReaderAnonymousInnerClass::ramBytesUsed()
{
  return 0;
}

PointsWriter::PointsReaderAnonymousInnerClass::
    ~PointsReaderAnonymousInnerClass()
{
}

shared_ptr<PointValues>
PointsWriter::PointsReaderAnonymousInnerClass::getValues(
    const wstring &fieldName)
{
  if (fieldName == fieldInfo->name == false) {
    throw invalid_argument(L"field name must match the field being merged");
  }

  return make_shared<PointValuesAnonymousInnerClass>(shared_from_this(),
                                                     fieldName);
}

PointsWriter::PointsReaderAnonymousInnerClass::PointValuesAnonymousInnerClass::
    PointValuesAnonymousInnerClass(
        shared_ptr<PointsReaderAnonymousInnerClass> outerInstance,
        const wstring &fieldName)
{
  this->outerInstance = outerInstance;
  this->fieldName = fieldName;
}

void PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::intersect(
        shared_ptr<PointValues::IntersectVisitor>
            mergedVisitor) 
{
  for (int i = 0; i < outerInstance->mergeState.pointsReaders->length; i++) {
    shared_ptr<PointsReader> pointsReader =
        outerInstance->mergeState.pointsReaders[i];
    if (pointsReader == nullptr) {
      // This segment has no points
      continue;
    }
    shared_ptr<FieldInfo> readerFieldInfo =
        outerInstance->mergeState.fieldInfos[i].fieldInfo(fieldName);
    if (readerFieldInfo == nullptr) {
      // This segment never saw this field
      continue;
    }

    if (readerFieldInfo->getPointDimensionCount() == 0) {
      // This segment saw this field, but the field did not index points in it:
      continue;
    }

    shared_ptr<PointValues> values = pointsReader->getValues(fieldName);
    if (values == nullptr) {
      continue;
    }
    shared_ptr<MergeState::DocMap> docMap =
        outerInstance->mergeState.docMaps[i];
    values->intersect(make_shared<IntersectVisitorAnonymousInnerClass>(
        shared_from_this(), mergedVisitor, docMap));
  }
}

PointsWriter::PointsReaderAnonymousInnerClass::PointValuesAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::IntersectVisitorAnonymousInnerClass(
        shared_ptr<PointValuesAnonymousInnerClass> outerInstance,
        shared_ptr<PointValues::IntersectVisitor> mergedVisitor,
        shared_ptr<MergeState::DocMap> docMap)
{
  this->outerInstance = outerInstance;
  this->mergedVisitor = mergedVisitor;
  this->docMap = docMap;
}

void PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::IntersectVisitorAnonymousInnerClass::visit(
        int docID)
{
  // Should never be called because our compare method never returns
  // Relation.CELL_INSIDE_QUERY
  throw make_shared<IllegalStateException>();
}

void PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::IntersectVisitorAnonymousInnerClass::visit(
        int docID, std::deque<char> &packedValue) 
{
  int newDocID = docMap->get(docID);
  if (newDocID != -1) {
    // Not deleted:
    mergedVisitor->visit(newDocID, packedValue);
  }
}

Relation
PointsWriter::PointsReaderAnonymousInnerClass::PointValuesAnonymousInnerClass::
    IntersectVisitorAnonymousInnerClass::compare(
        std::deque<char> &minPackedValue, std::deque<char> &maxPackedValue)
{
  // Forces this segment's PointsReader to always visit all docs + values:
  return Relation::CELL_CROSSES_QUERY;
}

int64_t
PointsWriter::PointsReaderAnonymousInnerClass::PointValuesAnonymousInnerClass::
    estimatePointCount(shared_ptr<PointValues::IntersectVisitor> visitor)
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char> PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::getMinPackedValue()
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char> PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::getMaxPackedValue()
{
  throw make_shared<UnsupportedOperationException>();
}

int PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::getNumDimensions()
{
  throw make_shared<UnsupportedOperationException>();
}

int PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::getBytesPerDimension()
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::size()
{
  return outerInstance->finalMaxPointCount;
}

int PointsWriter::PointsReaderAnonymousInnerClass::
    PointValuesAnonymousInnerClass::getDocCount()
{
  return outerInstance->finalDocCount;
}

void PointsWriter::PointsReaderAnonymousInnerClass::checkIntegrity() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

void PointsWriter::merge(shared_ptr<MergeState> mergeState) 
{
  // check each incoming reader
  for (auto reader : mergeState->pointsReaders) {
    if (reader != nullptr) {
      reader->checkIntegrity();
    }
  }
  // merge field at a time
  for (auto fieldInfo : mergeState->mergeFieldInfos) {
    if (fieldInfo->getPointDimensionCount() != 0) {
      mergeOneField(mergeState, fieldInfo);
    }
  }
  finish();
}
} // namespace org::apache::lucene::codecs