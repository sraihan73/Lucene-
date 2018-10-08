#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/spell/LuceneDictionary.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefIterator.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::search::spell
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using IndexReader = org::apache::lucene::index::IndexReader;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Test case for LuceneDictionary.
 * It first creates a simple index and then a couple of instances of
 * LuceneDictionary on different fields and checks if all the right text comes
 * back.
 */
class TestLuceneDictionary : public LuceneTestCase
{
  GET_CLASS_NAME(TestLuceneDictionary)

private:
  std::shared_ptr<Directory> store;
  std::shared_ptr<Analyzer> analyzer;
  std::shared_ptr<IndexReader> indexReader = nullptr;
  std::shared_ptr<LuceneDictionary> ld;
  std::shared_ptr<BytesRefIterator> it;
  std::shared_ptr<BytesRef> spare = std::make_shared<BytesRef>();

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testFieldNonExistent() ;

  virtual void testFieldAaa() ;

  virtual void testFieldContents_1() ;

  virtual void testFieldContents_2() ;

  virtual void testFieldZzz() ;

  virtual void testSpellchecker() ;

protected:
  std::shared_ptr<TestLuceneDictionary> shared_from_this()
  {
    return std::static_pointer_cast<TestLuceneDictionary>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/spell/
