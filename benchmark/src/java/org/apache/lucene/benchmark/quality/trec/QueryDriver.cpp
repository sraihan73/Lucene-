using namespace std;

#include "QueryDriver.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/FSDirectory.h"
#include "../Judge.h"
#include "../QualityBenchmark.h"
#include "../QualityQuery.h"
#include "../QualityQueryParser.h"
#include "../QualityStats.h"
#include "../utils/SimpleQQParser.h"
#include "../utils/SubmissionReport.h"
#include "TrecJudge.h"
#include "TrecTopicsReader.h"

namespace org::apache::lucene::benchmark::quality::trec
{
using SimpleQQParser =
    org::apache::lucene::benchmark::quality::utils::SimpleQQParser;
using SubmissionReport =
    org::apache::lucene::benchmark::quality::utils::SubmissionReport;
using namespace org::apache::lucene::benchmark::quality;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using FSDirectory = org::apache::lucene::store::FSDirectory;

void QueryDriver::main(std::deque<wstring> &args) 
{
  if (args.size() < 4 || args.size() > 5) {
    System::err::println(L"Usage: QueryDriver <topicsFile> <qrelsFile> "
                         L"<submissionFile> <indexDir> [querySpec]");
    System::err::println(L"topicsFile: input file containing queries");
    System::err::println(
        L"qrelsFile: input file containing relevance judgements");
    System::err::println(
        L"submissionFile: output submission file for trec_eval");
    System::err::println(L"indexDir: index directory");
    System::err::println(
        L"querySpec: string composed of fields to use in query consisting of "
        L"T=title,D=description,N=narrative:");
    System::err::println(L"\texample: TD (query on Title + Description). The "
                         L"default is T (title only)");
    exit(1);
  }

  shared_ptr<Path> topicsFile = Paths->get(args[0]);
  shared_ptr<Path> qrelsFile = Paths->get(args[1]);
  shared_ptr<Path> submissionFile = Paths->get(args[2]);
  shared_ptr<SubmissionReport> submitLog = make_shared<SubmissionReport>(
      make_shared<PrintWriter>(
          Files::newBufferedWriter(submissionFile, StandardCharsets::UTF_8)),
      L"lucene");
  shared_ptr<FSDirectory> dir = FSDirectory::open(Paths->get(args[3]));
  wstring fieldSpec = args.size() == 5
                          ? args[4]
                          : L"T"; // default to Title-only if not specified.
  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);

  int maxResults = 1000;
  wstring docNameField = L"docname";

  shared_ptr<PrintWriter> logger = make_shared<PrintWriter>(
      make_shared<OutputStreamWriter>(System::out, Charset::defaultCharset()),
      true);

  // use trec utilities to read trec topics into quality queries
  shared_ptr<TrecTopicsReader> qReader = make_shared<TrecTopicsReader>();
  std::deque<std::shared_ptr<QualityQuery>> qqs = qReader->readQueries(
      Files::newBufferedReader(topicsFile, StandardCharsets::UTF_8));

  // prepare judge, with trec utilities that read from a QRels file
  shared_ptr<Judge> judge = make_shared<TrecJudge>(
      Files::newBufferedReader(qrelsFile, StandardCharsets::UTF_8));

  // validate topics & judgments match each other
  judge->validateData(qqs, logger);

  shared_ptr<Set<wstring>> fieldSet = unordered_set<wstring>();
  if (fieldSpec.find(L'T') != wstring::npos) {
    fieldSet->add(L"title");
  }
  if (fieldSpec.find(L'D') != wstring::npos) {
    fieldSet->add(L"description");
  }
  if (fieldSpec.find(L'N') != wstring::npos) {
    fieldSet->add(L"narrative");
  }

  // set the parsing of quality queries into Lucene queries.
  shared_ptr<QualityQueryParser> qqParser = make_shared<SimpleQQParser>(
      fieldSet->toArray(std::deque<wstring>(0)), L"body");

  // run the benchmark
  shared_ptr<QualityBenchmark> qrun =
      make_shared<QualityBenchmark>(qqs, qqParser, searcher, docNameField);
  qrun->setMaxResults(maxResults);
  std::deque<std::shared_ptr<QualityStats>> stats =
      qrun->execute(judge, submitLog, logger);

  // print an avarage sum of the results
  shared_ptr<QualityStats> avg = QualityStats::average(stats);
  avg->log(L"SUMMARY", 2, logger, L"  ");
  delete reader;
  delete dir;
}
} // namespace org::apache::lucene::benchmark::quality::trec