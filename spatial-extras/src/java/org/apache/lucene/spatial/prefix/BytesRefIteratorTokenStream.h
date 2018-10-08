#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRefIterator.h"

#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/BytesTermAttribute.h"

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
namespace org::apache::lucene::spatial::prefix
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

/**
 * A TokenStream used internally by {@link
 * org.apache.lucene.spatial.prefix.PrefixTreeStrategy}.
 *
 * @lucene.internal
 */
class BytesRefIteratorTokenStream : public TokenStream
{
  GET_CLASS_NAME(BytesRefIteratorTokenStream)

public:
  virtual std::shared_ptr<BytesRefIterator> getBytesRefIterator();

  virtual std::shared_ptr<BytesRefIteratorTokenStream>
  setBytesRefIterator(std::shared_ptr<BytesRefIterator> iter);

  void reset()  override;

  bool incrementToken()  override final;

  // members
private:
  const std::shared_ptr<BytesTermAttribute> bytesAtt =
      addAttribute(BytesTermAttribute::typeid);

  std::shared_ptr<BytesRefIterator> bytesIter =
      nullptr; // null means not initialized

protected:
  std::shared_ptr<BytesRefIteratorTokenStream> shared_from_this()
  {
    return std::static_pointer_cast<BytesRefIteratorTokenStream>(
        org.apache.lucene.analysis.TokenStream::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/spatial/prefix/
