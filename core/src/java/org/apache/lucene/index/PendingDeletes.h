#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}

namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::index
{
class SegmentReader;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
namespace org::apache::lucene::index
{
class DocValuesFieldUpdates;
}
namespace org::apache::lucene::index
{
class Iterator;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class MergePolicy;
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

using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOSupplier = org::apache::lucene::util::IOSupplier;

/**
 * This class handles accounting and applying pending deletes for live segment
 * readers
 */
class PendingDeletes : public std::enable_shared_from_this<PendingDeletes>
{
  GET_CLASS_NAME(PendingDeletes)
protected:
  const std::shared_ptr<SegmentCommitInfo> info;
  // Read-only live docs, null until live docs are initialized or if all docs
  // are alive
private:
  std::shared_ptr<Bits> liveDocs;
  // Writeable live docs, null if this instance is not ready to accept writes,
  // in which case getMutableBits needs to be called
  std::shared_ptr<FixedBitSet> writeableLiveDocs;

protected:
  int pendingDeleteCount = 0;

private:
  bool liveDocsInitialized = false;

public:
  PendingDeletes(std::shared_ptr<SegmentReader> reader,
                 std::shared_ptr<SegmentCommitInfo> info);

  PendingDeletes(std::shared_ptr<SegmentCommitInfo> info);

private:
  PendingDeletes(std::shared_ptr<SegmentCommitInfo> info,
                 std::shared_ptr<Bits> liveDocs, bool liveDocsInitialized);

protected:
  virtual std::shared_ptr<FixedBitSet> getMutableBits();

  /**
   * Marks a document as deleted in this segment and return true if a document
   * got actually deleted or if the document was already deleted.
   */
public:
  virtual bool delete_(int docID) ;

  /**
   * Returns a snapshot of the current live docs.
   */
  virtual std::shared_ptr<Bits> getLiveDocs();

  /**
   * Returns a snapshot of the hard live docs.
   */
  virtual std::shared_ptr<Bits> getHardLiveDocs();

  /**
   * Returns the number of pending deletes that are not written to disk.
   */
protected:
  virtual int numPendingDeletes();

  /**
   * Called once a new reader is opened for this segment ie. when deletes or
   * updates are applied.
   */
public:
  virtual void
  onNewReader(std::shared_ptr<CodecReader> reader,
              std::shared_ptr<SegmentCommitInfo> info) ;

private:
  bool assertCheckLiveDocs(std::shared_ptr<Bits> bits, int expectedLength,
                           int expectedDeleteCount);

  /**
   * Resets the pending docs
   */
public:
  virtual void dropChanges();

  virtual std::wstring toString();

  /**
   * Writes the live docs to disk and returns <code>true</code> if any new docs
   * were written.
   */
  virtual bool writeLiveDocs(std::shared_ptr<Directory> dir) ;

  /**
   * Returns <code>true</code> iff the segment represented by this {@link
   * PendingDeletes} is fully deleted
   */
  virtual bool isFullyDeleted(IOSupplier<std::shared_ptr<CodecReader>>
                                  readerIOSupplier) ;

  /**
   * Called for every field update for the given field at flush time
   * @param info the field info of the field that's updated
   * @param iterator the values to apply
   */
  virtual void
  onDocValuesUpdate(std::shared_ptr<FieldInfo> info,
                    std::shared_ptr<DocValuesFieldUpdates::Iterator>
                        iterator) ;

  virtual int numDeletesToMerge(std::shared_ptr<MergePolicy> policy,
                                IOSupplier<std::shared_ptr<CodecReader>>
                                    readerIOSupplier) ;

  /**
   * Returns true if the given reader needs to be refreshed in order to see the
   * latest deletes
   */
  bool needsRefresh(std::shared_ptr<CodecReader> reader);

  /**
   * Returns the number of deleted docs in the segment.
   */
  int getDelCount();

  /**
   * Returns the number of live documents in this segment
   */
  int numDocs();

  // Call only from assert!
  virtual bool verifyDocCounts(std::shared_ptr<CodecReader> reader);
};

} // namespace org::apache::lucene::index
