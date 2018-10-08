using namespace std;

#include "UnknownDictionaryTest.h"
#include "../../../../../../../../java/org/apache/lucene/analysis/ja/util/CSVUtil.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ja/util/UnknownDictionaryWriter.h"

namespace org::apache::lucene::analysis::ja::dict
{
using CSVUtil = org::apache::lucene::analysis::ja::util::CSVUtil;
using UnknownDictionaryWriter =
    org::apache::lucene::analysis::ja::util::UnknownDictionaryWriter;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;
const wstring UnknownDictionaryTest::FILENAME = L"unk-tokeninfo-dict.obj";

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
    unkDic->putCharacterCategory(-1, L"KATAKANA");
    fail();
  } catch (const runtime_error &e) {
  }

  unkDic->putCharacterCategory(0, L"DEFAULT");
  unkDic->putCharacterCategory(1, L"GREEK");
  unkDic->putCharacterCategory(2, L"HIRAGANA");
  unkDic->putCharacterCategory(3, L"KATAKANA");
  unkDic->putCharacterCategory(4, L"KANJI");
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPut()
void UnknownDictionaryTest::testPut()
{
  shared_ptr<UnknownDictionaryWriter> unkDic =
      make_shared<UnknownDictionaryWriter>(10 * 1024 * 1024);
  try {
    unkDic->put(CSVUtil::parse(L"KANJI,1285,11426,名詞,一般,*,*,*,*,*,*,*"));
    fail();
  } catch (const runtime_error &e) {
  }

  wstring entry1 = L"ALPHA,1285,1285,13398,名詞,一般,*,*,*,*,*,*,*";
  wstring entry2 = L"HIRAGANA,1285,1285,13069,名詞,一般,*,*,*,*,*,*,*";
  wstring entry3 = L"KANJI,1285,1285,11426,名詞,一般,*,*,*,*,*,*,*";

  unkDic->putCharacterCategory(0, L"ALPHA");
  unkDic->putCharacterCategory(1, L"HIRAGANA");
  unkDic->putCharacterCategory(2, L"KANJI");

  unkDic->put(CSVUtil::parse(entry1));
  unkDic->put(CSVUtil::parse(entry2));
  unkDic->put(CSVUtil::parse(entry3));
}
} // namespace org::apache::lucene::analysis::ja::dict