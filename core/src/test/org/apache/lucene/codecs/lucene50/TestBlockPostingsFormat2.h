#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class RandomIndexWriter;
}
namespace org::apache::lucene::document
{
class Document;
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
namespace org::apache::lucene::codecs::lucene50
{

using Document = org::apache::lucene::document::Document;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Tests special cases of BlockPostingsFormat
 */

class TestBlockPostingsFormat2 : public LuceneTestCase
{
  GET_CLASS_NAME(TestBlockPostingsFormat2)
public:
  std::shared_ptr<Directory> dir;
  std::shared_ptr<RandomIndexWriter> iw;

  void setUp()  override;

  void tearDown()  override;

private:
  std::shared_ptr<Document> newDocument();

  /** tests terms with df = blocksize */
public:
  virtual void testDFBlockSize() ;

  /** tests terms with df % blocksize = 0 */
  virtual void testDFBlockSizeMultiple() ;

  /** tests terms with ttf = blocksize */
  virtual void testTTFBlockSize() ;

  /** tests terms with ttf % blocksize = 0 */
  virtual void testTTFBlockSizeMultiple() ;

protected:
  std::shared_ptr<TestBlockPostingsFormat2> shared_from_this()
  {
    return std::static_pointer_cast<TestBlockPostingsFormat2>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene50
