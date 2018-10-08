#pragma once
#include "../store/IndexInput.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

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
namespace org::apache::lucene::codecs
{

using IndexInput = org::apache::lucene::store::IndexInput;

/**
 * This abstract class reads skip lists with multiple levels.
 *
 * See {@link MultiLevelSkipListWriter} for the information about the encoding
 * of the multi level skip lists.
 *
 * Subclasses must implement the abstract method {@link #readSkipData(int,
 * IndexInput)} which defines the actual format of the skip data.
 * @lucene.experimental
 */

class MultiLevelSkipListReader
    : public std::enable_shared_from_this<MultiLevelSkipListReader>
{
  GET_CLASS_NAME(MultiLevelSkipListReader)
  /** the maximum number of skip levels possible for this index */
protected:
  int maxNumberOfSkipLevels = 0;

  // number of levels in this skip deque
private:
  int numberOfSkipLevels = 0;

  // Expert: defines the number of top skip levels to buffer in memory.
  // Reducing this number results in less memory usage, but possibly
  // slower performance due to more random I/Os.
  // Please notice that the space each level occupies is limited by
  // the skipInterval. The top level can not contain more than
  // skipLevel entries, the second top level can not contain more
  // than skipLevel^2 entries and so forth.
  int numberOfLevelsToBuffer = 1;

  int docCount = 0;

  /** skipStream for each level. */
  std::deque<std::shared_ptr<IndexInput>> skipStream;

  /** The start pointer of each skip level. */
  std::deque<int64_t> skipPointer;

  /**  skipInterval of each level. */
  std::deque<int> skipInterval;

  /** Number of docs skipped per level.
   * It's possible for some values to overflow a signed int, but this has been
   * accounted for.
   */
  std::deque<int> numSkipped;

  /** Doc id of current skip entry per level. */
protected:
  std::deque<int> skipDoc;

  /** Doc id of last read skip entry with docId &lt;= target. */
private:
  int lastDoc = 0;

  /** Child pointer of current skip entry per level. */
  std::deque<int64_t> childPointer;

  /** childPointer of last read skip entry with docId &lt;=
   *  target. */
  int64_t lastChildPointer = 0;

  bool inputIsBuffered = false;
  const int skipMultiplier;

  /** Creates a {@code MultiLevelSkipListReader}. */
protected:
  MultiLevelSkipListReader(std::shared_ptr<IndexInput> skipStream,
                           int maxSkipLevels, int skipInterval,
                           int skipMultiplier);

  /** Creates a {@code MultiLevelSkipListReader}, where
   *  {@code skipInterval} and {@code skipMultiplier} are
   *  the same. */
  MultiLevelSkipListReader(std::shared_ptr<IndexInput> skipStream,
                           int maxSkipLevels, int skipInterval);

  /** Returns the id of the doc to which the last call of {@link #skipTo(int)}
   *  has skipped.  */
public:
  virtual int getDoc();

  /** Skips entries to the first beyond the current whose document number is
   *  greater than or equal to <i>target</i>. Returns the current doc count.
   */
  virtual int skipTo(int target) ;

private:
  bool loadNextSkip(int level) ;

  /** Seeks the skip entry on the given level */
protected:
  virtual void seekChild(int level) ;

public:
  virtual ~MultiLevelSkipListReader();

  /** Initializes the reader, for reuse on a new term. */
  virtual void init(int64_t skipPointer, int df) ;

  /** Loads the skip levels  */
private:
  void loadSkipLevels() ;

  /**
   * Subclasses must implement the actual skip data encoding in this method.
   *
   * @param level the level skip data shall be read from
   * @param skipStream the skip stream to read from
   */
protected:
  virtual int readSkipData(int level,
                           std::shared_ptr<IndexInput> skipStream) = 0;

  /** Copies the values of the last read skip entry on this level */
  virtual void setLastSkipData(int level);

  /** used to buffer the top skip levels */
private:
  class SkipBuffer final : public IndexInput
  {
    GET_CLASS_NAME(SkipBuffer)
  private:
    std::deque<char> data;
    int64_t pointer = 0;
    int pos = 0;

  public:
    SkipBuffer(std::shared_ptr<IndexInput> input,
               int length) ;

    virtual ~SkipBuffer();

    int64_t getFilePointer() override;

    int64_t length() override;

    char readByte() override;

    void readBytes(std::deque<char> &b, int offset, int len) override;

    void seek(int64_t pos) override;

    std::shared_ptr<IndexInput>
    slice(const std::wstring &sliceDescription, int64_t offset,
          int64_t length)  override;

  protected:
    std::shared_ptr<SkipBuffer> shared_from_this()
    {
      return std::static_pointer_cast<SkipBuffer>(
          org.apache.lucene.store.IndexInput::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/codecs/
