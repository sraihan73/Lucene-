using namespace std;

#include "PointValues.h"

namespace org::apache::lucene::index
{
using BinaryPoint = org::apache::lucene::document::BinaryPoint;
using DoublePoint = org::apache::lucene::document::DoublePoint;
using Field = org::apache::lucene::document::Field;
using FloatPoint = org::apache::lucene::document::FloatPoint;
using IntPoint = org::apache::lucene::document::IntPoint;
using LongPoint = org::apache::lucene::document::LongPoint;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using StringHelper = org::apache::lucene::util::StringHelper;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;

int64_t PointValues::size(shared_ptr<IndexReader> reader,
                            const wstring &field) 
{
  int64_t size = 0;
  for (auto ctx : reader->leaves()) {
    shared_ptr<PointValues> values = ctx->reader()->getPointValues(field);
    if (values != nullptr) {
      size += values->size();
    }
  }
  return size;
}

int PointValues::getDocCount(shared_ptr<IndexReader> reader,
                             const wstring &field) 
{
  int count = 0;
  for (auto ctx : reader->leaves()) {
    shared_ptr<PointValues> values = ctx->reader()->getPointValues(field);
    if (values != nullptr) {
      count += values->getDocCount();
    }
  }
  return count;
}

std::deque<char>
PointValues::getMinPackedValue(shared_ptr<IndexReader> reader,
                               const wstring &field) 
{
  std::deque<char> minValue;
  for (auto ctx : reader->leaves()) {
    shared_ptr<PointValues> values = ctx->reader()->getPointValues(field);
    if (values == nullptr) {
      continue;
    }
    std::deque<char> leafMinValue = values->getMinPackedValue();
    if (leafMinValue.empty()) {
      continue;
    }
    if (minValue.empty()) {
      minValue = leafMinValue.clone();
    } else {
      constexpr int numDimensions = values->getNumDimensions();
      constexpr int numBytesPerDimension = values->getBytesPerDimension();
      for (int i = 0; i < numDimensions; ++i) {
        int offset = i * numBytesPerDimension;
        if (StringHelper::compare(numBytesPerDimension, leafMinValue, offset,
                                  minValue, offset) < 0) {
          System::arraycopy(leafMinValue, offset, minValue, offset,
                            numBytesPerDimension);
        }
      }
    }
  }
  return minValue;
}

std::deque<char>
PointValues::getMaxPackedValue(shared_ptr<IndexReader> reader,
                               const wstring &field) 
{
  std::deque<char> maxValue;
  for (auto ctx : reader->leaves()) {
    shared_ptr<PointValues> values = ctx->reader()->getPointValues(field);
    if (values == nullptr) {
      continue;
    }
    std::deque<char> leafMaxValue = values->getMaxPackedValue();
    if (leafMaxValue.empty()) {
      continue;
    }
    if (maxValue.empty()) {
      maxValue = leafMaxValue.clone();
    } else {
      constexpr int numDimensions = values->getNumDimensions();
      constexpr int numBytesPerDimension = values->getBytesPerDimension();
      for (int i = 0; i < numDimensions; ++i) {
        int offset = i * numBytesPerDimension;
        if (StringHelper::compare(numBytesPerDimension, leafMaxValue, offset,
                                  maxValue, offset) > 0) {
          System::arraycopy(leafMaxValue, offset, maxValue, offset,
                            numBytesPerDimension);
        }
      }
    }
  }
  return maxValue;
}

PointValues::PointValues() {}
} // namespace org::apache::lucene::index