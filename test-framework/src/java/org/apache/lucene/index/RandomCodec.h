#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <cmath>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsFormat;
}

namespace org::apache::lucene::codecs
{
class DocValuesFormat;
}
namespace org::apache::lucene::codecs
{
class PointsFormat;
}
namespace org::apache::lucene::codecs
{
class PointsWriter;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::util::bkd
{
class BKDWriter;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
}
namespace org::apache::lucene::store
{
class Directory;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::index
{

using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using AssertingCodec = org::apache::lucene::codecs::asserting::AssertingCodec;
using Directory = org::apache::lucene::store::Directory;
using BKDWriter = org::apache::lucene::util::bkd::BKDWriter;

/**
 * Codec that assigns per-field random postings formats.
 * <p>
 * The same field/format assignment will happen regardless of order,
 * a hash is computed up front that determines the mapping.
 * This means fields can be put into things like HashSets and added to
 * documents in different orders and the test will still be deterministic
 * and reproducable.
 */
class RandomCodec : public AssertingCodec
{
  GET_CLASS_NAME(RandomCodec)
  /** Shuffled deque of postings formats to use for new mappings */
private:
  std::deque<std::shared_ptr<PostingsFormat>> formats =
      std::deque<std::shared_ptr<PostingsFormat>>();

  /** Shuffled deque of docvalues formats to use for new mappings */
  std::deque<std::shared_ptr<DocValuesFormat>> dvFormats =
      std::deque<std::shared_ptr<DocValuesFormat>>();

  /** unique set of format names this codec knows about */
public:
  std::shared_ptr<Set<std::wstring>> formatNames =
      std::unordered_set<std::wstring>();

  /** unique set of docvalues format names this codec knows about */
  std::shared_ptr<Set<std::wstring>> dvFormatNames =
      std::unordered_set<std::wstring>();

  const std::shared_ptr<Set<std::wstring>> avoidCodecs;

  /** memorized field to postingsformat mappings */
  // note: we have to sync this map_obj even though it's just for
  // debugging/toString, otherwise DWPT's .toString() calls that iterate over
  // the map_obj can cause concurrentmodificationexception if indexwriter's
  // infostream is on
private:
  std::unordered_map<std::wstring, std::shared_ptr<PostingsFormat>>
      previousMappings = Collections::synchronizedMap(
          std::unordered_map<std::wstring, std::shared_ptr<PostingsFormat>>());
  std::unordered_map<std::wstring, std::shared_ptr<DocValuesFormat>>
      previousDVMappings = Collections::synchronizedMap(
          std::unordered_map<std::wstring, std::shared_ptr<DocValuesFormat>>());
  const int perFieldSeed;

  // a little messy: randomize the default codec's parameters here.
  // with the default values, we have e,g, 1024 points in leaf nodes,
  // which is less effective for testing.
  // TODO: improve how we randomize this...
  const int maxPointsInLeafNode;
  const double maxMBSortInHeap;
  const int bkdSplitRandomSeed;

public:
  std::shared_ptr<PointsFormat> pointsFormat() override;

private:
  class PointsFormatAnonymousInnerClass : public PointsFormat
  {
    GET_CLASS_NAME(PointsFormatAnonymousInnerClass)
  private:
    std::shared_ptr<RandomCodec> outerInstance;

  public:
    PointsFormatAnonymousInnerClass(std::shared_ptr<RandomCodec> outerInstance);

    std::shared_ptr<PointsWriter>
    fieldsWriter(std::shared_ptr<SegmentWriteState> writeState) throw(
        IOException) override;

  private:
    class Lucene60PointsWriterAnonymousInnerClass : public Lucene60PointsWriter
    {
      GET_CLASS_NAME(Lucene60PointsWriterAnonymousInnerClass)
    private:
      std::shared_ptr<PointsFormatAnonymousInnerClass> outerInstance;

      std::shared_ptr<org::apache::lucene::index::SegmentWriteState> writeState;

    public:
      Lucene60PointsWriterAnonymousInnerClass(
          std::shared_ptr<PointsFormatAnonymousInnerClass> outerInstance,
          std::shared_ptr<org::apache::lucene::index::SegmentWriteState>
              writeState,
          int maxPointsInLeafNode, double maxMBSortInHeap);

      void writeField(
          std::shared_ptr<FieldInfo> fieldInfo,
          std::shared_ptr<PointsReader> reader)  override;

    private:
      class IntersectVisitorAnonymousInnerClass
          : public std::enable_shared_from_this<
                IntersectVisitorAnonymousInnerClass>,
            public IntersectVisitor
      {
        GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
      private:
        std::shared_ptr<Lucene60PointsWriterAnonymousInnerClass> outerInstance;

        std::shared_ptr<BKDWriter> writer;

      public:
        IntersectVisitorAnonymousInnerClass(
            std::shared_ptr<Lucene60PointsWriterAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<BKDWriter> writer);

        void visit(int docID) override;

        void visit(int docID,
                   std::deque<char> &packedValue)  override;

        PointValues::Relation compare(std::deque<char> &minPackedValue,
                                      std::deque<char> &maxPackedValue);
      };

    protected:
      std::shared_ptr<Lucene60PointsWriterAnonymousInnerClass>
      shared_from_this()
      {
        return std::static_pointer_cast<
            Lucene60PointsWriterAnonymousInnerClass>(
            org.apache.lucene.codecs.lucene60
                .Lucene60PointsWriter::shared_from_this());
      }
    };

  public:
    std::shared_ptr<PointsReader>
    fieldsReader(std::shared_ptr<SegmentReadState> readState) throw(
        IOException) override;

  protected:
    std::shared_ptr<PointsFormatAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointsFormatAnonymousInnerClass>(
          org.apache.lucene.codecs.PointsFormat::shared_from_this());
    }
  };

public:
  std::shared_ptr<PostingsFormat>
  getPostingsFormatForField(const std::wstring &name) override;

  std::shared_ptr<DocValuesFormat>
  getDocValuesFormatForField(const std::wstring &name) override;

  RandomCodec(std::shared_ptr<Random> random,
              std::shared_ptr<Set<std::wstring>> avoidCodecs);

  RandomCodec(std::shared_ptr<Random> random);

private:
  void add(std::shared_ptr<Set<std::wstring>> avoidCodecs,
           std::deque<PostingsFormat> &postings);

  void addDocValues(std::shared_ptr<Set<std::wstring>> avoidCodecs,
                    std::deque<DocValuesFormat> &docvalues);

public:
  virtual std::wstring toString();

  /** Just like {@link BKDWriter} except it evilly picks random ways to split
   * cells on recursion to try to provoke geo APIs that get upset at fun
   * rectangles. */
private:
  class RandomlySplittingBKDWriter : public BKDWriter
  {
    GET_CLASS_NAME(RandomlySplittingBKDWriter)

  public:
    const std::shared_ptr<Random> random;

    RandomlySplittingBKDWriter(int maxDoc, std::shared_ptr<Directory> tempDir,
                               const std::wstring &tempFileNamePrefix,
                               int numDims, int bytesPerDim,
                               int maxPointsInLeafNode, double maxMBSortInHeap,
                               int64_t totalPointCount,
                               bool singleValuePerDoc,
                               int randomSeed) ;

  private:
    static bool getRandomSingleValuePerDoc(bool singleValuePerDoc,
                                           int randomSeed);

    static bool getRandomLongOrds(int64_t totalPointCount,
                                  bool singleValuePerDoc, int randomSeed);

    static int64_t getRandomOfflineSorterBufferMB(int randomSeed);

    static int getRandomOfflineSorterMaxTempFiles(int randomSeed);

  protected:
    int split(std::deque<char> &minPackedValue,
              std::deque<char> &maxPackedValue,
              std::deque<int> &parentDims) override;

  protected:
    std::shared_ptr<RandomlySplittingBKDWriter> shared_from_this()
    {
      return std::static_pointer_cast<RandomlySplittingBKDWriter>(
          org.apache.lucene.util.bkd.BKDWriter::shared_from_this());
    }
  };

protected:
  std::shared_ptr<RandomCodec> shared_from_this()
  {
    return std::static_pointer_cast<RandomCodec>(
        org.apache.lucene.codecs.asserting.AssertingCodec::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
