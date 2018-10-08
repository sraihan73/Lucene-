#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class RAMOutputStream;
}

namespace org::apache::lucene::document
{
class Document;
}
namespace org::apache::lucene::replicator::nrt
{
class NodeProcess;
}
namespace org::apache::lucene::store
{
class DataInput;
}
namespace org::apache::lucene::replicator::nrt
{
class Connection;
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

namespace org::apache::lucene::replicator::nrt
{

using Document = org::apache::lucene::document::Document;
using DataInput = org::apache::lucene::store::DataInput;
using RAMOutputStream = org::apache::lucene::store::RAMOutputStream;

/** This is a stupid yet functional transaction log: it never fsync's, never
 * prunes, it's over-synchronized, it hard-wires id field name to "docid", can
 *  only handle specific docs/fields used by this test, etc.  It's just barely
 * enough to show how a translog could work on top of NRT replication to
 * guarantee no data loss when nodes crash */

class SimpleTransLog : public std::enable_shared_from_this<SimpleTransLog>
{
  GET_CLASS_NAME(SimpleTransLog)

public:
  const std::shared_ptr<FileChannel> channel;
  const std::shared_ptr<RAMOutputStream> buffer =
      std::make_shared<RAMOutputStream>();
  std::deque<char> const intBuffer = std::deque<char>(4);
  const std::shared_ptr<ByteBuffer> intByteBuffer = ByteBuffer::wrap(intBuffer);

private:
  static const char OP_ADD_DOCUMENT = static_cast<char>(0);
  static const char OP_UPDATE_DOCUMENT = static_cast<char>(1);
  static const char OP_DELETE_DOCUMENTS = static_cast<char>(2);

public:
  SimpleTransLog(std::shared_ptr<Path> path) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t getNextLocation() ;

  /** Appends an addDocument op */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t
  addDocument(const std::wstring &id,
              std::shared_ptr<Document> doc) ;

  /** Appends an updateDocument op */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t
  updateDocument(const std::wstring &id,
                 std::shared_ptr<Document> doc) ;

  /** Appends a deleteDocuments op */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t deleteDocuments(const std::wstring &id) ;

  /** Writes buffer to the file and returns the start position. */
private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t flushBuffer() ;

  void writeBytesToChannel(std::shared_ptr<ByteBuffer> src) ;

  void
  readBytesFromChannel(int64_t pos,
                       std::shared_ptr<ByteBuffer> dest) ;

  /** Replays ops between start and end location against the provided writer.
   * Can run concurrently with ongoing operations. */
public:
  virtual void replay(std::shared_ptr<NodeProcess> primary, int64_t start,
                      int64_t end) ;

private:
  void replayAddDocument(std::shared_ptr<Connection> c,
                         std::shared_ptr<NodeProcess> primary,
                         std::shared_ptr<DataInput> in_) ;

  void replayDeleteDocuments(std::shared_ptr<Connection> c,
                             std::shared_ptr<NodeProcess> primary,
                             std::shared_ptr<DataInput> in_) ;

  /** Encodes doc into buffer.  NOTE: this is NOT general purpose!  It only
   * handles the fields used in this test! */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void encode(const std::wstring &id,
              std::shared_ptr<Document> doc) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void writeNullableString(const std::wstring &s) ;

  std::wstring
  readNullableString(std::shared_ptr<DataInput> in_) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~SimpleTransLog();
};

} // namespace org::apache::lucene::replicator::nrt
