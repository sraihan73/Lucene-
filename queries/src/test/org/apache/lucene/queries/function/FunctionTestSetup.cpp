using namespace std;

#include "FunctionTestSetup.h"

namespace org::apache::lucene::queries::function
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using Document = org::apache::lucene::document::Document;
using Field = org::apache::lucene::document::Field;
using FieldType = org::apache::lucene::document::FieldType;
using NumericDocValuesField =
    org::apache::lucene::document::NumericDocValuesField;
using SortedDocValuesField =
    org::apache::lucene::document::SortedDocValuesField;
using SortedNumericDocValuesField =
    org::apache::lucene::document::SortedNumericDocValuesField;
using StoredField = org::apache::lucene::document::StoredField;
using TextField = org::apache::lucene::document::TextField;
using IndexWriterConfig = org::apache::lucene::index::IndexWriterConfig;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using FloatFieldSource =
    org::apache::lucene::queries::function::valuesource::FloatFieldSource;
using IntFieldSource =
    org::apache::lucene::queries::function::valuesource::IntFieldSource;
using MultiValuedFloatFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedFloatFieldSource;
using MultiValuedIntFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedIntFieldSource;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using Directory = org::apache::lucene::store::Directory;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using NumericUtils = org::apache::lucene::util::NumericUtils;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::AfterClass;
using org::junit::Ignore;
float FunctionTestSetup::TEST_SCORE_TOLERANCE_DELTA = 0.001f;
const wstring FunctionTestSetup::ID_FIELD = L"id";
const wstring FunctionTestSetup::TEXT_FIELD = L"text";
const wstring FunctionTestSetup::INT_FIELD = L"iii";
const wstring FunctionTestSetup::INT_FIELD_MV_MIN = L"iii_min";
const wstring FunctionTestSetup::INT_FIELD_MV_MAX = L"iii_max";
const wstring FunctionTestSetup::FLOAT_FIELD = L"fff";
const wstring FunctionTestSetup::FLOAT_FIELD_MV_MIN = L"fff_min";
const wstring FunctionTestSetup::FLOAT_FIELD_MV_MAX = L"fff_max";
std::deque<wstring> const FunctionTestSetup::DOC_TEXT_LINES = {
    L"Well, this is just some plain text we use for creating the ",
    L"test documents. It used to be a text from an online collection ",
    L"devoted to first aid, but if there was there an (online) lawyers ",
    L"first aid collection with legal advices, \"it\" might have quite ",
    L"probably advised one not to include \"it\"'s text or the text of ",
    L"any other online collection in one's code, unless one has money ",
    L"that one don't need and one is happy to donate for lawyers ",
    L"charity. Anyhow at some point, rechecking the usage of this text, ",
    L"it became uncertain that this text is free to use, because ",
    L"the web site in the disclaimer of he eBook containing that text ",
    L"was not responding anymore, and at the same time, in projGut, ",
    L"searching for first aid no longer found that eBook as well. ",
    L"So here we are, with a perhaps much less interesting ",
    L"text for the test, but oh much much safer. "};
shared_ptr<org::apache::lucene::store::Directory> FunctionTestSetup::dir;
shared_ptr<org::apache::lucene::analysis::Analyzer> FunctionTestSetup::anlzr;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClassFunctionTestSetup()
// throws Exception
void FunctionTestSetup::afterClassFunctionTestSetup() 
{
  delete dir;
  dir.reset();
  delete anlzr;
  anlzr.reset();
}

void FunctionTestSetup::createIndex(bool doMultiSegment) 
{
  if (VERBOSE) {
    wcout << L"TEST: setUp" << endl;
  }
  // prepare a small index with just a few documents.
  dir = newDirectory();
  anlzr = make_shared<MockAnalyzer>(random());
  shared_ptr<IndexWriterConfig> iwc =
      newIndexWriterConfig(anlzr)->setMergePolicy(newLogMergePolicy());
  if (doMultiSegment) {
    iwc->setMaxBufferedDocs(TestUtil::nextInt(random(), 2, 7));
  }
  shared_ptr<RandomIndexWriter> iw =
      make_shared<RandomIndexWriter>(random(), dir, iwc);
  // add docs not exactly in natural ID order, to verify we do check the order
  // of docs by scores
  int remaining = N_DOCS;
  std::deque<bool> done(N_DOCS);
  int i = 0;
  while (remaining > 0) {
    if (done[i]) {
      throw runtime_error(L"to set this test correctly N_DOCS=" +
                          to_wstring(N_DOCS) +
                          L" must be primary and greater than 2!");
    }
    addDoc(iw, i);
    done[i] = true;
    i = (i + 4) % N_DOCS;
    remaining--;
  }
  if (!doMultiSegment) {
    if (VERBOSE) {
      wcout << L"TEST: setUp full merge" << endl;
    }
    iw->forceMerge(1);
  }
  delete iw;
  if (VERBOSE) {
    wcout << L"TEST: setUp done close" << endl;
  }
}

void FunctionTestSetup::addDoc(shared_ptr<RandomIndexWriter> iw,
                               int i) 
{
  shared_ptr<Document> d = make_shared<Document>();
  shared_ptr<Field> f;
  int scoreAndID = i + 1;

  shared_ptr<FieldType> customType =
      make_shared<FieldType>(TextField::TYPE_STORED);
  customType->setTokenized(false);
  customType->setOmitNorms(true);

  f = newField(ID_FIELD, id2String(scoreAndID),
               customType); // for debug purposes
  d->push_back(f);
  d->push_back(make_shared<SortedDocValuesField>(
      ID_FIELD, make_shared<BytesRef>(id2String(scoreAndID))));

  shared_ptr<FieldType> customType2 =
      make_shared<FieldType>(TextField::TYPE_NOT_STORED);
  customType2->setOmitNorms(true);
  f = newField(TEXT_FIELD,
               L"text of doc" + to_wstring(scoreAndID) + textLine(i),
               customType2); // for regular search
  d->push_back(f);

  f = make_shared<StoredField>(INT_FIELD, scoreAndID); // for function scoring
  d->push_back(f);
  d->push_back(make_shared<NumericDocValuesField>(INT_FIELD, scoreAndID));

  f = make_shared<StoredField>(FLOAT_FIELD, scoreAndID); // for function scoring
  d->push_back(f);
  d->push_back(make_shared<NumericDocValuesField>(
      FLOAT_FIELD, Float::floatToRawIntBits(scoreAndID)));

  f = make_shared<StoredField>(INT_FIELD_MV_MIN, scoreAndID);
  d->push_back(f);
  f = make_shared<StoredField>(INT_FIELD_MV_MIN, scoreAndID + 1);
  d->push_back(f);
  d->push_back(
      make_shared<SortedNumericDocValuesField>(INT_FIELD_MV_MIN, scoreAndID));
  d->push_back(make_shared<SortedNumericDocValuesField>(INT_FIELD_MV_MIN,
                                                        scoreAndID + 1));

  f = make_shared<StoredField>(INT_FIELD_MV_MAX, scoreAndID);
  d->push_back(f);
  f = make_shared<StoredField>(INT_FIELD_MV_MAX, scoreAndID - 1);
  d->push_back(f);
  d->push_back(
      make_shared<SortedNumericDocValuesField>(INT_FIELD_MV_MAX, scoreAndID));
  d->push_back(make_shared<SortedNumericDocValuesField>(INT_FIELD_MV_MAX,
                                                        scoreAndID - 1));

  f = make_shared<StoredField>(FLOAT_FIELD_MV_MIN, scoreAndID);
  d->push_back(f);
  f = make_shared<StoredField>(FLOAT_FIELD_MV_MIN, scoreAndID + 1);
  d->push_back(f);
  d->push_back(make_shared<SortedNumericDocValuesField>(
      FLOAT_FIELD_MV_MIN, NumericUtils::floatToSortableInt(scoreAndID)));
  d->push_back(make_shared<SortedNumericDocValuesField>(
      FLOAT_FIELD_MV_MIN, NumericUtils::floatToSortableInt(scoreAndID + 1)));

  f = make_shared<StoredField>(FLOAT_FIELD_MV_MAX, scoreAndID);
  d->push_back(f);
  f = make_shared<StoredField>(FLOAT_FIELD_MV_MAX, scoreAndID - 1);
  d->push_back(f);
  d->push_back(make_shared<SortedNumericDocValuesField>(
      FLOAT_FIELD_MV_MAX, NumericUtils::floatToSortableInt(scoreAndID)));
  d->push_back(make_shared<SortedNumericDocValuesField>(
      FLOAT_FIELD_MV_MAX, NumericUtils::floatToSortableInt(scoreAndID - 1)));

  iw->addDocument(d);
  log(L"added: " + d);
}

wstring FunctionTestSetup::id2String(int scoreAndID)
{
  wstring s = L"000000000" + to_wstring(scoreAndID);
  int n = (L"" + to_wstring(N_DOCS))->length() + 3;
  int k = s.length() - n;
  return L"ID" + s.substr(k);
}

wstring FunctionTestSetup::textLine(int docNum)
{
  return DOC_TEXT_LINES[docNum % DOC_TEXT_LINES.size()];
}

float FunctionTestSetup::expectedFieldScore(const wstring &docIDFieldVal)
{
  return stof(docIDFieldVal.substr(2));
}

void FunctionTestSetup::log(any o)
{
  if (VERBOSE) {
    // C++ TODO: There is no native C++ equivalent to 'toString':
    wcout << o.toString() << endl;
  }
}
} // namespace org::apache::lucene::queries::function