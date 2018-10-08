using namespace std;

#include "HighFreqTerms.h"

namespace org::apache::lucene::misc
{
using DirectoryReader = org::apache::lucene::index::DirectoryReader;
using IndexReader = org::apache::lucene::index::IndexReader;
using MultiFields = org::apache::lucene::index::MultiFields;
using Fields = org::apache::lucene::index::Fields;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Terms = org::apache::lucene::index::Terms;
using Directory = org::apache::lucene::store::Directory;
using FSDirectory = org::apache::lucene::store::FSDirectory;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;
using BytesRef = org::apache::lucene::util::BytesRef;
using org::apache::lucene::util::SuppressForbidden;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public static void main(std::wstring[] args) throws Exception
void HighFreqTerms::main(std::deque<wstring> &args) 
{
  wstring field = L"";
  int numTerms = DEFAULT_NUMTERMS;

  if (args.empty() || args.size() > 4) {
    usage();
    exit(1);
  }

  shared_ptr<Directory> dir = FSDirectory::open(Paths->get(args[0]));

  shared_ptr<Comparator<std::shared_ptr<TermStats>>> comparator =
      make_shared<DocFreqComparator>();

  for (int i = 1; i < args.size(); i++) {
    if (args[i] == L"-t") {
      comparator = make_shared<TotalTermFreqComparator>();
    } else {
      try {
        numTerms = stoi(args[i]);
      } catch (const NumberFormatException &e) {
        field = args[i];
      }
    }
  }

  shared_ptr<IndexReader> reader = DirectoryReader::open(dir);
  std::deque<std::shared_ptr<TermStats>> terms =
      getHighFreqTerms(reader, numTerms, field, comparator);

  for (int i = 0; i < terms.size(); i++) {
    printf(Locale::ROOT, L"%s:%s \t totalTF = %,d \t docFreq = %,d \n",
           terms[i]->field, terms[i]->termtext.utf8ToString(),
           terms[i]->totalTermFreq, terms[i]->docFreq);
  }
  delete reader;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") private static void usage()
void HighFreqTerms::usage()
{
  wcout << L"\n\n"
        << L"java org.apache.lucene.misc.HighFreqTerms <index dir> [-t] "
           L"[number_terms] [field]\n\t -t: order by totalTermFreq\n\n"
        << endl;
}

std::deque<std::shared_ptr<TermStats>> HighFreqTerms::getHighFreqTerms(
    shared_ptr<IndexReader> reader, int numTerms, const wstring &field,
    shared_ptr<Comparator<std::shared_ptr<TermStats>>>
        comparator) 
{
  shared_ptr<TermStatsQueue> tiq;

  if (field != L"") {
    shared_ptr<Terms> terms = MultiFields::getTerms(reader, field);
    if (terms == nullptr) {
      throw runtime_error(L"field " + field + L" not found");
    }

    shared_ptr<TermsEnum> termsEnum = terms->begin();
    tiq = make_shared<TermStatsQueue>(numTerms, comparator);
    tiq->fill(field, termsEnum);
  } else {
    shared_ptr<Fields> fields = MultiFields::getFields(reader);
    if (fields->empty()) {
      throw runtime_error(L"no fields found for this index");
    }
    tiq = make_shared<TermStatsQueue>(numTerms, comparator);
    for (auto fieldName : fields) {
      shared_ptr<Terms> terms = fields->terms(fieldName);
      if (terms != nullptr) {
        tiq->fill(fieldName, terms->begin());
      }
    }
  }

  std::deque<std::shared_ptr<TermStats>> result(tiq->size());
  // we want highest first so we read the queue and populate the array
  // starting at the end and work backwards
  int count = tiq->size() - 1;
  while (tiq->size() != 0) {
    result[count] = tiq->pop();
    count--;
  }
  return result;
}

int HighFreqTerms::DocFreqComparator::compare(shared_ptr<TermStats> a,
                                              shared_ptr<TermStats> b)
{
  int res = Long::compare(a->docFreq, b->docFreq);
  if (res == 0) {
    res = a->field.compare(b->field);
    if (res == 0) {
      res = a->termtext->compareTo(b->termtext);
    }
  }
  return res;
}

int HighFreqTerms::TotalTermFreqComparator::compare(shared_ptr<TermStats> a,
                                                    shared_ptr<TermStats> b)
{
  int res = Long::compare(a->totalTermFreq, b->totalTermFreq);
  if (res == 0) {
    res = a->field.compare(b->field);
    if (res == 0) {
      res = a->termtext->compareTo(b->termtext);
    }
  }
  return res;
}

HighFreqTerms::TermStatsQueue::TermStatsQueue(
    int size, shared_ptr<Comparator<std::shared_ptr<TermStats>>> comparator)
    : org::apache::lucene::util::PriorityQueue<TermStats>(size),
      comparator(comparator)
{
}

bool HighFreqTerms::TermStatsQueue::lessThan(shared_ptr<TermStats> termInfoA,
                                             shared_ptr<TermStats> termInfoB)
{
  return comparator->compare(termInfoA, termInfoB) < 0;
}

void HighFreqTerms::TermStatsQueue::fill(
    const wstring &field, shared_ptr<TermsEnum> termsEnum) 
{
  shared_ptr<BytesRef> term = nullptr;
  while ((term = termsEnum->next()) != nullptr) {
    insertWithOverflow(make_shared<TermStats>(field, term, termsEnum->docFreq(),
                                              termsEnum->totalTermFreq()));
  }
}
} // namespace org::apache::lucene::misc