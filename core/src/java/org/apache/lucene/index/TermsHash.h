#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class IntBlockPool;
}

namespace org::apache::lucene::util
{
class ByteBlockPool;
}
namespace org::apache::lucene::util
{
class Counter;
}
namespace org::apache::lucene::index
{
class DocumentsWriterPerThread;
}
namespace org::apache::lucene::index
{
class DocState;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::index
{
class Sorter;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::index
{
class TermsHashPerField;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class FieldInvertState;
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

using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using Counter = org::apache::lucene::util::Counter;
using IntBlockPool = org::apache::lucene::util::IntBlockPool;

/** This class is passed each token produced by the analyzer
 *  on each field during indexing, and it stores these
 *  tokens in a hash table, and allocates separate byte
 *  streams per token.  Consumers of this class, eg {@link
 *  FreqProxTermsWriter} and {@link TermVectorsConsumer},
GET_CLASS_NAME(,)
 *  write their own byte streams under each term. */
class TermsHash : public std::enable_shared_from_this<TermsHash>
{
  GET_CLASS_NAME(TermsHash)

public:
  const std::shared_ptr<TermsHash> nextTermsHash;

  const std::shared_ptr<IntBlockPool> intPool;
  const std::shared_ptr<ByteBlockPool> bytePool;
  std::shared_ptr<ByteBlockPool> termBytePool;
  const std::shared_ptr<Counter> bytesUsed;

  const std::shared_ptr<DocumentsWriterPerThread::DocState> docState;

  const bool trackAllocations;

  TermsHash(std::shared_ptr<DocumentsWriterPerThread> docWriter,
            bool trackAllocations, std::shared_ptr<TermsHash> nextTermsHash);

  virtual void abort();

  // Clear all state
  virtual void reset();

  virtual void
  flush(std::unordered_map<std::wstring, std::shared_ptr<TermsHashPerField>>
            &fieldsToFlush,
        std::shared_ptr<SegmentWriteState> state,
        std::shared_ptr<Sorter::DocMap> sortMap) ;

  virtual std::shared_ptr<TermsHashPerField>
  addField(std::shared_ptr<FieldInvertState> fieldInvertState,
           std::shared_ptr<FieldInfo> fieldInfo) = 0;

  virtual void finishDocument() ;

  virtual void startDocument() ;
};

} // namespace org::apache::lucene::index
