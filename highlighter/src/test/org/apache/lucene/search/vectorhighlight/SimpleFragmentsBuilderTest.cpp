using namespace std;

#include "SimpleFragmentsBuilderTest.h"

namespace org::apache::lucene::search::vectorhighlight
{
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using OpenMode = org::apache::lucene::index::IndexWriterConfig::OpenMode;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using Term = org::apache::lucene::index::Term;
using BooleanClause = org::apache::lucene::search::BooleanClause;
using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Query = org::apache::lucene::search::Query;
using TermQuery = org::apache::lucene::search::TermQuery;
using SimpleHTMLEncoder =
    org::apache::lucene::search::highlight::SimpleHTMLEncoder;
using Directory = org::apache::lucene::store::Directory;
using TestUtil = org::apache::lucene::util::TestUtil;

void SimpleFragmentsBuilderTest::test1TermIndex() 
{
  shared_ptr<FieldFragList> ffl =
      this->ffl(make_shared<TermQuery>(make_shared<Term>(F, L"a")), L"a");
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  TestUtil::assertEquals(L"<b>a</b>", sfb->createFragment(reader, 0, F, ffl));

  // change tags
  sfb = make_shared<SimpleFragmentsBuilder>(std::deque<wstring>{L"["},
                                            std::deque<wstring>{L"]"});
  TestUtil::assertEquals(L"[a]", sfb->createFragment(reader, 0, F, ffl));
}

void SimpleFragmentsBuilderTest::test2Frags() 
{
  shared_ptr<FieldFragList> ffl =
      this->ffl(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
                L"a b b b b b b b b b b b a b a b");
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  std::deque<wstring> f = sfb->createFragments(reader, 0, F, ffl, 3);
  // 3 snippets requested, but should be 2
  TestUtil::assertEquals(2, f.size());
  TestUtil::assertEquals(L"<b>a</b> b b b b b b b b b b", f[0]);
  TestUtil::assertEquals(L"b b <b>a</b> b <b>a</b> b", f[1]);
}

void SimpleFragmentsBuilderTest::test3Frags() 
{
  shared_ptr<BooleanQuery::Builder> booleanQuery =
      make_shared<BooleanQuery::Builder>();
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(F, L"a")),
                    BooleanClause::Occur::SHOULD);
  booleanQuery->add(make_shared<TermQuery>(make_shared<Term>(F, L"c")),
                    BooleanClause::Occur::SHOULD);

  shared_ptr<FieldFragList> ffl =
      this->ffl(booleanQuery->build(),
                L"a b b b b b b b b b b b a b a b b b b b c a a b b");
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  std::deque<wstring> f = sfb->createFragments(reader, 0, F, ffl, 3);
  TestUtil::assertEquals(3, f.size());
  TestUtil::assertEquals(L"<b>a</b> b b b b b b b b b b", f[0]);
  TestUtil::assertEquals(L"b b <b>a</b> b <b>a</b> b b b b b c", f[1]);
  TestUtil::assertEquals(L"<b>c</b> <b>a</b> <b>a</b> b b", f[2]);
}

void SimpleFragmentsBuilderTest::testTagsAndEncoder() 
{
  shared_ptr<FieldFragList> ffl = this->ffl(
      make_shared<TermQuery>(make_shared<Term>(F, L"a")), L"<h1> a </h1>");
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  std::deque<wstring> preTags = {L"["};
  std::deque<wstring> postTags = {L"]"};
  TestUtil::assertEquals(L"&lt;h1&gt; [a] &lt;&#x2F;h1&gt;",
                         sfb->createFragment(reader, 0, F, ffl, preTags,
                                             postTags,
                                             make_shared<SimpleHTMLEncoder>()));
}

shared_ptr<FieldFragList>
SimpleFragmentsBuilderTest::ffl(shared_ptr<Query> query,
                                const wstring &indexValue) 
{
  make1d1fIndex(indexValue);
  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(query, true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  return (make_shared<SimpleFragListBuilder>())->createFieldFragList(fpl, 20);
}

void SimpleFragmentsBuilderTest::test1PhraseShortMV() 
{
  makeIndexShortMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"d"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  // Should we probably be trimming?
  TestUtil::assertEquals(L"  a b c  <b>d</b> e",
                         sfb->createFragment(reader, 0, F, ffl));
}

void SimpleFragmentsBuilderTest::test1PhraseLongMV() 
{
  makeIndexLongMV();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"search", L"engines"}), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  TestUtil::assertEquals(
      L"customization: The most <b>search engines</b> use only one of these "
      L"methods. Even the <b>search engines</b> that says they can",
      sfb->createFragment(reader, 0, F, ffl));
}

void SimpleFragmentsBuilderTest::test1PhraseLongMVB() 
{
  makeIndexLongMVB();

  shared_ptr<FieldQuery> fq =
      make_shared<FieldQuery>(pqF({L"sp", L"pe", L"ee", L"ed"}), true,
                              true); // "speed" -(2gram)-> "sp","pe","ee","ed"
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  TestUtil::assertEquals(L"additional hardware. \nWhen you talk about "
                         L"processing <b>speed</b>, the",
                         sfb->createFragment(reader, 0, F, ffl));
}

void SimpleFragmentsBuilderTest::testUnstoredField() 
{
  makeUnstoredIndex();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"aaa"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  assertNull(sfb->createFragment(reader, 0, F, ffl));
}

void SimpleFragmentsBuilderTest::makeUnstoredIndex() 
{
  shared_ptr<IndexWriter> writer = make_shared<IndexWriter>(
      dir, (make_shared<IndexWriterConfig>(analyzerW))
               ->setOpenMode(IndexWriterConfig::OpenMode::CREATE));
  shared_ptr<Document> doc = make_shared<Document>();
  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorOffsets(true);
  customType->setStoreTermVectorPositions(true);
  doc->push_back(make_shared<Field>(F, L"aaa", customType));
  // doc.add( new Field( F, "aaa", Store.NO, Index.ANALYZED,
  // TermVector.WITH_POSITIONS_OFFSETS ) );
  writer->addDocument(doc);
  delete writer;
  if (reader != nullptr) {
    delete reader;
  }
  reader = DirectoryReader::open(dir);
}

void SimpleFragmentsBuilderTest::test1StrMV() 
{
  makeIndexStrMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"defg"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  sfb->setMultiValuedSeparator(L'/');
  TestUtil::assertEquals(L"abc/<b>defg</b>/hijkl",
                         sfb->createFragment(reader, 0, F, ffl));
}

void SimpleFragmentsBuilderTest::testMVSeparator() 
{
  makeIndexShortMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"d"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  sfb->setMultiValuedSeparator(L'/');
  TestUtil::assertEquals(L"//a b c//<b>d</b> e",
                         sfb->createFragment(reader, 0, F, ffl));
}

void SimpleFragmentsBuilderTest::testDiscreteMultiValueHighlighting() throw(
    runtime_error)
{
  makeIndexShortMV();

  shared_ptr<FieldQuery> fq = make_shared<FieldQuery>(tq(L"d"), true, true);
  shared_ptr<FieldTermStack> stack =
      make_shared<FieldTermStack>(reader, 0, F, fq);
  shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
  shared_ptr<SimpleFragListBuilder> sflb = make_shared<SimpleFragListBuilder>();
  shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 100);
  shared_ptr<SimpleFragmentsBuilder> sfb =
      make_shared<SimpleFragmentsBuilder>();
  sfb->setDiscreteMultiValueHighlighting(true);
  TestUtil::assertEquals(L"<b>d</b> e", sfb->createFragment(reader, 0, F, ffl));

  make1dmfIndex({L"some text to highlight", L"highlight other text"});
  fq = make_shared<FieldQuery>(tq(L"text"), true, true);
  stack = make_shared<FieldTermStack>(reader, 0, F, fq);
  fpl = make_shared<FieldPhraseList>(stack, fq);
  sflb = make_shared<SimpleFragListBuilder>();
  ffl = sflb->createFieldFragList(fpl, 32);
  std::deque<wstring> result = sfb->createFragments(reader, 0, F, ffl, 3);
  TestUtil::assertEquals(2, result.size());
  TestUtil::assertEquals(L"some <b>text</b> to highlight", result[0]);
  TestUtil::assertEquals(L"highlight other <b>text</b>", result[1]);

  fq = make_shared<FieldQuery>(tq(L"highlight"), true, true);
  stack = make_shared<FieldTermStack>(reader, 0, F, fq);
  fpl = make_shared<FieldPhraseList>(stack, fq);
  sflb = make_shared<SimpleFragListBuilder>();
  ffl = sflb->createFieldFragList(fpl, 32);
  result = sfb->createFragments(reader, 0, F, ffl, 3);
  TestUtil::assertEquals(2, result.size());
  TestUtil::assertEquals(L"text to <b>highlight</b>", result[0]);
  TestUtil::assertEquals(L"<b>highlight</b> other text", result[1]);
}

void SimpleFragmentsBuilderTest::
    testRandomDiscreteMultiValueHighlighting() 
{
  std::deque<wstring> randomValues(3 +
                                    random()->nextInt(10 * RANDOM_MULTIPLIER));
  for (int i = 0; i < randomValues.size(); i++) {
    wstring randomValue;
    do {
      randomValue = TestUtil::randomSimpleString(random());
    } while (L"" == randomValue);
    randomValues[i] = randomValue;
  }

  shared_ptr<Directory> dir = newDirectory();
  shared_ptr<RandomIndexWriter> writer = make_shared<RandomIndexWriter>(
      random(), dir,
      newIndexWriterConfig(make_shared<MockAnalyzer>(random()))
          ->setMergePolicy(newLogMergePolicy()));

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setStoreTermVectors(true);
  customType->setStoreTermVectorOffsets(true);
  customType->setStoreTermVectorPositions(true);

  int numDocs = randomValues.size() * 5;
  int numFields = 2 + random()->nextInt(5);
  int numTerms = 2 + random()->nextInt(3);
  deque<std::shared_ptr<Doc>> docs = deque<std::shared_ptr<Doc>>(numDocs);
  deque<std::shared_ptr<Document>> documents =
      deque<std::shared_ptr<Document>>(numDocs);
  unordered_map<wstring, Set<int>> valueToDocId =
      unordered_map<wstring, Set<int>>();
  for (int i = 0; i < numDocs; i++) {
    shared_ptr<Document> document = make_shared<Document>();
    // C++ NOTE: The following call to the 'RectangularVectors' helper class
    // reproduces the rectangular array initialization that is automatic in Java:
    // ORIGINAL LINE: std::wstring[][] fields = new std::wstring[numFields][numTerms];
    std::deque<std::deque<wstring>> fields =
        RectangularVectors::ReturnRectangularStdwstringVector(numFields,
                                                              numTerms);
    for (int j = 0; j < numFields; j++) {
      std::deque<wstring> fieldValues(numTerms);
      fieldValues[0] = getRandomValue(randomValues, valueToDocId, i);
      shared_ptr<StringBuilder> builder =
          make_shared<StringBuilder>(fieldValues[0]);
      for (int k = 1; k < numTerms; k++) {
        fieldValues[k] = getRandomValue(randomValues, valueToDocId, i);
        builder->append(L' ')->append(fieldValues[k]);
      }
      document->push_back(
          make_shared<Field>(F, builder->toString(), customType));
      fields[j] = fieldValues;
    }
    docs.push_back(make_shared<Doc>(fields));
    documents.push_back(document);
  }
  writer->addDocuments(documents);
  delete writer;
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);

  try {
    int highlightIters = 1 + random()->nextInt(120 * RANDOM_MULTIPLIER);
    for (int highlightIter = 0; highlightIter < highlightIters;
         highlightIter++) {
      wstring queryTerm = randomValues[random()->nextInt(randomValues.size())];
      int randomHit = valueToDocId[queryTerm]->begin()->next();
      deque<std::shared_ptr<StringBuilder>> builders =
          deque<std::shared_ptr<StringBuilder>>();
      for (auto fieldValues : docs[randomHit]->fieldValues) {
        shared_ptr<StringBuilder> builder = make_shared<StringBuilder>();
        bool hit = false;
        for (int i = 0; i < fieldValues.size(); i++) {
          if (queryTerm == fieldValues[i]) {
            builder->append(L"<b>")->append(queryTerm)->append(L"</b>");
            hit = true;
          } else {
            builder->append(fieldValues[i]);
          }
          if (i != fieldValues.size() - 1) {
            builder->append(L' ');
          }
        }
        if (hit) {
          builders.push_back(builder);
        }
      }

      shared_ptr<FieldQuery> fq =
          make_shared<FieldQuery>(tq(queryTerm), true, true);
      shared_ptr<FieldTermStack> stack =
          make_shared<FieldTermStack>(reader, randomHit, F, fq);

      shared_ptr<FieldPhraseList> fpl = make_shared<FieldPhraseList>(stack, fq);
      shared_ptr<SimpleFragListBuilder> sflb =
          make_shared<SimpleFragListBuilder>(100);
      shared_ptr<FieldFragList> ffl = sflb->createFieldFragList(fpl, 300);

      shared_ptr<SimpleFragmentsBuilder> sfb =
          make_shared<SimpleFragmentsBuilder>();
      sfb->setDiscreteMultiValueHighlighting(true);
      std::deque<wstring> actualFragments =
          sfb->createFragments(reader, randomHit, F, ffl, numFields);
      TestUtil::assertEquals(builders.size(), actualFragments.size());
      for (int i = 0; i < actualFragments.size(); i++) {
        TestUtil::assertEquals(builders[i]->toString(), actualFragments[i]);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete reader;
    delete dir;
  }
}

wstring SimpleFragmentsBuilderTest::getRandomValue(
    std::deque<wstring> &randomValues,
    unordered_map<wstring, Set<int>> &valueToDocId, int docId)
{
  wstring value = randomValues[random()->nextInt(randomValues.size())];
  if (valueToDocId.find(value) == valueToDocId.end()) {
    valueToDocId.emplace(value, unordered_set<int>());
  }
  valueToDocId[value]->add(docId);
  return value;
}

SimpleFragmentsBuilderTest::Doc::Doc(
    std::deque<std::deque<wstring>> &fieldValues)
    : fieldValues(fieldValues)
{
}
} // namespace org::apache::lucene::search::vectorhighlight