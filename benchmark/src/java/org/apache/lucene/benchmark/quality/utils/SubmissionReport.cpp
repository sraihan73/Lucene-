using namespace std;

#include "SubmissionReport.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/ScoreDoc.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/search/TopDocs.h"
#include "../QualityQuery.h"
#include "DocNameExtractor.h"

namespace org::apache::lucene::benchmark::quality::utils
{
using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;
using ScoreDoc = org::apache::lucene::search::ScoreDoc;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using TopDocs = org::apache::lucene::search::TopDocs;

SubmissionReport::SubmissionReport(shared_ptr<PrintWriter> logger,
                                   const wstring &name)
{
  this->logger = logger;
  this->name = name;
  nf = NumberFormat::getInstance(Locale::ROOT);
  nf->setMaximumFractionDigits(4);
  nf->setMinimumFractionDigits(4);
}

void SubmissionReport::report(
    shared_ptr<QualityQuery> qq, shared_ptr<TopDocs> td,
    const wstring &docNameField,
    shared_ptr<IndexSearcher> searcher) 
{
  if (logger == nullptr) {
    return;
  }
  std::deque<std::shared_ptr<ScoreDoc>> sd = td->scoreDocs;
  wstring sep = L" \t ";
  shared_ptr<DocNameExtractor> xt = make_shared<DocNameExtractor>(docNameField);
  for (int i = 0; i < sd.size(); i++) {
    wstring docName = xt->docName(searcher, sd[i]->doc);
    logger->println(qq->getQueryID() + sep + L"Q0" + sep + format(docName, 20) +
                    sep + format(L"" + to_wstring(i), 7) + sep +
                    nf->format(sd[i]->score) + sep + name);
  }
}

void SubmissionReport::flush()
{
  if (logger != nullptr) {
    logger->flush();
  }
}

wstring SubmissionReport::padd = L"                                    ";

wstring SubmissionReport::format(const wstring &s, int minLen)
{
  s = (s == L"" ? L"" : s);
  int n = max(minLen, s.length());
  return (s + padd).substr(0, n);
}
} // namespace org::apache::lucene::benchmark::quality::utils