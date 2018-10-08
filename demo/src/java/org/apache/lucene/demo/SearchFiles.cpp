using namespace std;

#include "SearchFiles.h"

namespace org::apache::lucene::demo
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using StandardAnalyzer =
    org::apache::lucene::analysis::standard::StandardAnalyzer;
using Document = org::apache::lucene::document::Document;
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using QueryParser = org::apache::lucene::queryparser::classic::QueryParser;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using TopDocs = org::apache::lucene::search::TopDocs;
using FSDirectory = org::apache::lucene::store::FSDirectory;

SearchFiles::SearchFiles() {}

void SearchFiles::main(std::deque<wstring> &args) 
{
  wstring usage = L"Usage:\tjava org.apache.lucene.demo.SearchFiles [-index "
                  L"dir] [-field f] [-repeat n] [-queries file] [-query "
                  L"string] [-raw] [-paging hitsPerPage]\n\nSee "
                  L"http://lucene.apache.org/core/4_1_0/demo/ for details.";
  if (args.size() > 0 && (L"-h" == args[0] || L"-help" == args[0])) {
    wcout << usage << endl;
    exit(0);
  }

  wstring index = L"index";
  wstring field = L"contents";
  wstring queries = L"";
  int repeat = 0;
  bool raw = false;
  wstring queryString = L"";
  int hitsPerPage = 10;

  for (int i = 0; i < args.size(); i++) {
    if (L"-index" == args[i]) {
      index = args[i + 1];
      i++;
    } else if (L"-field" == args[i]) {
      field = args[i + 1];
      i++;
    } else if (L"-queries" == args[i]) {
      queries = args[i + 1];
      i++;
    } else if (L"-query" == args[i]) {
      queryString = args[i + 1];
      i++;
    } else if (L"-repeat" == args[i]) {
      repeat = stoi(args[i + 1]);
      i++;
    } else if (L"-raw" == args[i]) {
      raw = true;
    } else if (L"-paging" == args[i]) {
      hitsPerPage = stoi(args[i + 1]);
      if (hitsPerPage <= 0) {
        System::err::println(L"There must be at least 1 hit per page.");
        exit(1);
      }
      i++;
    }
  }

  shared_ptr<IndexReader> reader =
      DirectoryReader::open(FSDirectory::open(Paths->get(index)));
  shared_ptr<IndexSearcher> searcher = make_shared<IndexSearcher>(reader);
  shared_ptr<Analyzer> analyzer = make_shared<StandardAnalyzer>();

  shared_ptr<BufferedReader> in_ = nullptr;
  if (queries != L"") {
    in_ =
        Files::newBufferedReader(Paths->get(queries), StandardCharsets::UTF_8);
  } else {
    in_ = make_shared<BufferedReader>(
        make_shared<InputStreamReader>(System::in, StandardCharsets::UTF_8));
  }
  shared_ptr<QueryParser> parser = make_shared<QueryParser>(field, analyzer);
  while (true) {
    if (queries == L"" && queryString == L"") { // prompt the user
      wcout << L"Enter query: " << endl;
    }

    wstring line = queryString != L"" ? queryString : in_->readLine();

    if (line == L"" || line.length() == -1) {
      break;
    }

    line = StringHelper::trim(line);
    if (line.length() == 0) {
      break;
    }

    shared_ptr<Query> query = parser->parse(line);
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << L"Searching for: " << query->toString(field) << endl;

    if (repeat > 0) { // repeat & time as benchmark
      Date start = Date();
      for (int i = 0; i < repeat; i++) {
        searcher->search(query, 100);
      }
      Date end = Date();
      wcout << L"Time: " << (end.getTime() - start.getTime()) << L"ms" << endl;
    }

    doPagingSearch(in_, searcher, query, hitsPerPage, raw,
                   queries == L"" && queryString == L"");

    if (queryString != L"") {
      break;
    }
  }
  delete reader;
}

void SearchFiles::doPagingSearch(shared_ptr<BufferedReader> in_,
                                 shared_ptr<IndexSearcher> searcher,
                                 shared_ptr<Query> query, int hitsPerPage,
                                 bool raw, bool interactive) 
{

  // Collect enough docs to show 5 pages
  shared_ptr<TopDocs> results = searcher->search(query, 5 * hitsPerPage);
  std::deque<std::shared_ptr<ScoreDoc>> hits = results->scoreDocs;

  int numTotalHits = Math::toIntExact(results->totalHits);
  wcout << numTotalHits << L" total matching documents" << endl;

  int start = 0;
  int end = min(numTotalHits, hitsPerPage);

  while (true) {
    if (end > hits.size()) {
      wcout << L"Only results 1 - " << hits.size() << L" of " << numTotalHits
            << L" total matching documents collected." << endl;
      wcout << L"Collect more (y/n) ?" << endl;
      wstring line = in_->readLine();
      if (line.length() == 0 || line[0] == L'n') {
        break;
      }

      hits = searcher->search(query, numTotalHits)->scoreDocs;
    }

    end = min(hits.size(), start + hitsPerPage);

    for (int i = start; i < end; i++) {
      if (raw) { // output raw format
        wcout << L"doc=" << hits[i]->doc << L" score=" << hits[i]->score
              << endl;
        continue;
      }

      shared_ptr<Document> doc = searcher->doc(hits[i]->doc);
      wstring path = doc[L"path"];
      if (path != L"") {
        wcout << (i << 1) << L". " << path << endl;
        wstring title = doc[L"title"];
        if (title != L"") {
          wcout << L"   Title: " << doc[L"title"] << endl;
        }
      } else {
        wcout << (i << 1) << L". " << L"No path for this document" << endl;
      }
    }

    if (!interactive || end == 0) {
      break;
    }

    if (numTotalHits >= end) {
      bool quit = false;
      while (true) {
        wcout << L"Press ";
        if (start - hitsPerPage >= 0) {
          wcout << L"(p)revious page, ";
        }
        if (start + hitsPerPage < numTotalHits) {
          wcout << L"(n)ext page, ";
        }
        wcout << L"(q)uit or enter number to jump to a page." << endl;

        wstring line = in_->readLine();
        if (line.length() == 0 || line[0] == L'q') {
          quit = true;
          break;
        }
        if (line[0] == L'p') {
          start = max(0, start - hitsPerPage);
          break;
        } else if (line[0] == L'n') {
          if (start + hitsPerPage < numTotalHits) {
            start += hitsPerPage;
          }
          break;
        } else {
          int page = stoi(line);
          if ((page - 1) * hitsPerPage < numTotalHits) {
            start = (page - 1) * hitsPerPage;
            break;
          } else {
            wcout << L"No such page" << endl;
          }
        }
      }
      if (quit) {
        break;
      }
      end = min(numTotalHits, start + hitsPerPage);
    }
  }
}
} // namespace org::apache::lucene::demo