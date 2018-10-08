using namespace std;

#include "FileDictionaryTest.h"

namespace org::apache::lucene::search::suggest
{
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

unordered_map::Entry<deque<wstring>, wstring>
FileDictionaryTest::generateFileEntry(const wstring &fieldDelimiter,
                                      bool hasWeight, bool hasPayload)
{
  deque<wstring> entryValues = deque<wstring>();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  wstring term = TestUtil::randomSimpleString(random(), 1, 300);
  sb->append(term);
  entryValues.push_back(term);
  if (hasWeight) {
    sb->append(fieldDelimiter);
    int64_t weight =
        TestUtil::nextLong(random(), numeric_limits<int64_t>::min(),
                           numeric_limits<int64_t>::max());
    sb->append(weight);
    entryValues.push_back(to_wstring(weight));
  }
  if (hasPayload) {
    sb->append(fieldDelimiter);
    wstring payload = TestUtil::randomSimpleString(random(), 1, 300);
    sb->append(payload);
    entryValues.push_back(payload);
  }
  sb->append(L"\n");
  return make_shared<SimpleEntry<>>(entryValues, sb->toString());
}

unordered_map::Entry<deque<deque<wstring>>, wstring>
FileDictionaryTest::generateFileInput(int count, const wstring &fieldDelimiter,
                                      bool hasWeights, bool hasPayloads)
{
  deque<deque<wstring>> entries = deque<deque<wstring>>();
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  bool hasPayload = hasPayloads;
  for (int i = 0; i < count; i++) {
    if (hasPayloads) {
      hasPayload = (i == 0) ? true : random()->nextBoolean();
    }
    unordered_map::Entry<deque<wstring>, wstring> entrySet = generateFileEntry(
        fieldDelimiter,
        (!hasPayloads && hasWeights) ? random()->nextBoolean() : hasWeights,
        hasPayload);
    entries.push_back(entrySet.getKey());
    sb->append(entrySet.getValue());
  }
  return make_shared<SimpleEntry<>>(entries, sb->toString());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFileWithTerm() throws
// java.io.IOException
void FileDictionaryTest::testFileWithTerm() 
{
  unordered_map::Entry<deque<deque<wstring>>, wstring> fileInput =
      generateFileInput(atLeast(100), FileDictionary::DEFAULT_FIELD_DELIMITER,
                        false, false);
  shared_ptr<InputStream> inputReader = make_shared<ByteArrayInputStream>(
      fileInput.getValue().getBytes(StandardCharsets::UTF_8));
  shared_ptr<FileDictionary> dictionary =
      make_shared<FileDictionary>(inputReader);
  deque<deque<wstring>> entries = fileInput.getKey();
  shared_ptr<InputIterator> inputIter = dictionary->getEntryIterator();
  assertFalse(inputIter->hasPayloads());
  shared_ptr<BytesRef> term;
  int count = 0;
  while ((term = inputIter->next()) != nullptr) {
    assertTrue(entries.size() > count);
    deque<wstring> entry = entries[count];
    assertTrue(entry.size() >= 1); // at least a term
    TestUtil::assertEquals(entry[0], term->utf8ToString());
    TestUtil::assertEquals(1, inputIter->weight());
    assertNull(inputIter->payload());
    count++;
  }
  TestUtil::assertEquals(count, entries.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFileWithWeight() throws
// java.io.IOException
void FileDictionaryTest::testFileWithWeight() 
{
  unordered_map::Entry<deque<deque<wstring>>, wstring> fileInput =
      generateFileInput(atLeast(100), FileDictionary::DEFAULT_FIELD_DELIMITER,
                        true, false);
  shared_ptr<InputStream> inputReader = make_shared<ByteArrayInputStream>(
      fileInput.getValue().getBytes(StandardCharsets::UTF_8));
  shared_ptr<FileDictionary> dictionary =
      make_shared<FileDictionary>(inputReader);
  deque<deque<wstring>> entries = fileInput.getKey();
  shared_ptr<InputIterator> inputIter = dictionary->getEntryIterator();
  assertFalse(inputIter->hasPayloads());
  shared_ptr<BytesRef> term;
  int count = 0;
  while ((term = inputIter->next()) != nullptr) {
    assertTrue(entries.size() > count);
    deque<wstring> entry = entries[count];
    assertTrue(entry.size() >= 1); // at least a term
    TestUtil::assertEquals(entry[0], term->utf8ToString());
    TestUtil::assertEquals(
        (entry.size() == 2) ? StringHelper::fromString<int64_t>(entry[1]) : 1,
        inputIter->weight());
    assertNull(inputIter->payload());
    count++;
  }
  TestUtil::assertEquals(count, entries.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFileWithWeightAndPayload() throws
// java.io.IOException
void FileDictionaryTest::testFileWithWeightAndPayload() 
{
  unordered_map::Entry<deque<deque<wstring>>, wstring> fileInput =
      generateFileInput(atLeast(100), FileDictionary::DEFAULT_FIELD_DELIMITER,
                        true, true);
  shared_ptr<InputStream> inputReader = make_shared<ByteArrayInputStream>(
      fileInput.getValue().getBytes(StandardCharsets::UTF_8));
  shared_ptr<FileDictionary> dictionary =
      make_shared<FileDictionary>(inputReader);
  deque<deque<wstring>> entries = fileInput.getKey();
  shared_ptr<InputIterator> inputIter = dictionary->getEntryIterator();
  assertTrue(inputIter->hasPayloads());
  shared_ptr<BytesRef> term;
  int count = 0;
  while ((term = inputIter->next()) != nullptr) {
    assertTrue(entries.size() > count);
    deque<wstring> entry = entries[count];
    assertTrue(entry.size() >= 2); // at least term and weight
    TestUtil::assertEquals(entry[0], term->utf8ToString());
    TestUtil::assertEquals(StringHelper::fromString<int64_t>(entry[1]),
                           inputIter->weight());
    if (entry.size() == 3) {
      TestUtil::assertEquals(entry[2], inputIter->payload()->utf8ToString());
    } else {
      TestUtil::assertEquals(inputIter->payload()->length, 0);
    }
    count++;
  }
  TestUtil::assertEquals(count, entries.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFileWithOneEntry() throws
// java.io.IOException
void FileDictionaryTest::testFileWithOneEntry() 
{
  unordered_map::Entry<deque<deque<wstring>>, wstring> fileInput =
      generateFileInput(1, FileDictionary::DEFAULT_FIELD_DELIMITER, true, true);
  shared_ptr<InputStream> inputReader = make_shared<ByteArrayInputStream>(
      fileInput.getValue().getBytes(StandardCharsets::UTF_8));
  shared_ptr<FileDictionary> dictionary =
      make_shared<FileDictionary>(inputReader);
  deque<deque<wstring>> entries = fileInput.getKey();
  shared_ptr<InputIterator> inputIter = dictionary->getEntryIterator();
  assertTrue(inputIter->hasPayloads());
  shared_ptr<BytesRef> term;
  int count = 0;
  while ((term = inputIter->next()) != nullptr) {
    assertTrue(entries.size() > count);
    deque<wstring> entry = entries[count];
    assertTrue(entry.size() >= 2); // at least term and weight
    TestUtil::assertEquals(entry[0], term->utf8ToString());
    TestUtil::assertEquals(StringHelper::fromString<int64_t>(entry[1]),
                           inputIter->weight());
    if (entry.size() == 3) {
      TestUtil::assertEquals(entry[2], inputIter->payload()->utf8ToString());
    } else {
      TestUtil::assertEquals(inputIter->payload()->length, 0);
    }
    count++;
  }
  TestUtil::assertEquals(count, entries.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testFileWithDifferentDelimiter() throws
// java.io.IOException
void FileDictionaryTest::testFileWithDifferentDelimiter() 
{
  unordered_map::Entry<deque<deque<wstring>>, wstring> fileInput =
      generateFileInput(atLeast(100), L" , ", true, true);
  shared_ptr<InputStream> inputReader = make_shared<ByteArrayInputStream>(
      fileInput.getValue().getBytes(StandardCharsets::UTF_8));
  shared_ptr<FileDictionary> dictionary =
      make_shared<FileDictionary>(inputReader, L" , ");
  deque<deque<wstring>> entries = fileInput.getKey();
  shared_ptr<InputIterator> inputIter = dictionary->getEntryIterator();
  assertTrue(inputIter->hasPayloads());
  shared_ptr<BytesRef> term;
  int count = 0;
  while ((term = inputIter->next()) != nullptr) {
    assertTrue(entries.size() > count);
    deque<wstring> entry = entries[count];
    assertTrue(entry.size() >= 2); // at least term and weight
    TestUtil::assertEquals(entry[0], term->utf8ToString());
    TestUtil::assertEquals(StringHelper::fromString<int64_t>(entry[1]),
                           inputIter->weight());
    if (entry.size() == 3) {
      TestUtil::assertEquals(entry[2], inputIter->payload()->utf8ToString());
    } else {
      TestUtil::assertEquals(inputIter->payload()->length, 0);
    }
    count++;
  }
  TestUtil::assertEquals(count, entries.size());
}
} // namespace org::apache::lucene::search::suggest