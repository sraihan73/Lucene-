#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentInfo;
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

/** Embeds a [read-only] SegmentInfo and adds per-commit
 *  fields.
 *
 *  @lucene.experimental */
class SegmentCommitInfo : public std::enable_shared_from_this<SegmentCommitInfo>
{
  GET_CLASS_NAME(SegmentCommitInfo)

  /** The {@link SegmentInfo} that we wrap. */
public:
  const std::shared_ptr<SegmentInfo> info;

  // How many deleted docs in the segment:
private:
  int delCount = 0;

  // How many soft-deleted docs in the segment that are not also hard-deleted:
  int softDelCount = 0;

  // Generation number of the live docs file (-1 if there
  // are no deletes yet):
  int64_t delGen = 0;

  // Normally 1+delGen, unless an exception was hit on last
  // attempt to write:
  int64_t nextWriteDelGen = 0;

  // Generation number of the FieldInfos (-1 if there are no updates)
  int64_t fieldInfosGen = 0;

  // Normally 1+fieldInfosGen, unless an exception was hit on last attempt to
  // write
  int64_t nextWriteFieldInfosGen = 0;

  // Generation number of the DocValues (-1 if there are no updates)
  int64_t docValuesGen = 0;

  // Normally 1+dvGen, unless an exception was hit on last attempt to
  // write
  int64_t nextWriteDocValuesGen = 0;

  // Track the per-field DocValues update files
  const std::unordered_map<int, Set<std::wstring>> dvUpdatesFiles =
      std::unordered_map<int, Set<std::wstring>>();

  // TODO should we add .files() to FieldInfosFormat, like we have on
  // LiveDocsFormat?
  // track the fieldInfos update files
  const std::shared_ptr<Set<std::wstring>> fieldInfosFiles =
      std::unordered_set<std::wstring>();

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile long sizeInBytes = -1;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t sizeInBytes_ = -1;

  // NOTE: only used in-RAM by IW to track buffered deletes;
  // this is never written to/read from the Directory
  int64_t bufferedDeletesGen = -1;

  /**
   * Sole constructor.
   *
   * @param info
   *          {@link SegmentInfo} that we wrap
   * @param delCount
   *          number of deleted documents in this segment
   * @param delGen
   *          deletion generation number (used to name deletion files)
   * @param fieldInfosGen
   *          FieldInfos generation number (used to name field-infos files)
   * @param docValuesGen
   *          DocValues generation number (used to name doc-values updates
   * files)
   */
public:
  SegmentCommitInfo(std::shared_ptr<SegmentInfo> info, int delCount,
                    int softDelCount, int64_t delGen, int64_t fieldInfosGen,
                    int64_t docValuesGen);

  /** Returns the per-field DocValues updates files. */
  virtual std::unordered_map<int, Set<std::wstring>> getDocValuesUpdatesFiles();

  /** Sets the DocValues updates file names, per field number. Does not deep
   * clone the map_obj. */
  virtual void setDocValuesUpdatesFiles(
      std::unordered_map<int, Set<std::wstring>> &dvUpdatesFiles);

  /** Returns the FieldInfos file names. */
  virtual std::shared_ptr<Set<std::wstring>> getFieldInfosFiles();

  /** Sets the FieldInfos file names. */
  virtual void
  setFieldInfosFiles(std::shared_ptr<Set<std::wstring>> fieldInfosFiles);

  /** Called when we succeed in writing deletes */
  virtual void advanceDelGen();

  /** Called if there was an exception while writing
   *  deletes, so that we don't try to write to the same
   *  file more than once. */
  virtual void advanceNextWriteDelGen();

  /** Gets the nextWriteDelGen. */
  virtual int64_t getNextWriteDelGen();

  /** Sets the nextWriteDelGen. */
  virtual void setNextWriteDelGen(int64_t v);

  /** Called when we succeed in writing a new FieldInfos generation. */
  virtual void advanceFieldInfosGen();

  /**
   * Called if there was an exception while writing a new generation of
   * FieldInfos, so that we don't try to write to the same file more than once.
   */
  virtual void advanceNextWriteFieldInfosGen();

  /** Gets the nextWriteFieldInfosGen. */
  virtual int64_t getNextWriteFieldInfosGen();

  /** Sets the nextWriteFieldInfosGen. */
  virtual void setNextWriteFieldInfosGen(int64_t v);

  /** Called when we succeed in writing a new DocValues generation. */
  virtual void advanceDocValuesGen();

  /**
   * Called if there was an exception while writing a new generation of
   * DocValues, so that we don't try to write to the same file more than once.
   */
  virtual void advanceNextWriteDocValuesGen();

  /** Gets the nextWriteDocValuesGen. */
  virtual int64_t getNextWriteDocValuesGen();

  /** Sets the nextWriteDocValuesGen. */
  virtual void setNextWriteDocValuesGen(int64_t v);

  /** Returns total size in bytes of all files for this
   *  segment. */
  virtual int64_t sizeInBytes() ;

  /** Returns all files in use by this segment. */
  virtual std::shared_ptr<std::deque<std::wstring>> files() ;

  virtual int64_t getBufferedDeletesGen();

  virtual void setBufferedDeletesGen(int64_t v);

  /** Returns true if there are any deletions for the
   * segment at this commit. */
  virtual bool hasDeletions();

  /** Returns true if there are any field updates for the segment in this
   * commit. */
  virtual bool hasFieldUpdates();

  /** Returns the next available generation number of the FieldInfos files. */
  virtual int64_t getNextFieldInfosGen();

  /**
   * Returns the generation number of the field infos file or -1 if there are no
   * field updates yet.
   */
  virtual int64_t getFieldInfosGen();

  /** Returns the next available generation number of the DocValues files. */
  virtual int64_t getNextDocValuesGen();

  /**
   * Returns the generation number of the DocValues file or -1 if there are no
   * doc-values updates yet.
   */
  virtual int64_t getDocValuesGen();

  /**
   * Returns the next available generation number
   * of the live docs file.
   */
  virtual int64_t getNextDelGen();

  /**
   * Returns generation number of the live docs file
   * or -1 if there are no deletes yet.
   */
  virtual int64_t getDelGen();

  /**
   * Returns the number of deleted docs in the segment.
   */
  virtual int getDelCount();

  /**
   * Returns the number of only soft-deleted docs.
   */
  virtual int getSoftDelCount();

  virtual void setDelCount(int delCount);

  virtual void setSoftDelCount(int softDelCount);

  /** Returns a description of this segment. */
  virtual std::wstring toString(int pendingDelCount);

  virtual std::wstring toString();

  std::shared_ptr<SegmentCommitInfo> clone() override;

  int getDelCount(bool includeSoftDeletes);
};

} // namespace org::apache::lucene::index
