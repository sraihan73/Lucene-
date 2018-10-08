#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThread.h"

#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsWriter.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/IndexableField.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"

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

using StoredFieldsWriter = org::apache::lucene::codecs::StoredFieldsWriter;

class StoredFieldsConsumer
    : public std::enable_shared_from_this<StoredFieldsConsumer>
{
  GET_CLASS_NAME(StoredFieldsConsumer)
public:
  const std::shared_ptr<DocumentsWriterPerThread> docWriter;
  std::shared_ptr<StoredFieldsWriter> writer;
  int lastDoc = 0;

  StoredFieldsConsumer(std::shared_ptr<DocumentsWriterPerThread> docWriter);

protected:
  virtual void initStoredFieldsWriter() ;

public:
  virtual void startDocument(int docID) ;

  virtual void
  writeField(std::shared_ptr<FieldInfo> info,
             std::shared_ptr<IndexableField> field) ;

  virtual void finishDocument() ;

  virtual void finish(int maxDoc) ;

  virtual void
  flush(std::shared_ptr<SegmentWriteState> state,
        std::shared_ptr<Sorter::DocMap> sortMap) ;

  virtual void abort();
};

} // #include  "core/src/java/org/apache/lucene/index/
