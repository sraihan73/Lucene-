#pragma once
#include "../../../../../../../../analysis/common/src/java/org/apache/lucene/analysis/en/EnglishAnalyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../../../test-framework/src/java/org/apache/lucene/index/RandomIndexWriter.h"
#include "../../../../../../java/org/apache/lucene/classification/ClassificationResult.h"
#include "../../../../../../java/org/apache/lucene/classification/document/DocumentClassifier.h"
#include "../ClassificationTestBase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
namespace org::apache::lucene::classification::document
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using ClassificationTestBase =
    org::apache::lucene::classification::ClassificationTestBase;
using Document = org::apache::lucene::document::Document;
using IndexReader = org::apache::lucene::index::IndexReader;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Base class for testing {@link org.apache.lucene.classification.Classifier}s
 */
template <typename T>
class DocumentClassificationTestBase : public ClassificationTestBase
{
  GET_CLASS_NAME(DocumentClassificationTestBase)

protected:
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<BytesRef> VIDEOGAME_RESULT =
      std::make_shared<BytesRef>(L"videogames");
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<BytesRef> VIDEOGAME_ANALYZED_RESULT =
      std::make_shared<BytesRef>(L"videogam");
  // C++ TODO: Native C++ does not allow initialization of static
  // non-const/integral fields in their declarations - choose the conversion
  // option for separate .h and .cpp files:
  static const std::shared_ptr<BytesRef> BATMAN_RESULT =
      std::make_shared<BytesRef>(L"batman");

  std::wstring titleFieldName = L"title";
  std::wstring authorFieldName = L"author";

  std::shared_ptr<Analyzer> analyzer;
  std::unordered_map<std::wstring, std::shared_ptr<Analyzer>> field2analyzer;
  std::shared_ptr<IndexReader> indexReader;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Before public void init() throws java.io.IOException
  virtual void init() 
  {
    analyzer = std::make_shared<EnglishAnalyzer>();
    field2analyzer = std::make_shared<LinkedHashMap<>>();
    field2analyzer.emplace(textFieldName, analyzer);
    field2analyzer.emplace(titleFieldName, analyzer);
    field2analyzer.emplace(authorFieldName, analyzer);
    indexReader = populateDocumentClassificationIndex(analyzer);
  }

protected:
  virtual double checkCorrectDocumentClassification(
      std::shared_ptr<DocumentClassifier<T>> classifier,
      std::shared_ptr<Document> inputDoc,
      T expectedResult) 
  {
    GET_CLASS_NAME(ifier, )
    std::shared_ptr<ClassificationResult<T>> classificationResult =
        classifier->assignClass(inputDoc);
    assertNotNull(classificationResult->getAssignedClass());
    assertEquals(L"got an assigned class of " +
                     classificationResult->getAssignedClass(),
                 expectedResult, classificationResult->getAssignedClass());
    double score = classificationResult->getScore();
    assertTrue(L"score should be between 0 and 1, got:" +
                   std::to_wstring(score),
               score <= 1 && score >= 0);
    return score;
  }

  virtual std::shared_ptr<IndexReader> populateDocumentClassificationIndex(
      std::shared_ptr<Analyzer> analyzer) 
  {
    indexWriter->close();
    indexWriter = std::make_shared<RandomIndexWriter>(
        random(), dir,
        newIndexWriterConfig(analyzer).setOpenMode(
            IndexWriterConfig::OpenMode::CREATE));
    indexWriter::commit();
    std::wstring text;
    std::wstring title;
    std::wstring author;

    std::shared_ptr<Document> doc = std::make_shared<Document>();
    title = L"Video games are an economic business";
    text = std::wstring(L"Video games have become an art form and an industry. "
                        L"The video game industry is of increasing") +
           L" commercial importance, with growth driven particularly by the "
           L"emerging Asian markets and mobile games." +
           L" As of 2015, video games generated sales of USD 74 billion "
           L"annually worldwide, and were the third-largest" +
           L" segment in the U.S. entertainment market, behind broadcast and "
           L"cable TV.";
    author = L"Ign";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(
        std::make_shared<Field>(categoryFieldName, L"videogames", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    title = L"Video games: the definition of fun on PC and consoles";
    text =
        std::wstring(L"A video game is an electronic game that involves human "
                     L"interaction with a user interface to generate") +
        L" visual feedback on a video device. The word video in video game "
        L"traditionally referred to a raster display device," +
        L"[1] but it now implies any type of display device that can produce "
        L"two- or three-dimensional images." +
        L" The electronic systems used to play video games are known as "
        L"platforms; examples of these are personal" +
        L" computers and video game consoles. These platforms range from large "
        L"mainframe computers to small handheld devices." +
        L" Specialized video games such as arcade games, while previously "
        L"common, have gradually declined in use.";
    author = L"Ign";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(
        std::make_shared<Field>(categoryFieldName, L"videogames", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    title = L"Video games: the history across PC, consoles and fun";
    text = std::wstring(L"Early games used interactive electronic devices with "
                        L"various display formats. The earliest example is") +
           L" from 1947—a device was filed for a patent on 25 January 1947, by "
           L"Thomas T. Goldsmith Jr. and Estle Ray Mann," +
           L" and issued on 14 December 1948, as U.S. Patent 2455992.[2]" +
           L"Inspired by radar display tech, it consisted of an analog device "
           L"that allowed a user to control a deque-drawn" +
           L" dot on the screen to simulate a missile being fired at targets, "
           L"which were drawings fixed to the screen.[3]";
    author = L"Ign";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(
        std::make_shared<Field>(categoryFieldName, L"videogames", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    title = L"Video games: the history";
    text = std::wstring(L"Early games used interactive electronic devices with "
                        L"various display formats. The earliest example is") +
           L" from 1947—a device was filed for a patent on 25 January 1947, by "
           L"Thomas T. Goldsmith Jr. and Estle Ray Mann," +
           L" and issued on 14 December 1948, as U.S. Patent 2455992.[2]" +
           L"Inspired by radar display tech, it consisted of an analog device "
           L"that allowed a user to control a deque-drawn" +
           L" dot on the screen to simulate a missile being fired at targets, "
           L"which were drawings fixed to the screen.[3]";
    author = L"Ign";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(
        std::make_shared<Field>(categoryFieldName, L"videogames", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    title = L"Batman: Arkham Knight PC Benchmarks, For What They're Worth";
    text =
        std::wstring(L"Although I didn’t spend much time playing Batman: "
                     L"Arkham Origins, I remember the game rather well after") +
        L" testing it on no less than 30 graphics cards and 20 CPUs. Arkham "
        L"Origins appeared to take full advantage of" +
        L" Unreal Engine 3, it ran smoothly on affordable GPUs, though it’s "
        L"worth remembering that Origins was developed " +
        L"for last-gen consoles.This week marked the arrival of Batman: Arkham "
        L"Knight, the fourth entry in WB’s Batman:" +
        L" Arkham series and a direct sequel to 2013’s Arkham Origins 2011’s "
        L"Arkham City." +
        L"Arkham Knight is also powered by Unreal Engine 3, but you can expect "
        L"noticeably improved graphics, in part because" +
        L" the PlayStation 4 and Xbox One have replaced the PS3 and 360 as the "
        L"lowest common denominator.";
    author = L"Rocksteady Studios";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"batman", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    title = L"Face-Off: Batman: Arkham Knight, the Dark Knight returns!";
    text =
        std::wstring(L"Despite the drama surrounding the PC release leading to "
                     L"its subsequent withdrawal, there's a sense of success") +
        L" in the console space as PlayStation 4 owners, and indeed those on "
        L"Xbox One, get a superb rendition of Batman:" +
        L" Arkham Knight. It's fair to say Rocksteady sized up each console's "
        L"strengths well ahead of producing its first" +
        L" current-gen title, and it's paid off in one of the best Batman "
        L"games we've seen in years.";
    author = L"Rocksteady Studios";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"batman", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    title = L"Batman: Arkham Knight Having More Trouble, But This Time not in "
            L"Gotham";
    text = std::wstring(
               L"As news began to break about the numerous issues affecting "
               L"the PC version of Batman: Arkham Knight, players") +
           L" of the console version breathed a sigh of relief and got back to "
           L"playing the game. Now players of the PlayStation" +
           L" 4 version are having problems of their own, albeit much less "
           L"severe ones." +
           L"This time Batman will have a difficult time in Gotham.";
    author = L"Rocksteady Studios";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"batman", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    title = L"Batman: Arkham Knight the new legend of Gotham";
    text = std::wstring(L"As news began to break about the numerous issues "
                        L"affecting the PC version of the game, players") +
           L" of the console version breathed a sigh of relief and got back to "
           L"play. Now players of the PlayStation" +
           L" 4 version are having problems of their own, albeit much less "
           L"severe ones.";
    author = L"Rocksteady Studios";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(std::make_shared<Field>(categoryFieldName, L"batman", ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    indexWriter::addDocument(doc);

    doc = std::make_shared<Document>();
    text = L"unlabeled doc";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    indexWriter::addDocument(doc);

    indexWriter::commit();
    return indexWriter::getReader();
  }

  virtual std::shared_ptr<Document> getVideoGameDocument()
  {
    std::shared_ptr<Document> doc = std::make_shared<Document>();
    std::wstring title = L"The new generation of PC and Console Video games";
    std::wstring text =
        std::wstring(
            L"Recently a lot of games have been released for the latest "
            L"generations of consoles and personal computers.") +
        L"One of them is Batman: Arkham Knight released recently on PS4, X-box "
        L"and personal computer." +
        L"Another important video game that will be released in November is "
        L"Assassin's Creed, a classic series that sees its new installement on "
        L"Halloween." +
        L"Recently a lot of problems affected the Assassin's creed series but "
        L"this time it should ran smoothly on affordable GPUs." +
        L"Players are waiting for the versions of their favourite video games "
        L"and so do we.";
    std::wstring author = L"Ign";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    return doc;
  }

  virtual std::shared_ptr<Document> getBatmanDocument()
  {
    std::shared_ptr<Document> doc = std::make_shared<Document>();
    std::wstring title = L"Batman: Arkham Knight new adventures for the super "
                         L"hero across Gotham, the Dark Knight has returned!";
    std::wstring title2 = L"I am a second title !";
    std::wstring text =
        std::wstring(
            L"This game is the electronic version of the famous super hero "
            L"adventures.It involves the interaction with the open world") +
        L" of the city of Gotham. Finally the player will be able to have fun "
        L"on its personal device." +
        L" The three-dimensional images of the game are stunning, because it "
        L"uses the Unreal Engine 3." +
        L" The systems available are PS4, X-Box and personal computer." +
        L" Will the simulate missile that is going to be  fired, success ?\" "
        L"+\n" +
        L" Will this video game make the history" +
        L" Help you favourite super hero to defeat all his enemies. The Dark "
        L"Knight has returned !";
    std::wstring author = L"Rocksteady Studios";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title2, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    return doc;
  }

  virtual std::shared_ptr<Document> getBatmanAmbiguosDocument()
  {
    std::shared_ptr<Document> doc = std::make_shared<Document>();
    std::wstring title =
        L"Batman: Arkham Knight new adventures for the super hero across "
        L"Gotham, the Dark Knight has returned! Batman will win !";
    std::wstring text =
        std::wstring(L"Early games used interactive electronic devices with "
                     L"various display formats. The earliest example is") +
        L" from 1947—a device was filed for a patent on 25 January 1947, by "
        L"Thomas T. Goldsmith Jr. and Estle Ray Mann," +
        L" and issued on 14 December 1948, as U.S. Patent 2455992.[2]" +
        L"Inspired by radar display tech, it consisted of an analog device "
        L"that allowed a user to control a deque-drawn" +
        L" dot on the screen to simulate a missile being fired at targets, "
        L"which were drawings fixed to the screen.[3]";
    std::wstring author = L"Ign";
    doc->push_back(std::make_shared<Field>(textFieldName, text, ft));
    doc->push_back(std::make_shared<Field>(titleFieldName, title, ft));
    doc->push_back(std::make_shared<Field>(authorFieldName, author, ft));
    doc->push_back(std::make_shared<Field>(booleanFieldName, L"false", ft));
    return doc;
  }

protected:
  std::shared_ptr<DocumentClassificationTestBase> shared_from_this()
  {
    return std::static_pointer_cast<DocumentClassificationTestBase>(
        org.apache.lucene.classification
            .ClassificationTestBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::classification::document
