#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/RAMOutputStream.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"

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

using IndexOutput = org::apache::lucene::store::IndexOutput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;

/**
 * This abstract class writes skip lists with multiple levels.
 *
 * <pre>
 *
 * Example for skipInterval = 3:
 *                                                     c            (skip level
 * 2) c                 c                 c            (skip level 1) x     x x
 * x     x     x     x     x     x     x      (skip level 0) d d d d d d d d d d
 * d d d d d d d d d d d d d d d d d d d d d d  (posting deque) 3     6     9 12
 * 15    18    21    24    27    30     (df)
 *
 * d - document
 * x - skip data
 * c - skip data with child pointer
 *
 * Skip level i contains every skipInterval-th entry from skip level i-1.
 * Therefore the number of entries on level i is: floor(df / ((skipInterval ^ (i
 * + 1))).
 *
 * Each skip entry on a level {@code i>0} contains a pointer to the
 * corresponding skip entry in deque i-1. This guarantees a logarithmic amount of
 * skips to find the target document.
 *
 * While this class takes care of writing the different skip levels,
 * subclasses must define the actual format of the skip data.
 * </pre>
 * @lucene.experimental
 */

class MultiLevelSkipListWriter
    : public std::enable_shared_from_this<MultiLevelSkipListWriter>
{
  GET_CLASS_NAME(MultiLevelSkipListWriter)
  /** number of levels in this skip deque */
protected:
  int numberOfSkipLevels = 0;

  /** the skip interval in the deque with level = 0 */
private:
  int skipInterval = 0;

  /** skipInterval used for level &gt; 0 */
  int skipMultiplier = 0;

  /** for every skip level a different buffer is used  */
  std::deque<std::shared_ptr<RAMOutputStream>> skipBuffer;

  /** Creates a {@code MultiLevelSkipListWriter}. */
protected:
  MultiLevelSkipListWriter(int skipInterval, int skipMultiplier,
                           int maxSkipLevels, int df);

  /** Creates a {@code MultiLevelSkipListWriter}, where
   *  {@code skipInterval} and {@code skipMultiplier} are
   *  the same. */
  MultiLevelSkipListWriter(int skipInterval, int maxSkipLevels, int df);

  /** Allocates internal skip buffers. */
  virtual void init();

  /** Creates new buffers or empties the existing ones */
  virtual void resetSkip();

  /**
   * Subclasses must implement the actual skip data encoding in this method.
   *
   * @param level the level skip data shall be writing for
   * @param skipBuffer the skip buffer to write to
   */
  virtual void writeSkipData(int level,
                             std::shared_ptr<IndexOutput> skipBuffer) = 0;

  /**
   * Writes the current skip data to the buffers. The current document frequency
   * determines the max level is skip data is to be written to.
   *
   * @param df the current document frequency
   * @throws IOException If an I/O error occurs
   */
public:
  virtual void bufferSkip(int df) ;

  /**
   * Writes the buffered skip lists to the given output.
   *
   * @param output the IndexOutput the skip lists shall be written to
   * @return the pointer the skip deque starts
   */
  virtual int64_t
  writeSkip(std::shared_ptr<IndexOutput> output) ;
};

} // #include  "core/src/java/org/apache/lucene/codecs/
