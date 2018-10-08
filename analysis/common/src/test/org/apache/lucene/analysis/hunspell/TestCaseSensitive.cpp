using namespace std;

#include "TestCaseSensitive.h"

namespace org::apache::lucene::analysis::hunspell
{
using org::junit::BeforeClass;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestCaseSensitive::beforeClass() 
{
  init(L"casesensitive.aff", L"casesensitive.dic");
}

void TestCaseSensitive::testAllPossibilities()
{
  assertStemsTo(L"drink", {L"drink"});
  assertStemsTo(L"drinks", {L"drink"});
  assertStemsTo(L"drinkS", {L"drink"});
  assertStemsTo(L"gooddrinks", {L"drink"});
  assertStemsTo(L"Gooddrinks", {L"drink", L"drink"});
  assertStemsTo(L"GOODdrinks", {L"drink"});
  assertStemsTo(L"gooddrinkS", {L"drink"});
  assertStemsTo(L"GooddrinkS", {L"drink"});
  assertStemsTo(L"gooddrink", {L"drink"});
  assertStemsTo(L"Gooddrink", {L"drink", L"drink"});
  assertStemsTo(L"GOODdrink", {L"drink"});
  assertStemsTo(L"Drink", {L"drink", L"Drink"});
  assertStemsTo(L"Drinks", {L"drink", L"Drink"});
  assertStemsTo(L"DrinkS", {L"Drink"});
  assertStemsTo(L"goodDrinks", {L"Drink"});
  assertStemsTo(L"GoodDrinks", {L"Drink"});
  assertStemsTo(L"GOODDrinks", {L"Drink"});
  assertStemsTo(L"goodDrinkS", {L"Drink"});
  assertStemsTo(L"GoodDrinkS", {L"Drink"});
  assertStemsTo(L"GOODDrinkS", {L"Drink"});
  assertStemsTo(L"goodDrink", {L"Drink"});
  assertStemsTo(L"GoodDrink", {L"Drink"});
  assertStemsTo(L"GOODDrink", {L"Drink"});
  assertStemsTo(L"DRINK", {L"DRINK", L"drink", L"Drink"});
  assertStemsTo(L"DRINKs", {L"DRINK"});
  assertStemsTo(L"DRINKS", {L"DRINK", L"drink", L"Drink"});
  assertStemsTo(L"goodDRINKs", {L"DRINK"});
  assertStemsTo(L"GoodDRINKs", {L"DRINK"});
  assertStemsTo(L"GOODDRINKs", {L"DRINK"});
  assertStemsTo(L"goodDRINKS", {L"DRINK"});
  assertStemsTo(L"GoodDRINKS", {L"DRINK"});
  assertStemsTo(L"GOODDRINKS", {L"DRINK", L"drink", L"drink"});
  assertStemsTo(L"goodDRINK", {L"DRINK"});
  assertStemsTo(L"GoodDRINK", {L"DRINK"});
  assertStemsTo(L"GOODDRINK", {L"DRINK", L"drink", L"drink"});
}
} // namespace org::apache::lucene::analysis::hunspell