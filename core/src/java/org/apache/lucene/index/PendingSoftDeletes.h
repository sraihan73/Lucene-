#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PendingDeletes.h"

#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReader.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/index/DocValuesFieldUpdates.h"
#include  "core/src/java/org/apache/lucene/index/Iterator.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"

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

using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOSupplier = org::apache::lucene::util::IOSupplier;

class PendingSoftDeletes final : public PendingDeletes
{
  GET_CLASS_NAME(PendingSoftDeletes)

private:
  const std::wstring field;
  int64_t dvGeneration = -2;
  const std::shared_ptr<PendingDeletes> hardDeletes;

public:
  PendingSoftDeletes(const std::wstring &field,
                     std::shared_ptr<SegmentCommitInfo> info);

  PendingSoftDeletes(const std::wstring &field,
                     std::shared_ptr<SegmentReader> reader,
                     std::shared_ptr<SegmentCommitInfo> info);

  bool delete_(int docID)  override;

protected:
  int numPendingDeletes() override;

public:
  void onNewReader(
      std::shared_ptr<CodecReader> reader,
      std::shared_ptr<SegmentCommitInfo> info)  override;

  bool
  writeLiveDocs(std::shared_ptr<Directory> dir)  override;

  void dropChanges() override;

  /**
   * Clears all bits in the given bitset that are set and are also in the given
   * DocIdSetIterator.
   *
   * @param iterator the doc ID set iterator for apply
   * @param bits the bit set to apply the deletes to
   * @return the number of bits changed by this function
   */
  static int
  applySoftDeletes(std::shared_ptr<DocIdSetIterator> iterator,
                   std::shared_ptr<FixedBitSet> bits) ;

  void onDocValuesUpdate(std::shared_ptr<FieldInfo> info,
                         std::shared_ptr<DocValuesFieldUpdates::Iterator>
                             iterator)  override;

private:
  bool assertPendingDeletes();

public:
  virtual std::wstring toString();

  int numDeletesToMerge(std::shared_ptr<MergePolicy> policy,
                        IOSupplier<std::shared_ptr<CodecReader>>
                            readerIOSupplier)  override;

private:
  void ensureInitialized(IOSupplier<std::shared_ptr<CodecReader>>
                             readerIOSupplier) ;

public:
  bool isFullyDeleted(IOSupplier<std::shared_ptr<CodecReader>>
                          readerIOSupplier)  override;

private:
  std::shared_ptr<FieldInfos> readFieldInfos() ;

public:
  std::shared_ptr<Bits> getHardLiveDocs() override;

  static int
  countSoftDeletes(std::shared_ptr<DocIdSetIterator> softDeletedDocs,
                   std::shared_ptr<Bits> hardDeletes) ;

protected:
  std::shared_ptr<PendingSoftDeletes> shared_from_this()
  {
    return std::static_pointer_cast<PendingSoftDeletes>(
        PendingDeletes::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
