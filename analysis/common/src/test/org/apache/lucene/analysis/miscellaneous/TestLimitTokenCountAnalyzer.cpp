using namespace std;

#include "TestLimitTokenCountAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Document.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/document/Field.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexWriterConfig.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Term.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockAnalyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenCountAnalyzer.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexWriter = org::apache::lucene::index::IndexWriter;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using Term = org::apache::lucene::index::Term;
using Directory = org::apache::lucene::store::Directory;
using TestUtil = org::apache::lucene::util::TestUtil;

void TestLimitTokenCountAnalyzer::testLimitTokenCountAnalyzer() throw(
    IOException)
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockAnalyzer> mock = make_shared<MockAnalyzer>(random());

    // if we are consuming all tokens, we can use the checks,
    // otherwise we can't
    mock->setEnableChecks(consumeAll);
    shared_ptr<Analyzer> a =
        make_shared<LimitTokenCountAnalyzer>(mock, 2, consumeAll);

    // dont use assertAnalyzesTo here, as the end offset is not the end of the
    // string (unless consumeAll is true, in which case it's correct)!
    assertTokenStreamContents(a->tokenStream(L"dummy", L"1  2     3  4  5"),
                              std::deque<wstring>{L"1", L"2"},
                              std::deque<int>{0, 3}, std::deque<int>{1, 4},
                              consumeAll ? 16 : nullptr);
    assertTokenStreamContents(a->tokenStream(L"dummy", L"1 2 3 4 5"),
                              std::deque<wstring>{L"1", L"2"},
                              std::deque<int>{0, 2}, std::deque<int>{1, 3},
                              consumeAll ? 9 : nullptr);

    // less than the limit, ensure we behave correctly
    assertTokenStreamContents(a->tokenStream(L"dummy", L"1  "),
                              std::deque<wstring>{L"1"}, std::deque<int>{0},
                              std::deque<int>{1}, consumeAll ? 3 : nullptr);

    // equal to limit
    assertTokenStreamContents(a->tokenStream(L"dummy", L"1  2  "),
                              std::deque<wstring>{L"1", L"2"},
                              std::deque<int>{0, 3}, std::deque<int>{1, 4},
                              consumeAll ? 6 : nullptr);
    delete a;
  }
}

void TestLimitTokenCountAnalyzer::testLimitTokenCountIndexWriter() throw(
    IOException)
{

  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<Directory> dir = newDirectory();
    int limit = TestUtil::nextInt(random(), 50, 101000);
    shared_ptr<MockAnalyzer> mock = make_shared<MockAnalyzer>(random());

    // if we are consuming all tokens, we can use the checks,
    // otherwise we can't
    mock->setEnableChecks(consumeAll);
    shared_ptr<Analyzer> a =
        make_shared<LimitTokenCountAnalyzer>(mock, limit, consumeAll);

    shared_ptr<IndexWriter> writer =
        make_shared<IndexWriter>(dir, make_shared<IndexWriterConfig>(a));

    shared_ptr<Document> doc = make_shared<Document>();
    shared_ptr<StringBuilder> b = make_shared<StringBuilder>();
    for (int i = 1; i < limit; i++) {
      b->append(L" a");
    }
    b->append(L" x");
    b->append(L" z");
    doc->push_back(newTextField(L"field", b->toString(), Field::Store::NO));
    writer->addDocument(doc);
    delete writer;

    shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
    shared_ptr<Term> t = make_shared<Term>(L"field", L"x");
    TestUtil::assertEquals(1, reader->docFreq(t));
    t = make_shared<Term>(L"field", L"z");
    TestUtil::assertEquals(0, reader->docFreq(t));
    delete reader;
    delete dir;
    delete a;
  }
}
} // namespace org::apache::lucene::analysis::miscellaneous