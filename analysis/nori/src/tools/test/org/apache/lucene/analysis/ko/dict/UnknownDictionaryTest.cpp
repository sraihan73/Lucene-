using namespace std;

#include "UnknownDictionaryTest.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ko/util/CSVUtil.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/util/UnknownDictionaryWriter.h"

namespace org::apache::lucene::analysis::ko::dict
{
using CSVUtil = org::apache::lucene::analysis::ko::util::CSVUtil;
using UnknownDictionaryWriter =
    org::apache::lucene::analysis::ko::util::UnknownDictionaryWriter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPutCharacterCategory()
void UnknownDictionaryTest::testPutCharacterCategory()
{
  shared_ptr<UnknownDictionaryWriter> unkDic =
      make_shared<UnknownDictionaryWriter>(10 * 1024 * 1024);

  try {
    unkDic->putCharacterCategory(0, L"DUMMY_NAME");
    fail();
  } catch (const runtime_error &e) {
  }

  try {
    unkDic->putCharacterCategory(-1, L"HANGUL");
    fail();
  } catch (const runtime_error &e) {
  }

  unkDic->putCharacterCategory(0, L"DEFAULT");
  unkDic->putCharacterCategory(1, L"GREEK");
  unkDic->putCharacterCategory(2, L"HANJA");
  unkDic->putCharacterCategory(3, L"HANGUL");
  unkDic->putCharacterCategory(4, L"KANJI");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPut()
void UnknownDictionaryTest::testPut()
{
  shared_ptr<UnknownDictionaryWriter> unkDic =
      make_shared<UnknownDictionaryWriter>(10 * 1024 * 1024);
  try {
    unkDic->put(CSVUtil::parse(L"HANGUL,1800,3562,UNKNOWN,*,*,*,*,*,*,*"));
    fail();
  } catch (const runtime_error &e) {
  }

  wstring entry1 = L"ALPHA,1793,3533,795,SL,*,*,*,*,*,*,*";
  wstring entry2 = L"HANGUL,1800,3562,10247,UNKNOWN,*,*,*,*,*,*,*";
  wstring entry3 = L"HANJA,1792,3554,-821,SH,*,*,*,*,*,*,*";

  unkDic->putCharacterCategory(0, L"ALPHA");
  unkDic->putCharacterCategory(1, L"HANGUL");
  unkDic->putCharacterCategory(2, L"HANJA");

  unkDic->put(CSVUtil::parse(entry1));
  unkDic->put(CSVUtil::parse(entry2));
  unkDic->put(CSVUtil::parse(entry3));
}
} // namespace org::apache::lucene::analysis::ko::dict