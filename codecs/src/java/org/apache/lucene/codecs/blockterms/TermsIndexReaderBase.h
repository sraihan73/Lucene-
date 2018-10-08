#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/blockterms/FieldIndexEnum.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::codecs::blockterms
{

using FieldInfo = org::apache::lucene::index::FieldInfo;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;

// TODO
//   - allow for non-regular index intervals?  eg with a
//     long string of rare terms, you don't need such
//     frequent indexing

/**
 * {@link BlockTermsReader} interacts with an instance of this class
 * to manage its terms index.  The writer must accept
 * indexed terms (many pairs of BytesRef text + long
 * fileOffset), and then this reader must be able to
 * retrieve the nearest index term to a provided term
 * text.
 * @lucene.experimental */

class TermsIndexReaderBase
    : public std::enable_shared_from_this<TermsIndexReaderBase>,
      public Accountable
{
  GET_CLASS_NAME(TermsIndexReaderBase)

public:
  virtual std::shared_ptr<FieldIndexEnum>
  getFieldEnum(std::shared_ptr<FieldInfo> fieldInfo) = 0;

  void close() = 0;
  override

      virtual bool
      supportsOrd() = 0;

  /**
   * Similar to TermsEnum, except, the only "metadata" it
   * reports for a given indexed term is the long fileOffset
   * into the main terms dictionary file.
   */
public:
  class FieldIndexEnum : public std::enable_shared_from_this<FieldIndexEnum>
  {
    GET_CLASS_NAME(FieldIndexEnum)

    /** Seeks to "largest" indexed term that's &lt;=
     *  term; returns file pointer index (into the main
     *  terms index file) for that term */
  public:
    virtual int64_t seek(std::shared_ptr<BytesRef> term) = 0;

    /** Returns -1 at end */
    virtual int64_t next() = 0;

    virtual std::shared_ptr<BytesRef> term() = 0;

    /** Only implemented if {@link TermsIndexReaderBase#supportsOrd()} returns
     * true. */
    virtual int64_t seek(int64_t ord) = 0;

    /** Only implemented if {@link TermsIndexReaderBase#supportsOrd()} returns
     * true. */
    virtual int64_t ord() = 0;
  };
};

} // #include  "core/src/java/org/apache/lucene/codecs/blockterms/
