#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/join/BitSetProducer.h"
#include  "core/src/java/org/apache/lucene/document/Document.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"

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
namespace org::apache::lucene::search::join
{

using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestBlockJoinValidation : public LuceneTestCase
{
  GET_CLASS_NAME(TestBlockJoinValidation)

public:
  static constexpr int AMOUNT_OF_SEGMENTS = 5;
  static constexpr int AMOUNT_OF_PARENT_DOCS = 10;
  static constexpr int AMOUNT_OF_CHILD_DOCS = 5;
  static const int AMOUNT_OF_DOCS_IN_SEGMENT =
      AMOUNT_OF_PARENT_DOCS + AMOUNT_OF_PARENT_DOCS * AMOUNT_OF_CHILD_DOCS;

private:
  std::shared_ptr<Directory> directory;
  std::shared_ptr<IndexReader> indexReader;
  std::shared_ptr<IndexSearcher> indexSearcher;
  std::shared_ptr<BitSetProducer> parentsFilter;

public:
  void setUp()  override;

  void tearDown()  override;

  virtual void testNextDocValidationForToParentBjq() ;

  virtual void testNextDocValidationForToChildBjq() ;

  virtual void testAdvanceValidationForToChildBjq() ;

private:
  static std::deque<std::shared_ptr<Document>>
  createDocsForSegment(int segmentNumber);

  static std::deque<std::shared_ptr<Document>>
  createParentDocWithChildren(int segmentNumber, int parentNumber);

  static std::shared_ptr<Document> createParentDoc(int segmentNumber,
                                                   int parentNumber);

  static std::shared_ptr<Document>
  createChildDoc(int segmentNumber, int parentNumber, int childNumber);

  static std::wstring createFieldValue(std::deque<int> &documentNumbers);

  static std::shared_ptr<Query>
  createChildrenQueryWithOneParent(int childNumber);

  static std::shared_ptr<Query>
  createParentsQueryWithOneChild(int randomChildNumber);

  static int getRandomParentId();

  static int getRandomParentNumber();

  static std::shared_ptr<Query> randomChildQuery(int randomChildNumber);

  static int getRandomChildNumber(int notLessThan);

protected:
  std::shared_ptr<TestBlockJoinValidation> shared_from_this()
  {
    return std::static_pointer_cast<TestBlockJoinValidation>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
