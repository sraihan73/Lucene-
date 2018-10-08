using namespace std;

#include "TestIndonesianStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/id/IndonesianStemFilter.h"

namespace org::apache::lucene::analysis::id
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestIndonesianStemmer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  /* full stemming, no stopwords */
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  /* inflectional-only stemming */
  b = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
}

TestIndonesianStemmer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestIndonesianStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndonesianStemmer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<IndonesianStemFilter>(tokenizer));
}

TestIndonesianStemmer::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestIndonesianStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndonesianStemmer::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<IndonesianStemFilter>(tokenizer, false));
}

void TestIndonesianStemmer::tearDown() 
{
  IOUtils::close({a, b});
  BaseTokenStreamTestCase::tearDown();
}

void TestIndonesianStemmer::testExamples() 
{
  checkOneTerm(a, L"bukukah", L"buku");
  checkOneTerm(a, L"adalah", L"ada");
  checkOneTerm(a, L"bukupun", L"buku");
  checkOneTerm(a, L"bukuku", L"buku");
  checkOneTerm(a, L"bukumu", L"buku");
  checkOneTerm(a, L"bukunya", L"buku");
  checkOneTerm(a, L"mengukur", L"ukur");
  checkOneTerm(a, L"menyapu", L"sapu");
  checkOneTerm(a, L"menduga", L"duga");
  checkOneTerm(a, L"menuduh", L"uduh");
  checkOneTerm(a, L"membaca", L"baca");
  checkOneTerm(a, L"merusak", L"rusak");
  checkOneTerm(a, L"pengukur", L"ukur");
  checkOneTerm(a, L"penyapu", L"sapu");
  checkOneTerm(a, L"penduga", L"duga");
  checkOneTerm(a, L"pembaca", L"baca");
  checkOneTerm(a, L"diukur", L"ukur");
  checkOneTerm(a, L"tersapu", L"sapu");
  checkOneTerm(a, L"kekasih", L"kasih");
  checkOneTerm(a, L"berlari", L"lari");
  checkOneTerm(a, L"belajar", L"ajar");
  checkOneTerm(a, L"bekerja", L"kerja");
  checkOneTerm(a, L"perjelas", L"jelas");
  checkOneTerm(a, L"pelajar", L"ajar");
  checkOneTerm(a, L"pekerja", L"kerja");
  checkOneTerm(a, L"tarikkan", L"tarik");
  checkOneTerm(a, L"ambilkan", L"ambil");
  checkOneTerm(a, L"mengambilkan", L"ambil");
  checkOneTerm(a, L"makanan", L"makan");
  checkOneTerm(a, L"janjian", L"janji");
  checkOneTerm(a, L"perjanjian", L"janji");
  checkOneTerm(a, L"tandai", L"tanda");
  checkOneTerm(a, L"dapati", L"dapat");
  checkOneTerm(a, L"mendapati", L"dapat");
  checkOneTerm(a, L"pantai", L"panta");
}

void TestIndonesianStemmer::testIRExamples() 
{
  checkOneTerm(a, L"penyalahgunaan", L"salahguna");
  checkOneTerm(a, L"menyalahgunakan", L"salahguna");
  checkOneTerm(a, L"disalahgunakan", L"salahguna");

  checkOneTerm(a, L"pertanggungjawaban", L"tanggungjawab");
  checkOneTerm(a, L"mempertanggungjawabkan", L"tanggungjawab");
  checkOneTerm(a, L"dipertanggungjawabkan", L"tanggungjawab");

  checkOneTerm(a, L"pelaksanaan", L"laksana");
  checkOneTerm(a, L"pelaksana", L"laksana");
  checkOneTerm(a, L"melaksanakan", L"laksana");
  checkOneTerm(a, L"dilaksanakan", L"laksana");

  checkOneTerm(a, L"melibatkan", L"libat");
  checkOneTerm(a, L"terlibat", L"libat");

  checkOneTerm(a, L"penculikan", L"culik");
  checkOneTerm(a, L"menculik", L"culik");
  checkOneTerm(a, L"diculik", L"culik");
  checkOneTerm(a, L"penculik", L"culik");

  checkOneTerm(a, L"perubahan", L"ubah");
  checkOneTerm(a, L"peledakan", L"ledak");
  checkOneTerm(a, L"penanganan", L"tangan");
  checkOneTerm(a, L"kepolisian", L"polisi");
  checkOneTerm(a, L"kenaikan", L"naik");
  checkOneTerm(a, L"bersenjata", L"senjata");
  checkOneTerm(a, L"penyelewengan", L"seleweng");
  checkOneTerm(a, L"kecelakaan", L"celaka");
}

void TestIndonesianStemmer::testInflectionalOnly() 
{
  checkOneTerm(b, L"bukunya", L"buku");
  checkOneTerm(b, L"bukukah", L"buku");
  checkOneTerm(b, L"bukunyakah", L"buku");
  checkOneTerm(b, L"dibukukannya", L"dibukukan");
}

void TestIndonesianStemmer::testShouldntStem() 
{
  checkOneTerm(a, L"bersenjata", L"senjata");
  checkOneTerm(a, L"bukukah", L"buku");
  checkOneTerm(a, L"gigi", L"gigi");
}

void TestIndonesianStemmer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestIndonesianStemmer::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestIndonesianStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndonesianStemmer::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<IndonesianStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::id