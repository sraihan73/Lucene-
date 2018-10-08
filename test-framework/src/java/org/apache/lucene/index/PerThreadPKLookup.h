#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TermsEnum;
}

namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

/** Utility class to do efficient primary-key (only 1 doc contains the
 *  given term) lookups by segment, re-using the enums.  This class is
 *  not thread safe, so it is the caller's job to create and use one
 *  instance of this per thread.  Do not use this if a term may appear
 *  in more than one document!  It will only return the first one it
 *  finds. */
class PerThreadPKLookup : public std::enable_shared_from_this<PerThreadPKLookup>
{
  GET_CLASS_NAME(PerThreadPKLookup)

protected:
  std::deque<std::shared_ptr<TermsEnum>> const termsEnums;
  std::deque<std::shared_ptr<PostingsEnum>> const postingsEnums;
  std::deque<std::shared_ptr<Bits>> const liveDocs;
  std::deque<int> const docBases;
  const int numSegs;
  const bool hasDeletions;

public:
  PerThreadPKLookup(std::shared_ptr<IndexReader> r,
                    const std::wstring &idFieldName) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<LeafReaderContext>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<PerThreadPKLookup> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<PerThreadPKLookup> outerInstance);

    int compare(std::shared_ptr<LeafReaderContext> c1,
                std::shared_ptr<LeafReaderContext> c2);
  };

  /** Returns docID if found, else -1. */
public:
  virtual int lookup(std::shared_ptr<BytesRef> id) ;

  // TODO: add reopen method to carry over re-used enums...?
};

} // namespace org::apache::lucene::index
