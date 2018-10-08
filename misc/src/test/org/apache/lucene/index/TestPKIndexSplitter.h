#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/Term.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"

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

using Document = org::apache::lucene::document::Document;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestPKIndexSplitter : public LuceneTestCase
{
  GET_CLASS_NAME(TestPKIndexSplitter)

public:
  virtual void testSplit() ;

private:
  void checkSplitting(std::shared_ptr<Directory> dir,
                      std::shared_ptr<Term> splitTerm, int leftCount,
                      int rightCount) ;

  void checkContents(std::shared_ptr<IndexReader> ir,
                     const std::wstring &indexname) ;

  std::shared_ptr<Document>
  createDocument(int n, const std::wstring &indexName, int numFields,
                 std::shared_ptr<NumberFormat> format);

protected:
  std::shared_ptr<TestPKIndexSplitter> shared_from_this()
  {
    return std::static_pointer_cast<TestPKIndexSplitter>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
