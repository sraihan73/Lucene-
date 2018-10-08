using namespace std;

#include "QualityQueriesFinder.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/DirectoryReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/IndexReader.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/MultiFields.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/Terms.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/index/TermsEnum.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/FSDirectory.h"

namespace org::apache::lucene::benchmark::quality::utils
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Terms = org::apache::lucene::index::Terms;
using MultiFields = org::apache::lucene::index::MultiFields;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
const wstring QualityQueriesFinder::newline =
    System::getProperty(L"line.separator");

QualityQueriesFinder::QualityQueriesFinder(shared_ptr<Directory> dir)
{
  this->dir = dir;
}

void QualityQueriesFinder::main(std::deque<wstring> &args) 
{
  if (args.size() < 1) {
    System::err::println(L"Usage: java QualityQueriesFinder <index-dir>");
    exit(1);
  }
  shared_ptr<QualityQueriesFinder> qqf =
      make_shared<QualityQueriesFinder>(FSDirectory::open(Paths->get(args[0])));
  std::deque<wstring> q = qqf->bestQueries(L"body", 20);
  for (int i = 0; i < q.size(); i++) {
    wcout << newline << formatQueryAsTrecTopic(i, q[i], L"", L"") << endl;
  }
}

std::deque<wstring>
QualityQueriesFinder::bestQueries(const wstring &field,
                                  int numQueries) 
{
  std::deque<wstring> words = bestTerms(L"body", 4 * numQueries);
  int n = words.size();
  int m = n / 4;
  std::deque<wstring> res(m);
  for (int i = 0; i < res.size(); i++) {
    res[i] = words[i] + L" " + words[m + i] + L"  " + words[n - 1 - m - i] +
             L" " + words[n - 1 - i];
    // System.out.println("query["+i+"]:  "+res[i]);
  }
  return res;
}

wstring QualityQueriesFinder::formatQueryAsTrecTopic(int qnum,
                                                     const wstring &title,
                                                     const wstring &description,
                                                     const wstring &narrative)
{
  return L"<top>" + newline + L"<num> Number: " + to_wstring(qnum) + newline +
         newline + L"<title> " + (title == L"" ? L"" : title) + newline +
         newline + L"<desc> Description:" + newline +
         (description == L"" ? L"" : description) + newline + newline +
         L"<narr> Narrative:" + newline + (narrative == L"" ? L"" : narrative) +
         newline + newline + L"</top>";
}

std::deque<wstring>
QualityQueriesFinder::bestTerms(const wstring &field,
                                int numTerms) 
{
  shared_ptr<PriorityQueue<std::shared_ptr<TermDf>>> pq =
      make_shared<TermsDfQueue>(numTerms);
  shared_ptr<IndexReader> ir = DirectoryReader::open(dir);
  try {
    int threshold = ir->maxDoc() / 10; // ignore words too common.
    shared_ptr<Terms> terms = MultiFields::getTerms(ir, field);
    if (terms != nullptr) {
      shared_ptr<TermsEnum> termsEnum = terms->begin();
      while (termsEnum->next() != nullptr) {
        int df = termsEnum->docFreq();
        if (df < threshold) {
          wstring ttxt = termsEnum->term()->utf8ToString();
          pq->insertWithOverflow(make_shared<TermDf>(ttxt, df));
        }
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    delete ir;
  }
  std::deque<wstring> res(pq->size());
  int i = 0;
  while (pq->size() > 0) {
    shared_ptr<TermDf> tdf = pq->pop();
    res[i++] = tdf->word;
    wcout << i << L".   word:  " << tdf->df << L"   " << tdf->word << endl;
  }
  return res;
}

QualityQueriesFinder::TermDf::TermDf(const wstring &word, int freq)
{
  this->word = word;
  this->df = freq;
}

QualityQueriesFinder::TermsDfQueue::TermsDfQueue(int maxSize)
    : org::apache::lucene::util::PriorityQueue<TermDf>(maxSize)
{
}

bool QualityQueriesFinder::TermsDfQueue::lessThan(shared_ptr<TermDf> tf1,
                                                  shared_ptr<TermDf> tf2)
{
  return tf1->df < tf2->df;
}
} // namespace org::apache::lucene::benchmark::quality::utils