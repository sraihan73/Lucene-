using namespace std;

#include "TestVersion.h"

namespace org::apache::lucene::util
{

void TestVersion::testOnOrAfter() 
{
  for (shared_ptr<Field> field : Version::typeid->getDeclaredFields()) {
    if (Modifier::isStatic(field->getModifiers()) &&
        field->getType() == Version::typeid) {
      shared_ptr<Version> v =
          std::static_pointer_cast<Version>(field->get(Version::typeid));
      assertTrue(L"LATEST must be always onOrAfter(" + v + L")",
                 Version::LATEST->onOrAfter(v));
    }
  }
  assertTrue(Version::LUCENE_7_0_0->onOrAfter(Version::LUCENE_6_0_0));
}

void TestVersion::testToString()
{
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"6.0.0", Version::LUCENE_6_0_0->toString());
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"7.0.0", Version::LUCENE_7_0_0->toString());
}

void TestVersion::testParseLeniently() 
{
  assertEquals(Version::LUCENE_6_0_0, Version::parseLeniently(L"6.0"));
  assertEquals(Version::LUCENE_6_0_0, Version::parseLeniently(L"6.0.0"));
  assertEquals(Version::LUCENE_6_0_0, Version::parseLeniently(L"LUCENE_60"));
  assertEquals(Version::LUCENE_6_0_0, Version::parseLeniently(L"LUCENE_6_0"));
  assertEquals(Version::LUCENE_6_0_0, Version::parseLeniently(L"LUCENE_6_0_0"));
  assertEquals(Version::LUCENE_7_0_0, Version::parseLeniently(L"7.0"));
  assertEquals(Version::LUCENE_7_0_0, Version::parseLeniently(L"7.0.0"));
  assertEquals(Version::LUCENE_7_0_0, Version::parseLeniently(L"LUCENE_70"));
  assertEquals(Version::LUCENE_7_0_0, Version::parseLeniently(L"LUCENE_7_0"));
  assertEquals(Version::LUCENE_7_0_0, Version::parseLeniently(L"LUCENE_7_0_0"));
  assertEquals(Version::LATEST, Version::parseLeniently(L"LATEST"));
  assertEquals(Version::LATEST, Version::parseLeniently(L"latest"));
  assertEquals(Version::LATEST, Version::parseLeniently(L"LUCENE_CURRENT"));
  assertEquals(Version::LATEST, Version::parseLeniently(L"lucene_current"));
}

void TestVersion::testParseLenientlyExceptions()
{
  shared_ptr<ParseException> expected = expectThrows(
      ParseException::typeid, [&]() { Version::parseLeniently(L"LUCENE"); });
  assertTrue(expected->getMessage()->contains(L"LUCENE"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parseLeniently(L"LUCENE_610"); });
  assertTrue(expected->getMessage()->contains(L"LUCENE_610"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parseLeniently(L"LUCENE61"); });
  assertTrue(expected->getMessage()->contains(L"LUCENE61"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parseLeniently(L"LUCENE_6.0.0"); });
  assertTrue(expected->getMessage()->contains(L"LUCENE_6.0.0"));
}

void TestVersion::testParseLenientlyOnAllConstants() 
{
  bool atLeastOne = false;
  for (shared_ptr<Field> field : Version::typeid->getDeclaredFields()) {
    if (Modifier::isStatic(field->getModifiers()) &&
        field->getType() == Version::typeid) {
      atLeastOne = true;
      shared_ptr<Version> v =
          std::static_pointer_cast<Version>(field->get(Version::typeid));
      // C++ TODO: There is no native C++ equivalent to 'toString':
      assertEquals(v, Version::parseLeniently(v->toString()));
      assertEquals(v, Version::parseLeniently(field->getName()));
      assertEquals(v, Version::parseLeniently(
                          field->getName()->toLowerCase(Locale::ROOT)));
    }
  }
  assertTrue(atLeastOne);
}

void TestVersion::testParse() 
{
  assertEquals(Version::LUCENE_6_0_0, Version::parse(L"6.0.0"));
  assertEquals(Version::LUCENE_7_0_0, Version::parse(L"7.0.0"));

  // Version does not pass judgement on the major version:
  assertEquals(1, Version::parse(L"1.0")->major);
  assertEquals(7, Version::parse(L"7.0.0")->major);
}

void TestVersion::testForwardsCompatibility() 
{
  assertTrue(Version::parse(L"6.10.20")->onOrAfter(Version::LUCENE_6_0_0));
}

void TestVersion::testParseExceptions()
{
  shared_ptr<ParseException> expected = expectThrows(
      ParseException::typeid, [&]() { Version::parse(L"LUCENE_6_0_0"); });
  assertTrue(expected->getMessage()->contains(L"LUCENE_6_0_0"));

  expected =
      expectThrows(ParseException::typeid, [&]() { Version::parse(L"6.256"); });
  assertTrue(expected->getMessage()->contains(L"6.256"));

  expected =
      expectThrows(ParseException::typeid, [&]() { Version::parse(L"6.-1"); });
  assertTrue(expected->getMessage()->contains(L"6.-1"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.1.256"); });
  assertTrue(expected->getMessage()->contains(L"6.1.256"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.1.-1"); });
  assertTrue(expected->getMessage()->contains(L"6.1.-1"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.1.1.3"); });
  assertTrue(expected->getMessage()->contains(L"6.1.1.3"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.1.1.-1"); });
  assertTrue(expected->getMessage()->contains(L"6.1.1.-1"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.1.1.1"); });
  assertTrue(expected->getMessage()->contains(L"6.1.1.1"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.1.1.2"); });
  assertTrue(expected->getMessage()->contains(L"6.1.1.2"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.0.0.0"); });
  assertTrue(expected->getMessage()->contains(L"6.0.0.0"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6.0.0.1.42"); });
  assertTrue(expected->getMessage()->contains(L"6.0.0.1.42"));

  expected = expectThrows(ParseException::typeid,
                          [&]() { Version::parse(L"6..0.1"); });
  assertTrue(expected->getMessage()->contains(L"6..0.1"));
}

void TestVersion::testDeprecations() 
{
  // all but the latest version should be deprecated
  bool atLeastOne = false;
  for (shared_ptr<Field> field : Version::typeid->getDeclaredFields()) {
    if (Modifier::isStatic(field->getModifiers()) &&
        field->getType() == Version::typeid) {
      atLeastOne = true;
      shared_ptr<Version> v =
          std::static_pointer_cast<Version>(field->get(Version::typeid));
      constexpr bool dep = field->isAnnotationPresent(Deprecated::typeid);
      if (v->equals(Version::LATEST) &&
          field->getName().equals(L"LUCENE_CURRENT") == false) {
        assertFalse(field->getName() + L" should not be deprecated", dep);
      } else {
        assertTrue(field->getName() + L" should be deprecated", dep);
      }
    }
  }
  assertTrue(atLeastOne);
}

void TestVersion::testLatestVersionCommonBuild()
{
  // common-build.xml sets 'tests.LUCENE_VERSION', if not, we skip this test!
  wstring commonBuildVersion = System::getProperty(L"tests.LUCENE_VERSION");
  assumeTrue(L"Null 'tests.LUCENE_VERSION' test property. You should run the "
             L"tests with the official Lucene build file",
             commonBuildVersion != L"");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"Version.LATEST does not match the one given in common-build.xml",
      Version::LATEST->toString(), commonBuildVersion);
}

void TestVersion::testEqualsHashCode() 
{
  shared_ptr<Random> random = TestVersion::random();
  wstring version = L"" + (4 + random->nextInt(1)) + L"." +
                    random->nextInt(10) + L"." + random->nextInt(10);
  shared_ptr<Version> v1 = Version::parseLeniently(version);
  shared_ptr<Version> v2 = Version::parseLeniently(version);
  assertEquals(v1->hashCode(), v2->hashCode());
  assertEquals(v1, v2);
  constexpr int iters = 10 + random->nextInt(20);
  for (int i = 0; i < iters; i++) {
    wstring v = L"" + (4 + random->nextInt(1)) + L"." + random->nextInt(10) +
                L"." + random->nextInt(10);
    if (v == version) {
      assertEquals(Version::parseLeniently(v)->hashCode(), v1->hashCode());
      assertEquals(Version::parseLeniently(v), v1);
    } else {
      assertFalse(Version::parseLeniently(v)->equals(v1));
    }
  }
}
} // namespace org::apache::lucene::util