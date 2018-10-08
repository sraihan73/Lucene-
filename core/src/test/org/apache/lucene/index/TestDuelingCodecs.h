#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::codecs
{
class Codec;
}
namespace org::apache::lucene::index
{
class RandomIndexWriter;
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

using Codec = org::apache::lucene::codecs::Codec;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Compares one codec against another
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Slow public class TestDuelingCodecs extends
// org.apache.lucene.util.LuceneTestCase
class TestDuelingCodecs : public LuceneTestCase
{
public:
  std::shared_ptr<Directory> leftDir;
  std::shared_ptr<IndexReader> leftReader;
  std::shared_ptr<Codec> leftCodec;

  std::shared_ptr<Directory> rightDir;
  std::shared_ptr<IndexReader> rightReader;
  std::shared_ptr<Codec> rightCodec;
  std::shared_ptr<RandomIndexWriter> leftWriter;
  std::shared_ptr<RandomIndexWriter> rightWriter;
  int64_t seed = 0;
  std::wstring info; // for debugging

  void setUp()  override;

  void tearDown()  override;

  /**
   * populates a writer with random stuff. this must be fully reproducable with
   * the seed!
   */
  static void createRandomIndex(int numdocs,
                                std::shared_ptr<RandomIndexWriter> writer,
                                int64_t seed) ;

  /**
   * checks the two indexes are equivalent
   */
  // we use a small amount of docs here, so it works with any codec
  virtual void testEquals() ;

  virtual void testCrazyReaderEquals() ;

protected:
  std::shared_ptr<TestDuelingCodecs> shared_from_this()
  {
    return std::static_pointer_cast<TestDuelingCodecs>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
