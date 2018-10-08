#pragma once
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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

using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestFixBrokenOffsets : public LuceneTestCase
{
  GET_CLASS_NAME(TestFixBrokenOffsets)

  // Run this in Lucene 6.x:
  //
  //     ant test -Dtestcase=TestFixBrokenOffsets
  //     -Dtestmethod=testCreateBrokenOffsetsIndex -Dtests.codec=default
  //     -Dtests.useSecurityManager=false
  /*
  public void testCreateBrokenOffsetsIndex() throws IOException {

    Path indexDir = Paths.get("/tmp/brokenoffsets");
    Files.deleteIfExists(indexDir);
    Directory dir = newFSDirectory(indexDir);
    IndexWriter writer = new IndexWriter(dir, new IndexWriterConfig());

    Document doc = new Document();
    FieldType fieldType = new FieldType(TextField.TYPE_STORED);
    fieldType.setStoreTermVectors(true);
    fieldType.setStoreTermVectorPositions(true);
    fieldType.setStoreTermVectorOffsets(true);
    Field field = new Field("foo", "bar", fieldType);
    field.setTokenStream(new CannedTokenStream(new Token("foo", 10, 13), new
  Token("foo", 7, 9))); doc.add(field); writer.addDocument(doc);
    writer.commit();

    // 2nd segment
    doc = new Document();
    field = new Field("foo", "bar", fieldType);
    field.setTokenStream(new CannedTokenStream(new Token("bar", 15, 17), new
  Token("bar", 1, 5))); doc.add(field); writer.addDocument(doc);

    writer.close();

    dir.close();
  }
  */

public:
  virtual void testFixBrokenOffsetsIndex() ;

protected:
  std::shared_ptr<TestFixBrokenOffsets> shared_from_this()
  {
    return std::static_pointer_cast<TestFixBrokenOffsets>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
