#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/DataInput.h"

#include  "core/src/java/org/apache/lucene/store/IndexOutput.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/ReplicaNode.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/FileMetaData.h"

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

using DataInput = org::apache::lucene::store::DataInput;
using IndexOutput = org::apache::lucene::store::IndexOutput;

/** Copies one file from an incoming DataInput to a dest filename in a local
 * Directory */

class CopyOneFile : public std::enable_shared_from_this<CopyOneFile>
{
  GET_CLASS_NAME(CopyOneFile)
private:
  const std::shared_ptr<DataInput> in_;
  const std::shared_ptr<IndexOutput> out;
  const std::shared_ptr<ReplicaNode> dest;

public:
  const std::wstring name;
  const std::wstring tmpName;
  const std::shared_ptr<FileMetaData> metaData;
  const int64_t bytesToCopy;

private:
  const int64_t copyStartNS;
  std::deque<char> const buffer;

  int64_t bytesCopied = 0;

public:
  CopyOneFile(std::shared_ptr<DataInput> in_, std::shared_ptr<ReplicaNode> dest,
              const std::wstring &name, std::shared_ptr<FileMetaData> metaData,
              std::deque<char> &buffer) ;

  /** Transfers this file copy to another input, continuing where the first one
   * left off */
  CopyOneFile(std::shared_ptr<CopyOneFile> other,
              std::shared_ptr<DataInput> in_);

  virtual ~CopyOneFile();

  /** Copy another chunk of bytes, returning true once the copy is done */
  virtual bool visit() ;

  virtual int64_t getBytesCopied();
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
