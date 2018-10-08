#pragma once
#include "../util/Accountable.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
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
namespace org::apache::lucene::codecs
{

using StoredFieldVisitor = org::apache::lucene::index::StoredFieldVisitor;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * Codec API for reading stored fields.
 * <p>
 * You need to implement {@link #visitDocument(int, StoredFieldVisitor)} to
 * read the stored fields for a document, implement {@link #clone()} (creating
 * clones of any IndexInputs used, etc), and {@link #close()}
 * @lucene.experimental
 */
class StoredFieldsReader
    : public std::enable_shared_from_this<StoredFieldsReader>,
      public Cloneable,
      public Accountable
{
  GET_CLASS_NAME(StoredFieldsReader)
  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  StoredFieldsReader();

  /** Visit the stored fields for document <code>docID</code> */
public:
  virtual void visitDocument(int docID,
                             std::shared_ptr<StoredFieldVisitor> visitor) = 0;

  std::shared_ptr<StoredFieldsReader> clone() = 0;
  override

      /**
       * Checks consistency of this reader.
       * <p>
       * Note that this may be costly in terms of I/O, e.g.
       * may involve computing a checksum value against large data files.
       * @lucene.internal
       */
      virtual void
      checkIntegrity() = 0;

  /**
   * Returns an instance optimized for merging.
   * <p>
   * The default implementation returns {@code this} */
  virtual std::shared_ptr<StoredFieldsReader>
  getMergeInstance() ;
};

} // namespace org::apache::lucene::codecs
