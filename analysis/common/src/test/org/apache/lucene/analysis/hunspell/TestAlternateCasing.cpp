using namespace std;

#include "TestAlternateCasing.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestAlternateCasing::beforeClass() 
{
  init(L"alternate-casing.aff", L"alternate-casing.dic");
}

void TestAlternateCasing::testPossibilities()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"DRİNK", {L"drink"});
  assertStemsTo(L"DRINK");
  assertStemsTo(L"drinki", {L"drink"});
  assertStemsTo(L"DRİNKİ", {L"drink"});
  assertStemsTo(L"DRİNKI");
  assertStemsTo(L"DRINKI");
  assertStemsTo(L"DRINKİ");
  assertStemsTo(L"idrink", {L"drink"});
  assertStemsTo(L"İDRİNK", {L"drink"});
  assertStemsTo(L"IDRİNK");
  assertStemsTo(L"IDRINK");
  assertStemsTo(L"İDRINK");
  assertStemsTo(L"idrinki", {L"drink"});
  assertStemsTo(L"İDRİNKİ", {L"drink"});
  assertStemsTo(L"rıver", {L"rıver"});
  assertStemsTo(L"RIVER", {L"rıver"});
  assertStemsTo(L"RİVER");
  assertStemsTo(L"rıverı", {L"rıver"});
  assertStemsTo(L"RIVERI", {L"rıver"});
  assertStemsTo(L"RİVERI");
  assertStemsTo(L"RİVERİ");
  assertStemsTo(L"RIVERİ");
  assertStemsTo(L"ırıver", {L"rıver"});
  assertStemsTo(L"IRIVER", {L"rıver"});
  assertStemsTo(L"IRİVER");
  assertStemsTo(L"İRİVER");
  assertStemsTo(L"İRIVER");
  assertStemsTo(L"ırıverı", {L"rıver"});
  assertStemsTo(L"IRIVERI", {L"rıver"});
  assertStemsTo(L"Irıverı", {L"rıver"});
}
} // namespace org::apache::lucene::analysis::hunspell