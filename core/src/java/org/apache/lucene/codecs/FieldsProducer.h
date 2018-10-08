#pragma once
#include "../index/Fields.h"
#include "../util/Accountable.h"
#include "stringhelper.h"
#include <memory>

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

/** Abstract API that produces terms, doc, freq, prox, offset and
 *  payloads postings.
 *
 * @lucene.experimental
 */

class FieldsProducer : public Fields, public Accountable
{
  GET_CLASS_NAME(FieldsProducer)
  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  FieldsProducer();

public:
  void close() = 0;
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
  virtual std::shared_ptr<FieldsProducer> getMergeInstance() ;

protected:
  std::shared_ptr<FieldsProducer> shared_from_this()
  {
    return std::static_pointer_cast<FieldsProducer>(
        org.apache.lucene.index.Fields::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs
