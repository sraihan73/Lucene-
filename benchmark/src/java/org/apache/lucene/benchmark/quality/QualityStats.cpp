using namespace std;

#include "QualityStats.h"

namespace org::apache::lucene::benchmark::quality
{

QualityStats::RecallPoint::RecallPoint(int rank, double recall)
{
  this->rank = rank;
  this->recall = recall;
}

int QualityStats::RecallPoint::getRank() { return rank; }

double QualityStats::RecallPoint::getRecall() { return recall; }

QualityStats::QualityStats(double maxGoodPoints, int64_t searchTime)
{
  this->maxGoodPoints = maxGoodPoints;
  this->searchTime = searchTime;
  this->recallPoints = deque<std::shared_ptr<RecallPoint>>();
  pAt = std::deque<double>(MAX_POINTS + 1); // pAt[0] unused.
}

void QualityStats::addResult(int n, bool isRelevant,
                             int64_t docNameExtractTime)
{
  if (abs(numPoints + 1 - n) > 1E-6) {
    throw invalid_argument(L"point " + to_wstring(n) + L" illegal after " +
                           to_wstring(numPoints) + L" points!");
  }
  if (isRelevant) {
    numGoodPoints += 1;
    recallPoints.push_back(make_shared<RecallPoint>(n, numGoodPoints));
    if (recallPoints.size() == 1 &&
        n <= 5) { // first point, but only within 5 top scores.
      mrr = 1.0 / n;
    }
  }
  numPoints = n;
  double p = numGoodPoints / numPoints;
  if (isRelevant) {
    pReleventSum += p;
  }
  if (n < pAt.size()) {
    pAt[n] = p;
  }
  recall = maxGoodPoints <= 0 ? p : numGoodPoints / maxGoodPoints;
  docNamesExtractTime += docNameExtractTime;
}

double QualityStats::getPrecisionAt(int n)
{
  if (n < 1 || n > MAX_POINTS) {
    throw invalid_argument(L"n=" + to_wstring(n) + L" - but it must be in [1," +
                           to_wstring(MAX_POINTS) + L"] range!");
  }
  if (n > numPoints) {
    return (numPoints * pAt[static_cast<int>(numPoints)]) / n;
  }
  return pAt[n];
}

double QualityStats::getAvp()
{
  return maxGoodPoints == 0 ? 0 : pReleventSum / maxGoodPoints;
}

double QualityStats::getRecall() { return recall; }

void QualityStats::log(const wstring &title, int paddLines,
                       shared_ptr<PrintWriter> logger, const wstring &prefix)
{
  for (int i = 0; i < paddLines; i++) {
    logger->println();
  }
  if (title != L"" && StringHelper::trim(title)->length() > 0) {
    logger->println(title);
  }
  prefix = prefix == L"" ? L"" : prefix;
  shared_ptr<NumberFormat> nf = NumberFormat::getInstance(Locale::ROOT);
  nf->setMaximumFractionDigits(3);
  nf->setMinimumFractionDigits(3);
  nf->setGroupingUsed(true);
  int M = 19;
  logger->println(
      prefix + format(L"Search Seconds: ", M) +
      fracFormat(nf->format(static_cast<double>(searchTime) / 1000)));
  logger->println(
      prefix + format(L"DocName Seconds: ", M) +
      fracFormat(nf->format(static_cast<double>(docNamesExtractTime) / 1000)));
  logger->println(prefix + format(L"Num Points: ", M) +
                  fracFormat(nf->format(numPoints)));
  logger->println(prefix + format(L"Num Good Points: ", M) +
                  fracFormat(nf->format(numGoodPoints)));
  logger->println(prefix + format(L"Max Good Points: ", M) +
                  fracFormat(nf->format(maxGoodPoints)));
  logger->println(prefix + format(L"Average Precision: ", M) +
                  fracFormat(nf->format(getAvp())));
  logger->println(prefix + format(L"MRR: ", M) +
                  fracFormat(nf->format(getMRR())));
  logger->println(prefix + format(L"Recall: ", M) +
                  fracFormat(nf->format(getRecall())));
  for (int i = 1; i < static_cast<int>(numPoints) && i < pAt.size(); i++) {
    logger->println(prefix +
                    format(L"Precision At " + to_wstring(i) + L": ", M) +
                    fracFormat(nf->format(getPrecisionAt(i))));
  }
  for (int i = 0; i < paddLines; i++) {
    logger->println();
  }
}

wstring QualityStats::padd = L"                                    ";

wstring QualityStats::format(const wstring &s, int minLen)
{
  s = (s == L"" ? L"" : s);
  int n = max(minLen, s.length());
  return (s + padd).substr(0, n);
}

wstring QualityStats::fracFormat(const wstring &frac)
{
  int k = (int)frac.find(L'.');
  wstring s1 = padd + frac.substr(0, k);
  int n = max(k, 6);
  s1 = s1.substr(s1.length() - n);
  return s1 + frac.substr(k);
}

shared_ptr<QualityStats>
QualityStats::average(std::deque<std::shared_ptr<QualityStats>> &stats)
{
  shared_ptr<QualityStats> avg = make_shared<QualityStats>(0, 0);
  if (stats.empty()) {
    // weired, no stats to average!
    return avg;
  }
  int m = 0; // queries with positive judgements
  // aggregate
  for (int i = 0; i < stats.size(); i++) {
    avg->searchTime += stats[i]->searchTime;
    avg->docNamesExtractTime += stats[i]->docNamesExtractTime;
    if (stats[i]->maxGoodPoints > 0) {
      m++;
      avg->numGoodPoints += stats[i]->numGoodPoints;
      avg->numPoints += stats[i]->numPoints;
      avg->pReleventSum += stats[i]->getAvp();
      avg->recall += stats[i]->recall;
      avg->mrr += stats[i]->getMRR();
      avg->maxGoodPoints += stats[i]->maxGoodPoints;
      for (int j = 1; j < avg->pAt.size(); j++) {
        avg->pAt[j] += stats[i]->getPrecisionAt(j);
      }
    }
  }
  assert((m > 0, L"Fishy: no \"good\" queries!"));
  // take average: times go by all queries, other measures go by "good" queries
  // only.
  avg->searchTime /= stats.size();
  avg->docNamesExtractTime /= stats.size();
  avg->numGoodPoints /= m;
  avg->numPoints /= m;
  avg->recall /= m;
  avg->mrr /= m;
  avg->maxGoodPoints /= m;
  for (int j = 1; j < avg->pAt.size(); j++) {
    avg->pAt[j] /= m;
  }
  avg->pReleventSum /= m;                  // this is actually avgp now
  avg->pReleventSum *= avg->maxGoodPoints; // so that getAvgP() would be correct

  return avg;
}

int64_t QualityStats::getDocNamesExtractTime() { return docNamesExtractTime; }

double QualityStats::getMaxGoodPoints() { return maxGoodPoints; }

double QualityStats::getNumGoodPoints() { return numGoodPoints; }

double QualityStats::getNumPoints() { return numPoints; }

std::deque<std::shared_ptr<RecallPoint>> QualityStats::getRecallPoints()
{
  return recallPoints.toArray(std::deque<std::shared_ptr<RecallPoint>>(0));
}

double QualityStats::getMRR() { return mrr; }

int64_t QualityStats::getSearchTime() { return searchTime; }
} // namespace org::apache::lucene::benchmark::quality