#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DocMap.h"

#include  "core/src/java/org/apache/lucene/index/SegmentInfo.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/util/InfoStream.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
#include  "core/src/java/org/apache/lucene/util/packed/PackedLongValues.h"
#include  "core/src/java/org/apache/lucene/search/Sort.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"

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

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Sort = org::apache::lucene::search::Sort;
using Bits = org::apache::lucene::util::Bits;
using InfoStream = org::apache::lucene::util::InfoStream;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

/** Holds common state used during segment merging.
 *
 * @lucene.experimental */
class MergeState : public std::enable_shared_from_this<MergeState>
{
  GET_CLASS_NAME(MergeState)

  /** Maps document IDs from old segments to document IDs in the new segment */
public:
  std::deque<std::shared_ptr<DocMap>> const docMaps;

  // Only used by IW when it must remap deletes that arrived against the merging
  // segments while a merge was running:
  std::deque<std::shared_ptr<DocMap>> const leafDocMaps;

  /** {@link SegmentInfo} of the newly merged segment. */
  const std::shared_ptr<SegmentInfo> segmentInfo;

  /** {@link FieldInfos} of the newly merged segment. */
  std::shared_ptr<FieldInfos> mergeFieldInfos;

  /** Stored field producers being merged */
  std::deque<std::shared_ptr<StoredFieldsReader>> const storedFieldsReaders;

  /** Term deque producers being merged */
  std::deque<std::shared_ptr<TermVectorsReader>> const termVectorsReaders;

  /** Norms producers being merged */
  std::deque<std::shared_ptr<NormsProducer>> const normsProducers;

  /** DocValues producers being merged */
  std::deque<std::shared_ptr<DocValuesProducer>> const docValuesProducers;

  /** FieldInfos being merged */
  std::deque<std::shared_ptr<FieldInfos>> const fieldInfos;

  /** Live docs for each reader */
  std::deque<std::shared_ptr<Bits>> const liveDocs;

  /** Postings to merge */
  std::deque<std::shared_ptr<FieldsProducer>> const fieldsProducers;

  /** Point readers to merge */
  std::deque<std::shared_ptr<PointsReader>> const pointsReaders;

  /** Max docs per reader */
  std::deque<int> const maxDocs;

  /** InfoStream for debugging messages. */
  const std::shared_ptr<InfoStream> infoStream;

  /** Indicates if the index needs to be sorted **/
  bool needsIndexSort = false;

  /** Sole constructor. */
  MergeState(std::deque<std::shared_ptr<CodecReader>> &originalReaders,
             std::shared_ptr<SegmentInfo> segmentInfo,
             std::shared_ptr<InfoStream> infoStream) ;

  // Remap docIDs around deletions
private:
  std::deque<std::shared_ptr<DocMap>>
  buildDeletionDocMaps(std::deque<std::shared_ptr<CodecReader>> &readers);

private:
  class DocMapAnonymousInnerClass : public DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass)
  private:
    std::shared_ptr<MergeState> outerInstance;

    std::shared_ptr<Bits> liveDocs;
    std::shared_ptr<PackedLongValues> delDocMap;
    int docBase = 0;

  public:
    DocMapAnonymousInnerClass(std::shared_ptr<MergeState> outerInstance,
                              std::shared_ptr<Bits> liveDocs,
                              std::shared_ptr<PackedLongValues> delDocMap,
                              int docBase);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass>(
          DocMap::shared_from_this());
    }
  };

private:
  std::deque<std::shared_ptr<DocMap>>
  buildDocMaps(std::deque<std::shared_ptr<CodecReader>> &readers,
               std::shared_ptr<Sort> indexSort) ;

  std::deque<std::shared_ptr<CodecReader>>
  maybeSortReaders(std::deque<std::shared_ptr<CodecReader>> &originalReaders,
                   std::shared_ptr<SegmentInfo> segmentInfo) ;

private:
  class DocMapAnonymousInnerClass2 : public DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass2)
  private:
    std::shared_ptr<MergeState> outerInstance;

  public:
    DocMapAnonymousInnerClass2(std::shared_ptr<MergeState> outerInstance);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass2>(
          DocMap::shared_from_this());
    }
  };

private:
  class DocMapAnonymousInnerClass3 : public DocMap
  {
    GET_CLASS_NAME(DocMapAnonymousInnerClass3)
  private:
    std::shared_ptr<MergeState> outerInstance;

    std::shared_ptr<org::apache::lucene::index::Sorter::DocMap> sortDocMap;

  public:
    DocMapAnonymousInnerClass3(
        std::shared_ptr<MergeState> outerInstance,
        std::shared_ptr<org::apache::lucene::index::Sorter::DocMap> sortDocMap);

    int get(int docID) override;

  protected:
    std::shared_ptr<DocMapAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<DocMapAnonymousInnerClass3>(
          DocMap::shared_from_this());
    }
  };

  /** A map_obj of doc IDs. */
public:
  class DocMap : public std::enable_shared_from_this<DocMap>
  {
    GET_CLASS_NAME(DocMap)
    /** Sole constructor */
  public:
    DocMap();

    /** Return the mapped docID or -1 if the given doc is not mapped. */
    virtual int get(int docID) = 0;
  };

public:
  static std::shared_ptr<PackedLongValues>
  removeDeletes(int const maxDoc, std::shared_ptr<Bits> liveDocs);
};

} // #include  "core/src/java/org/apache/lucene/index/
