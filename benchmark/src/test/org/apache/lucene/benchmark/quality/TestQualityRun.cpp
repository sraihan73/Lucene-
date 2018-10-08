using namespace std;

#include "TestQualityRun.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/Judge.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/QualityBenchmark.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/QualityQuery.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/QualityQueryParser.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/QualityStats.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/trec/TrecJudge.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/trec/TrecTopicsReader.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/utils/SimpleQQParser.h"
#include "../../../../../../java/org/apache/lucene/benchmark/quality/utils/SubmissionReport.h"

namespace org::apache::lucene::benchmark::quality
{
using BenchmarkTestCase = org::apache::lucene::benchmark::BenchmarkTestCase;
using TrecJudge = org::apache::lucene::benchmark::quality::trec::TrecJudge;
using TrecTopicsReader =
    org::apache::lucene::benchmark::quality::trec::TrecTopicsReader;
using SimpleQQParser =
    org::apache::lucene::benchmark::quality::utils::SimpleQQParser;
using SubmissionReport =
    org::apache::lucene::benchmark::quality::utils::SubmissionReport;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Directory = org::apache::lucene::store::Directory;

void TestQualityRun::setUp() 
{
  BenchmarkTestCase::setUp();
  copyToWorkDir(L"reuters.578.lines.txt.bz2");
}

void TestQualityRun::testTrecQuality() 
{
  // first create the partial reuters index
  createReutersIndex();

  int maxResults = 1000;
  wstring docNameField =
      L"doctitle"; // orig docID is in the linedoc format title

  shared_ptr<PrintWriter> logger =
      VERBOSE ? make_shared<PrintWriter>(
                    make_shared<OutputStreamWriter>(System::out,
                                                    Charset::defaultCharset()),
                    true)
              : nullptr;

  // prepare topics
  shared_ptr<InputStream> topics =
      getClass().getResourceAsStream(L"trecTopics.txt");
  shared_ptr<TrecTopicsReader> qReader = make_shared<TrecTopicsReader>();
  std::deque<std::shared_ptr<QualityQuery>> qqs =
      qReader->readQueries(make_shared<BufferedReader>(
          make_shared<InputStreamReader>(topics, StandardCharsets::UTF_8)));

  // prepare judge
  shared_ptr<InputStream> qrels =
      getClass().getResourceAsStream(L"trecQRels.txt");
  shared_ptr<Judge> judge = make_shared<TrecJudge>(make_shared<BufferedReader>(
      make_shared<InputStreamReader>(qrels, StandardCharsets::UTF_8)));

  // validate topics & judgments match each other
  judge->validateData(qqs, logger);

  shared_ptr<Directory> dir = newFSDirectory(getWorkDir()->resolve(L"index"));
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);

  shared_ptr<QualityQueryParser> qqParser =
      make_shared<SimpleQQParser>(L"title", L"body");
  shared_ptr<QualityBenchmark> qrun =
      make_shared<QualityBenchmark>(qqs, qqParser, searcher, docNameField);

  shared_ptr<SubmissionReport> submitLog =
      VERBOSE ? make_shared<SubmissionReport>(logger, L"TestRun") : nullptr;
  qrun->setMaxResults(maxResults);
  std::deque<std::shared_ptr<QualityStats>> stats =
      qrun->execute(judge, submitLog, logger);

  // --------- verify by the way judgments were altered for this test:
  // for some queries, depending on m = qnum % 8
  // m==0: avg_precision and recall are hurt, by marking fake docs as relevant
  // m==1: precision_at_n and avg_precision are hurt, by unmarking relevant docs
  // m==2: all precision, precision_at_n and recall are hurt.
  // m>=3: these queries remain perfect
  for (int i = 0; i < stats.size(); i++) {
    shared_ptr<QualityStats> s = stats[i];
    switch (i % 8) {

    case 0:
      assertTrue(L"avg-p should be hurt: " + to_wstring(s->getAvp()),
                 1.0 > s->getAvp());
      assertTrue(L"recall should be hurt: " + to_wstring(s->getRecall()),
                 1.0 > s->getRecall());
      for (int j = 1; j <= QualityStats::MAX_POINTS; j++) {
        assertEquals(L"p_at_" + to_wstring(j) + L" should be perfect: " +
                         to_wstring(s->getPrecisionAt(j)),
                     1.0, s->getPrecisionAt(j), 1E-2);
      }
      break;

    case 1:
      assertTrue(L"avg-p should be hurt", 1.0 > s->getAvp());
      assertEquals(L"recall should be perfect: " + to_wstring(s->getRecall()),
                   1.0, s->getRecall(), 1E-2);
      for (int j = 1; j <= QualityStats::MAX_POINTS; j++) {
        assertTrue(L"p_at_" + to_wstring(j) + L" should be hurt: " +
                       to_wstring(s->getPrecisionAt(j)),
                   1.0 > s->getPrecisionAt(j));
      }
      break;

    case 2:
      assertTrue(L"avg-p should be hurt: " + to_wstring(s->getAvp()),
                 1.0 > s->getAvp());
      assertTrue(L"recall should be hurt: " + to_wstring(s->getRecall()),
                 1.0 > s->getRecall());
      for (int j = 1; j <= QualityStats::MAX_POINTS; j++) {
        assertTrue(L"p_at_" + to_wstring(j) + L" should be hurt: " +
                       to_wstring(s->getPrecisionAt(j)),
                   1.0 > s->getPrecisionAt(j));
      }
      break;

    default: {
      assertEquals(L"avg-p should be perfect: " + to_wstring(s->getAvp()), 1.0,
                   s->getAvp(), 1E-2);
      assertEquals(L"recall should be perfect: " + to_wstring(s->getRecall()),
                   1.0, s->getRecall(), 1E-2);
      for (int j = 1; j <= QualityStats::MAX_POINTS; j++) {
        assertEquals(L"p_at_" + to_wstring(j) + L" should be perfect: " +
                         to_wstring(s->getPrecisionAt(j)),
                     1.0, s->getPrecisionAt(j), 1E-2);
      }
    }
    }
  }

  shared_ptr<QualityStats> avg = QualityStats::average(stats);
  if (logger != nullptr) {
    avg->log(L"Average statistis:", 1, logger, L"  ");
  }

  assertTrue(L"mean avg-p should be hurt: " + to_wstring(avg->getAvp()),
             1.0 > avg->getAvp());
  assertTrue(L"avg recall should be hurt: " + to_wstring(avg->getRecall()),
             1.0 > avg->getRecall());
  for (int j = 1; j <= QualityStats::MAX_POINTS; j++) {
    assertTrue(L"avg p_at_" + to_wstring(j) + L" should be hurt: " +
                   to_wstring(avg->getPrecisionAt(j)),
               1.0 > avg->getPrecisionAt(j));
  }

  delete reader;
  delete dir;
}

void TestQualityRun::testTrecTopicsReader() 
{
  // prepare topics
  shared_ptr<InputStream> topicsFile =
      getClass().getResourceAsStream(L"trecTopics.txt");
  shared_ptr<TrecTopicsReader> qReader = make_shared<TrecTopicsReader>();
  std::deque<std::shared_ptr<QualityQuery>> qqs =
      qReader->readQueries(make_shared<BufferedReader>(
          make_shared<InputStreamReader>(topicsFile, StandardCharsets::UTF_8)));

  assertEquals(20, qqs.size());

  shared_ptr<QualityQuery> qq = qqs[0];
  assertEquals(L"statement months  total 1987", qq->getValue(L"title"));
  assertEquals(L"Topic 0 Description Line 1 Topic 0 Description Line 2",
               qq->getValue(L"description"));
  assertEquals(L"Topic 0 Narrative Line 1 Topic 0 Narrative Line 2",
               qq->getValue(L"narrative"));

  qq = qqs[1];
  assertEquals(L"agreed 15  against five", qq->getValue(L"title"));
  assertEquals(L"Topic 1 Description Line 1 Topic 1 Description Line 2",
               qq->getValue(L"description"));
  assertEquals(L"Topic 1 Narrative Line 1 Topic 1 Narrative Line 2",
               qq->getValue(L"narrative"));

  qq = qqs[19];
  assertEquals(L"20 while  common week", qq->getValue(L"title"));
  assertEquals(L"Topic 19 Description Line 1 Topic 19 Description Line 2",
               qq->getValue(L"description"));
  assertEquals(L"Topic 19 Narrative Line 1 Topic 19 Narrative Line 2",
               qq->getValue(L"narrative"));
}

void TestQualityRun::createReutersIndex() 
{
  // 1. alg definition
  std::deque<wstring> algLines = {
      L"# ----- properties ",
      L"content.source=org.apache.lucene.benchmark.byTask.feeds.LineDocSource",
      L"analyzer=org.apache.lucene.analysis.standard.ClassicAnalyzer",
      L"docs.file=" + getWorkDirResourcePath(L"reuters.578.lines.txt.bz2"),
      L"content.source.log.step=2500",
      L"doc.term.deque=false",
      L"content.source.forever=false",
      L"directory=FSDirectory",
      L"doc.stored=true",
      L"doc.tokenized=true",
      L"# ----- alg ",
      L"ResetSystemErase",
      L"CreateIndex",
      L"{ AddDoc } : *",
      L"CloseIndex"};

  // 2. execute the algorithm  (required in every "logic" test)
  execBenchmark(algLines);
}
} // namespace org::apache::lucene::benchmark::quality