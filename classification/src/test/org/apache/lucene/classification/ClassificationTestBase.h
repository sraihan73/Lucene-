#pragma once
#include "../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/FieldType.h"
#include "../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../core/src/java/org/apache/lucene/index/LeafReader.h"
#include "../../../../../../../core/src/java/org/apache/lucene/search/Query.h"
#include "../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/LuceneTestCase.h"
#include "../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../java/org/apache/lucene/classification/ClassificationResult.h"
#include "../../../../../java/org/apache/lucene/classification/Classifier.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::classification
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using FieldType = org::apache::lucene::document::FieldType;
using LeafReader = org::apache::lucene::index::LeafReader;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Query = org::apache::lucene::search::Query;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Base class for testing {@link Classifier}s
 */
template <typename T>
class ClassificationTestBase : public LuceneTestCase
{
  GET_CLASS_NAME(ClassificationTestBase)
protected:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::wstring POLITICS_INPUT =
      std::wstring(L"Here are some interesting questions and answers about "
                   L"Mitt Romney.. ") +
      L"If you don't know the answer to the question about Mitt Romney, then "
      L"simply click on the answer below the question section.";
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<BytesRef> POLITICS_RESULT =
      std::make_shared<BytesRef>(L"politics");

  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::wstring TECHNOLOGY_INPUT =
      std::wstring(L"Much is made of what the likes of Facebook, Google and "
                   L"Apple know about users.") +
      L" Truth is, Amazon may know more.";

  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::wstring STRONG_TECHNOLOGY_INPUT =
      std::wstring(L"Much is made of what the likes of Facebook, Google and "
                   L"Apple know about users.") +
      L" Truth is, Amazon may know more. This technology observation is "
      L"extracted from the internet.";

  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::wstring SUPER_STRONG_TECHNOLOGY_INPUT =
      std::wstring(L"More than 400 million people trust Google with their "
                   L"e-mail, and 50 million store files") +
      L" in the cloud using the Dropbox service. People manage their bank "
      L"accounts, pay bills, trade stocks and " +
      L"generally transfer or store huge volumes of personal data online. "
      L"traveling seeks raises some questions Republican presidential. ";

  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<BytesRef> TECHNOLOGY_RESULT =
      std::make_shared<BytesRef>(L"technology");

  std::shared_ptr<RandomIndexWriter> indexWriter;
  std::shared_ptr<Directory> dir;
  std::shared_ptr<FieldType> ft;

  std::wstring textFieldName;
  std::wstring categoryFieldName;
  std::wstring booleanFieldName;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @Before public void setUp() throws Exception
  void setUp()  override
  {
    LuceneTestCase::setUp();
    dir = newDirectory();
    indexWriter = std::make_shared<RandomIndexWriter>(random(), dir);
    textFieldName = L"text";
    categoryFieldName = L"cat";
    booleanFieldName = L"bool";
    ft = std::make_shared<FieldType>(TextField::TYPE_STORED);
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorOffsets(true);
    ft->setStoreTermVectorPositions(true);
  }

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @After public void tearDown() throws Exception
  void tearDown()  override
  {
    LuceneTestCase::tearDown();
    delete indexWriter;
    delete dir;
  }

protected:
  virtual std::shared_ptr<ClassificationResult<T>>
  checkCorrectClassification(std::shared_ptr<Classifier<T>> classifier,
                             const std::wstring &inputDoc,
                             T expectedResult) 
  {
    GET_CLASS_NAME(ifier, )
    std::shared_ptr<ClassificationResult<T>> classificationResult =
        classifier->assignClass(inputDoc);
    T assignedClass = classificationResult->getAssignedClass();
    assertNotNull(assignedClass);
    assertEquals(L"got an assigned class of " + assignedClass,
                 std::static_pointer_cast<BytesRef>(expectedResult) != nullptr
                     ? (std::static_pointer_cast<BytesRef>(expectedResult))
                           ->utf8ToString()
                     : expectedResult,
                 std::static_pointer_cast<BytesRef>(assignedClass) != nullptr
                     ? (std::static_pointer_cast<BytesRef>(assignedClass))
                           ->utf8ToString()
                     : assignedClass);
    double score = classificationResult->getScore();
    assertTrue(L"score should be between 0 and 1, got:" +
                   std::to_wstring(score),
               score <= 1 && score >= 0);
    return classificationResult;
  }

  virtual void checkOnlineClassification(
      std::shared_ptr<Classifier<T>> classifier, const std::wstring &inputDoc,
      T expectedResult, std::shared_ptr<Analyzer> analyzer,
      const std::wstring &textFieldName,
      const std::wstring &classFieldName) 
  {
    GET_CLASS_NAME(ifier, )
    checkOnlineClassification(classifier, inputDoc, expectedResult, analyzer,
                              textFieldName, classFieldName, nullptr);
  }

  virtual void checkOnlineClassification(
      std::shared_ptr<Classifier<T>> classifier, const std::wstring &inputDoc,
      T expectedResult, std::shared_ptr<Analyzer> analyzer,
      const std::wstring &textFieldName, const std::wstring &classFieldName,
      std::shared_ptr<Query> query) 
  {
    GET_CLASS_NAME(ifier, )
    getSampleIndex(analyzer);

    std::shared_ptr<ClassificationResult<T>> classificationResult =
        classifier->assignClass(inputDoc);
    assertNotNull(classificationResult->getAssignedClass());
    assertEquals(L"got an assigned class of " +
                     classificationResult->getAssignedClass(),
                 expectedResult, classificationResult->getAssignedClass());
    double score = classificationResult->getScore();
    assertTrue(L"score should be between 0 and 1, got: " +
                   std::to_wstring(score),
               score <= 1 && score >= 0);
    updateSampleIndex();
    std::shared_ptr<ClassificationResult<T>> secondClassificationResult =
        classifier->assignClass(inputDoc);
    TestUtil::assertEquals(classificationResult->getAssignedClass(),
                           secondClassificationResult->getAssignedClass());
    TestUtil::assertEquals(
        static_cast<Double>(score),
        static_cast<Double>(secondClassificationResult->getScore()));
  }

  virtual std::shared_ptr<LeafReader>
  getSampleIndex(std::shared_ptr<Analyzer> analyzer) 
  {
    delete indexWriter;
    indexWriter = std::make_shared<RandomIndexWriter>(
        random(), dir,
        newIndexWriterConfig(analyzer)->setOpenMode(
            IndexWriterConfig::OpenMode::CREATE));
    indexWriter->commit();

    std::wstring text;

    std::shared_ptr<Document> doc = std::make_shared<Document>();
    text = std::wstring(L"The traveling press secretary for Mitt Romney lost "
                        L"his cool and cursed at reporters ") +
           L"who attempted to ask questions of the Republican presidential "
           L"candidate in a public plaza near the Tomb of " +
           L"the Unknown Soldier in Warsaw Tuesday.";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"politics", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"true", ft));

    indexWriter->addDocument(doc);

    doc = std::make_shared<Document>();
    text = std::wstring(L"Mitt Romney seeks to assure Israel and Iran, as well "
                        L"as Jewish voters in the United") +
           L" States, that he will be tougher against Iran's nuclear ambitions "
           L"than President Barack Obama.";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"politics", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"true", ft));
    indexWriter->addDocument(doc);

    doc = std::make_shared<Document>();
    text = std::wstring(L"And there's a threshold question that he has to "
                        L"answer for the American people and ") +
           L"that's whether he is prepared to be commander-in-chief,\" she "
           L"continued. \"As we look to the past events, we " +
           L"know that this raises some questions about his preparedness and "
           L"we'll see how the rest of his trip goes.\"";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"politics", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"true", ft));
    indexWriter->addDocument(doc);

    doc = std::make_shared<Document>();
    text = std::wstring(L"Still, when it comes to gun policy, many "
                        L"congressional Democrats have \"decided to ") +
           L"keep quiet and not go there,\" said Alan Lizotte, dean and "
           L"professor at the State University of New York at " +
           L"Albany's School of Criminal Justice.";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"politics", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"true", ft));
    indexWriter->addDocument(doc);

    doc = std::make_shared<Document>();
    text = std::wstring(L"Standing amongst the thousands of people at the "
                        L"state Capitol, Jorstad, director of ") +
           L"technology at the University of Wisconsin-La Crosse, documented "
           L"the historic moment and shared it with the " +
           L"world through the Internet.";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(
        std::make_shared<Field>(categoryFieldName, L"technology", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter->addDocument(doc);

    doc = std::make_shared<Document>();
    text = std::wstring(L"So, about all those experts and analysts who've "
                        L"spent the past year or so saying ") +
           L"Facebook was going to make a phone. A new expert has stepped "
           L"forward to say it's not going to happen.";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(
        std::make_shared<Field>(categoryFieldName, L"technology", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter->addDocument(doc);

    doc = std::make_shared<Document>();
    text = std::wstring(L"More than 400 million people trust Google with their "
                        L"e-mail, and 50 million store files") +
           L" in the cloud using the Dropbox service. People manage their bank "
           L"accounts, pay bills, trade stocks and " +
           L"generally transfer or store huge volumes of personal data online.";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(
        std::make_shared<Field>(categoryFieldName, L"technology", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter->addDocument(doc);

    doc = std::make_shared<Document>();
    text = L"unlabeled doc";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    indexWriter->addDocument(doc);

    indexWriter->commit();
    indexWriter->forceMerge(1);
    return getOnlyLeafReader(indexWriter->getReader());
  }

  virtual std::shared_ptr<LeafReader>
  getRandomIndex(std::shared_ptr<Analyzer> analyzer,
                 int size) 
  {
    delete indexWriter;
    indexWriter = std::make_shared<RandomIndexWriter>(
        random(), dir,
        newIndexWriterConfig(analyzer)->setOpenMode(
            IndexWriterConfig::OpenMode::CREATE));
    indexWriter->deleteAll();
    indexWriter->commit();

    std::shared_ptr<FieldType> ft =
        std::make_shared<FieldType>(TextField::TYPE_STORED);
    ft->setStoreTermVectors(true);
    ft->setStoreTermVectorOffsets(true);
    ft->setStoreTermVectorPositions(true);
    std::shared_ptr<Random> random = ClassificationTestBase::random();
    for (int i = 0; i < size; i++) {
      bool b = random->nextBoolean();
      std::shared_ptr<Document> doc = std::make_shared<Document>();
      doc->push_back(std::make_shared<Field>(textFieldName,
                                             createRandomString(random), ft));
      doc->push_back(std::make_shared<Field>(
          categoryFieldName, std::wstring::valueOf(random->nextInt(1000)), ft));
      doc->push_back(std::make_shared<Field>(booleanFieldName,
                                             StringHelper::toString(b), ft));
      indexWriter->addDocument(doc);
    }
    indexWriter->commit();
    indexWriter->forceMerge(1);
    return getOnlyLeafReader(indexWriter->getReader());
  }

private:
  std::wstring
