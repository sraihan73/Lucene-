#pragma once
#include "../util/Accountable.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Fields.h"

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

using Fields = org::apache::lucene::index::Fields;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * Codec API for reading term vectors:
 *
 * @lucene.experimental
 */
class TermVectorsReader
    : public std::enable_shared_from_this<TermVectorsReader>,
      public Cloneable,
      public Accountable
{
  GET_CLASS_NAME(TermVectorsReader)

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  TermVectorsReader();

  /** Returns term vectors for this document, or null if
   *  term vectors were not indexed. If offsets are
   *  available they are in an {@link OffsetAttribute}
   *  available from the {@link org.apache.lucene.index.PostingsEnum}. */
public:
  virtual std::shared_ptr<Fields> get(int doc) = 0;

  /**
   * Checks consistency of this reader.
   * <p>
   * Note that this may be costly in terms of I/O, e.g.
   * may involve computing a checksum value against large data files.
   * @lucene.internal
   */
  virtual void checkIntegrity() = 0;

  /** Create a clone that one caller at a time may use to
   *  read term vectors. */
  std::shared_ptr<TermVectorsReader> clone() = 0;
  override

      /**
       * Returns an instance optimized for merging.
       * <p>
       * The default implementation returns {@code this} */
      virtual std::shared_ptr<TermVectorsReader>
      getMergeInstance() ;
};

} // #include  "core/src/java/org/apache/lucene/codecs/
