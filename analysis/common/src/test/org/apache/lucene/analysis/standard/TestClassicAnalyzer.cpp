using namespace std;

#include "TestClassicAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/TextField.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/RAMDirectory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"
#include "../../../../../../java/org/apache/lucene/analysis/standard/ClassicAnalyzer.h"

namespace org::apache::lucene::analysis::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using TextField = org::apache::lucene::document::TextField;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using MultiFields = org::apache::lucene::index::MultiFields;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using RAMDirectory = org::apache::lucene::store::RAMDirectory;
using BytesRef = org::apache::lucene::util::BytesRef;

void TestClassicAnalyzer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<ClassicAnalyzer>();
}

void TestClassicAnalyzer::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestClassicAnalyzer::testMaxTermLength() 
{
  shared_ptr<ClassicAnalyzer> sa = make_shared<ClassicAnalyzer>();
  sa->setMaxTokenLength(5);
  assertAnalyzesTo(sa, L"ab cd toolong xy z",
                   std::deque<wstring>{L"ab", L"cd", L"xy", L"z"});
  delete sa;
}

void TestClassicAnalyzer::testMaxTermLength2() 
{
  shared_ptr<ClassicAnalyzer> sa = make_shared<ClassicAnalyzer>();
  assertAnalyzesTo(sa, L"ab cd toolong xy z",
                   std::deque<wstring>{L"ab", L"cd", L"toolong", L"xy", L"z"});
  sa->setMaxTokenLength(5);

  assertAnalyzesTo(sa, L"ab cd toolong xy z",
                   std::deque<wstring>{L"ab", L"cd", L"xy", L"z"},
                   std::deque<int>{1, 1, 2, 1});
  delete sa;
}

void TestClassicAnalyzer::testMaxTermLength3() 
{
  std::deque<wchar_t> chars(255);
  for (int i = 0; i < 255; i++) {
    chars[i] = L'a';
  }
  wstring longTerm = wstring(chars, 0, 255);

  assertAnalyzesTo(a, L"ab cd " + longTerm + L" xy z",
                   std::deque<wstring>{L"ab", L"cd", longTerm, L"xy", L"z"});
  assertAnalyzesTo(a, L"ab cd " + longTerm + L"a xy z",
                   std::deque<wstring>{L"ab", L"cd", L"xy", L"z"});
}

void TestClassicAnalyzer::testAlphanumeric() 
{
  // alphanumeric tokens
  assertAnalyzesTo(a, L"B2B", std::deque<wstring>{L"b2b"});
  assertAnalyzesTo(a, L"2B", std::deque<wstring>{L"2b"});
}

void TestClassicAnalyzer::testUnderscores() 
{
  // underscores are delimiters, but not in email addresses (below)
  assertAnalyzesTo(a, L"word_having_underscore",
                   std::deque<wstring>{L"word", L"having", L"underscore"});
  assertAnalyzesTo(a, L"word_with_underscore_and_stopwords",
                   std::deque<wstring>{L"word", L"underscore", L"stopwords"});
}

void TestClassicAnalyzer::testDelimiters() 
{
  // other delimiters: "-", "/", ","
  assertAnalyzesTo(a, L"some-dashed-phrase",
                   std::deque<wstring>{L"some", L"dashed", L"phrase"});
  assertAnalyzesTo(a, L"dogs,chase,cats",
                   std::deque<wstring>{L"dogs", L"chase", L"cats"});
  assertAnalyzesTo(a, L"ac/dc", std::deque<wstring>{L"ac", L"dc"});
}

void TestClassicAnalyzer::testApostrophes() 
{
  // internal apostrophes: O'Reilly, you're, O'Reilly's
  // possessives are actually removed by StardardFilter, not the tokenizer
  assertAnalyzesTo(a, L"O'Reilly", std::deque<wstring>{L"o'reilly"});
  assertAnalyzesTo(a, L"you're", std::deque<wstring>{L"you're"});
  assertAnalyzesTo(a, L"she's", std::deque<wstring>{L"she"});
  assertAnalyzesTo(a, L"Jim's", std::deque<wstring>{L"jim"});
  assertAnalyzesTo(a, L"don't", std::deque<wstring>{L"don't"});
  assertAnalyzesTo(a, L"O'Reilly's", std::deque<wstring>{L"o'reilly"});
}

void TestClassicAnalyzer::testTSADash() 
{
  // t and s had been stopwords in Lucene <= 2.0, which made it impossible
  // to correctly search for these terms:
  assertAnalyzesTo(a, L"s-class", std::deque<wstring>{L"s", L"class"});
  assertAnalyzesTo(a, L"t-com", std::deque<wstring>{L"t", L"com"});
  // 'a' is still a stopword:
  assertAnalyzesTo(a, L"a-class", std::deque<wstring>{L"class"});
}

void TestClassicAnalyzer::testCompanyNames() 
{
  // company names
  assertAnalyzesTo(a, L"AT&T", std::deque<wstring>{L"at&t"});
  assertAnalyzesTo(a, L"Excite@Home", std::deque<wstring>{L"excite@home"});
}

void TestClassicAnalyzer::testLucene1140() 
{
  try {
    shared_ptr<ClassicAnalyzer> analyzer = make_shared<ClassicAnalyzer>();
    assertAnalyzesTo(analyzer, L"www.nutch.org.",
                     std::deque<wstring>{L"www.nutch.org"},
                     std::deque<wstring>{L"<HOST>"});
    delete analyzer;
  } catch (const NullPointerException &e) {
    fail(L"Should not throw an NPE and it did");
  }
}

void TestClassicAnalyzer::testDomainNames() 
{
  // Current lucene should not show the bug
  shared_ptr<ClassicAnalyzer> a2 = make_shared<ClassicAnalyzer>();

  // domain names
  assertAnalyzesTo(a2, L"www.nutch.org",
                   std::deque<wstring>{L"www.nutch.org"});
  // Notice the trailing .  See
  // https://issues.apache.org/jira/browse/LUCENE-1068.
  // the following should be recognized as HOST:
  assertAnalyzesTo(a2, L"www.nutch.org.",
                   std::deque<wstring>{L"www.nutch.org"},
                   std::deque<wstring>{L"<HOST>"});

  // 2.3 should show the bug. But, alas, it's obsolete, we don't support it.
  // a2 = new ClassicAnalyzer(org.apache.lucene.util.Version.LUCENE_23);
  // assertAnalyzesTo(a2, "www.nutch.org.", new std::wstring[]{ "wwwnutchorg" }, new
  // std::wstring[] { "<ACRONYM>" });

  // 2.4 should not show the bug. But, alas, it's also obsolete,
  // so we check latest released (Robert's gonna break this on 4.0 soon :) )
  delete a2;
  a2 = make_shared<ClassicAnalyzer>();
  assertAnalyzesTo(a2, L"www.nutch.org.",
                   std::deque<wstring>{L"www.nutch.org"},
                   std::deque<wstring>{L"<HOST>"});
  delete a2;
}

void TestClassicAnalyzer::testEMailAddresses() 
{
  // email addresses, possibly with underscores, periods, etc
  assertAnalyzesTo(a, L"test@example.com",
                   std::deque<wstring>{L"test@example.com"});
  assertAnalyzesTo(a, L"first.lastname@example.com",
                   std::deque<wstring>{L"first.lastname@example.com"});
  assertAnalyzesTo(a, L"first_lastname@example.com",
                   std::deque<wstring>{L"first_lastname@example.com"});
}

void TestClassicAnalyzer::testNumeric() 
{
  // floating point, serial, model numbers, ip addresses, etc.
  // every other segment must have at least one digit
  assertAnalyzesTo(a, L"21.35", std::deque<wstring>{L"21.35"});
  assertAnalyzesTo(a, L"R2D2 C3PO", std::deque<wstring>{L"r2d2", L"c3po"});
  assertAnalyzesTo(a, L"216.239.63.104",
                   std::deque<wstring>{L"216.239.63.104"});
  assertAnalyzesTo(a, L"1-2-3", std::deque<wstring>{L"1-2-3"});
  assertAnalyzesTo(a, L"a1-b2-c3", std::deque<wstring>{L"a1-b2-c3"});
  assertAnalyzesTo(a, L"a1-b-c3", std::deque<wstring>{L"a1-b-c3"});
}

void TestClassicAnalyzer::testTextWithNumbers() 
{
  // numbers
  assertAnalyzesTo(a, L"David has 5000 bones",
                   std::deque<wstring>{L"david", L"has", L"5000", L"bones"});
}

void TestClassicAnalyzer::testVariousText() 
{
  // various
  assertAnalyzesTo(
      a, L"C embedded developers wanted",
      std::deque<wstring>{L"c", L"embedded", L"developers", L"wanted"});
  assertAnalyzesTo(a, L"foo bar FOO BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  assertAnalyzesTo(a, L"foo      bar .  FOO <> BAR",
                   std::deque<wstring>{L"foo", L"bar", L"foo", L"bar"});
  assertAnalyzesTo(a, L"\"QUOTED\" word",
                   std::deque<wstring>{L"quoted", L"word"});
}

void TestClassicAnalyzer::testAcronyms() 
{
  // acronyms have their dots stripped
  assertAnalyzesTo(a, L"U.S.A.", std::deque<wstring>{L"usa"});
}

void TestClassicAnalyzer::testCPlusPlusHash() 
{
  // It would be nice to change the grammar in StandardTokenizer.jj to make "C#"
  // and "C++" end up as tokens.
  assertAnalyzesTo(a, L"C++", std::deque<wstring>{L"c"});
  assertAnalyzesTo(a, L"C#", std::deque<wstring>{L"c"});
}

void TestClassicAnalyzer::testKorean() 
{
  // Korean words
  assertAnalyzesTo(a, L"안녕하세요 한글입니다",
                   std::deque<wstring>{L"안녕하세요", L"한글입니다"});
}

void TestClassicAnalyzer::testComplianceFileName() 
{
  assertAnalyzesTo(a, L"2004.jpg", std::deque<wstring>{L"2004.jpg"},
                   std::deque<wstring>{L"<HOST>"});
}

void TestClassicAnalyzer::testComplianceNumericIncorrect() 
{
  assertAnalyzesTo(a, L"62.46", std::deque<wstring>{L"62.46"},
                   std::deque<wstring>{L"<HOST>"});
}

void TestClassicAnalyzer::testComplianceNumericLong() 
{
  assertAnalyzesTo(a, L"978-0-94045043-1",
                   std::deque<wstring>{L"978-0-94045043-1"},
                   std::deque<wstring>{L"<NUM>"});
}

void TestClassicAnalyzer::testComplianceNumericFile() 
{
  assertAnalyzesTo(a, L"78academyawards/rules/rule02.html",
                   std::deque<wstring>{L"78academyawards/rules/rule02.html"},
                   std::deque<wstring>{L"<NUM>"});
}

void TestClassicAnalyzer::testComplianceNumericWithUnderscores() throw(
    runtime_error)
{
  assertAnalyzesTo(
      a, L"2006-03-11t082958z_01_ban130523_rtridst_0_ozabs",
      std::deque<wstring>{L"2006-03-11t082958z_01_ban130523_rtridst_0_ozabs"},
      std::deque<wstring>{L"<NUM>"});
}

void TestClassicAnalyzer::testComplianceNumericWithDash() 
{
  assertAnalyzesTo(a, L"mid-20th", std::deque<wstring>{L"mid-20th"},
                   std::deque<wstring>{L"<NUM>"});
}

void TestClassicAnalyzer::testComplianceManyTokens() 
{
  assertAnalyzesTo(
      a,
      wstring(L"/money.cnn.com/magazines/fortune/fortune_archive/2007/03/19/"
              L"8402357/index.htm ") +
          L"safari-0-sheikh-zayed-grand-mosque.jpg",
      std::deque<wstring>{L"money.cnn.com", L"magazines", L"fortune",
                           L"fortune", L"archive/2007/03/19/8402357",
                           L"index.htm", L"safari-0-sheikh", L"zayed", L"grand",
                           L"mosque.jpg"},
      std::deque<wstring>{L"<HOST>", L"<ALPHANUM>", L"<ALPHANUM>",
                           L"<ALPHANUM>", L"<NUM>", L"<HOST>", L"<NUM>",
                           L"<ALPHANUM>", L"<ALPHANUM>", L"<HOST>"});
}

void TestClassicAnalyzer::testJava14BWCompatibility() 
{
  shared_ptr<ClassicAnalyzer> sa = make_shared<ClassicAnalyzer>();
  assertAnalyzesTo(sa, L"test\u02C6test",
                   std::deque<wstring>{L"test", L"test"});
  delete sa;
}

void TestClassicAnalyzer::testWickedLongTerm() 
{
  shared_ptr<RAMDirectory> dir = make_shared<RAMDirectory>();
  shared_ptr<Analyzer> analyzer = make_shared<ClassicAnalyzer>();
  shared_ptr<IndexWriter> writer =
      make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(analyzer));

  std::deque<wchar_t> chars(IndexWriter::MAX_TERM_LENGTH);
  Arrays::fill(chars, L'x');
  shared_ptr<Document> doc = make_shared<Document>();
  const wstring bigTerm = wstring(chars);

  // This produces a too-long term:
  wstring contents = L"abc xyz x" + bigTerm + L" another term";
  doc->push_back(
      make_shared<TextField>(L"content", contents, Field::Store::NO));
  writer->addDocument(doc);

  // Make sure we can add another normal document
  doc = make_shared<Document>();
  doc->push_back(
      make_shared<TextField>(L"content", L"abc bbb ccc", Field::Store::NO));
  writer->addDocument(doc);
  delete writer;

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);

  // Make sure all terms < max size were indexed
  assertEquals(2, reader->docFreq(make_shared<Term>(L"content", L"abc")));
  assertEquals(1, reader->docFreq(make_shared<Term>(L"content", L"bbb")));
  assertEquals(1, reader->docFreq(make_shared<Term>(L"content", L"term")));
  assertEquals(1, reader->docFreq(make_shared<Term>(L"content", L"another")));

  // Make sure position is still incremented when
  // massive term is skipped:
  shared_ptr<PostingsEnum> tps = MultiFields::getTermPositionsEnum(
      reader, L"content", make_shared<BytesRef>(L"another"));
  assertTrue(tps->nextDoc() != DocIdSetIterator::NO_MORE_DOCS);
  assertEquals(1, tps->freq());
  assertEquals(3, tps->nextPosition());

  // Make sure the doc that has the massive term is in
  // the index:
  assertEquals(L"document with wicked long term should is not in the index!", 2,
               reader->numDocs());

  delete reader;

  // Make sure we can add a document with exactly the
  // maximum length term, and search on that term:
  doc = make_shared<Document>();
  doc->push_back(make_shared<TextField>(L"content", bigTerm, Field::Store::NO));
  shared_ptr<ClassicAnalyzer> sa = make_shared<ClassicAnalyzer>();
  sa->setMaxTokenLength(100000);
  writer = make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(sa));
  writer->addDocument(doc);
  delete writer;
  reader = DirectoryReader::open(dir);
  assertEquals(1, reader->docFreq(make_shared<Term>(L"content", bigTerm)));
  delete reader;

  delete dir;
  delete analyzer;
  delete sa;
}

void TestClassicAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<ClassicAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}

void TestClassicAnalyzer::testRandomHugeStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<ClassicAnalyzer>();
  checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER, 8192);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::standard