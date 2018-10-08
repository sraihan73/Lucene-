using namespace std;

#include "TrecJudge.h"
#include "../QualityQuery.h"

namespace org::apache::lucene::benchmark::quality::trec
{
using Judge = org::apache::lucene::benchmark::quality::Judge;
using QualityQuery = org::apache::lucene::benchmark::quality::QualityQuery;

TrecJudge::TrecJudge(shared_ptr<BufferedReader> reader) 
{
  judgements = unordered_map<wstring, std::shared_ptr<QRelJudgement>>();
  shared_ptr<QRelJudgement> curr = nullptr;
  wstring zero = L"0";
  wstring line;

  try {
    while (L"" != (line = reader->readLine())) {
      line = StringHelper::trim(line);
      if (line.length() == 0 || L'#' == line[0]) {
        continue;
      }
      shared_ptr<StringTokenizer> st = make_shared<StringTokenizer>(line);
      wstring queryID = st->nextToken();
      st->nextToken();
      wstring docName = st->nextToken();
      bool relevant = zero != st->nextToken();
      assert((!st->hasMoreTokens(),
              L"wrong format: " + line + L"  next: " + st->nextToken()));
      if (relevant) { // only keep relevant docs
        if (curr == nullptr || !curr->queryID.equals(queryID)) {
          curr = judgements[queryID];
          if (curr == nullptr) {
            curr = make_shared<QRelJudgement>(queryID);
            judgements.emplace(queryID, curr);
          }
        }
        curr->addRelevandDoc(docName);
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    reader->close();
  }
}

bool TrecJudge::isRelevant(const wstring &docName,
                           shared_ptr<QualityQuery> query)
{
  shared_ptr<QRelJudgement> qrj = judgements[query->getQueryID()];
  return qrj != nullptr && qrj->isRelevant(docName);
}

TrecJudge::QRelJudgement::QRelJudgement(const wstring &queryID)
{
  this->queryID = queryID;
  relevantDocs = unordered_map<wstring, wstring>();
}

void TrecJudge::QRelJudgement::addRelevandDoc(const wstring &docName)
{
  relevantDocs.emplace(docName, docName);
}

bool TrecJudge::QRelJudgement::isRelevant(const wstring &docName)
{
  return relevantDocs.find(docName) != relevantDocs.end();
}

int TrecJudge::QRelJudgement::maxRecall() { return relevantDocs.size(); }

bool TrecJudge::validateData(std::deque<std::shared_ptr<QualityQuery>> &qq,
                             shared_ptr<PrintWriter> logger)
{
  unordered_map<wstring, std::shared_ptr<QRelJudgement>> missingQueries =
      unordered_map<wstring, std::shared_ptr<QRelJudgement>>(judgements);
  deque<wstring> missingJudgements = deque<wstring>();
  for (int i = 0; i < qq.size(); i++) {
    wstring id = qq[i]->getQueryID();
    if (missingQueries.find(id) != missingQueries.end()) {
      missingQueries.erase(id);
    } else {
      missingJudgements.push_back(id);
    }
  }
  bool isValid = true;
  if (missingJudgements.size() > 0) {
    isValid = false;
    if (logger != nullptr) {
      logger->println(L"WARNING: " + missingJudgements.size() +
                      L" queries have no judgments! - ");
      for (int i = 0; i < missingJudgements.size(); i++) {
        logger->println(L"   " + missingJudgements[i]);
      }
    }
  }
  if (missingQueries.size() > 0) {
    isValid = false;
    if (logger != nullptr) {
      logger->println(L"WARNING: " + missingQueries.size() +
                      L" judgments match no query! - ");
      for (auto id : missingQueries) {
        logger->println(L"   " + id.first);
      }
    }
  }
  return isValid;
}

int TrecJudge::maxRecall(shared_ptr<QualityQuery> query)
{
  shared_ptr<QRelJudgement> qrj = judgements[query->getQueryID()];
  if (qrj != nullptr) {
    return qrj->maxRecall();
  }
  return 0;
}
} // namespace org::apache::lucene::benchmark::quality::trec