using namespace std;

#include "WordDictionary.h"
#include "../AnalyzerProfile.h"
#include "../Utility.h"

namespace org::apache::lucene::analysis::cn::smart::hhmm
{
using AnalyzerProfile =
    org::apache::lucene::analysis::cn::smart::AnalyzerProfile;
using Utility = org::apache::lucene::analysis::cn::smart::Utility;

WordDictionary::WordDictionary() {}

shared_ptr<WordDictionary> WordDictionary::singleInstance;

// C++ WARNING: The following method was originally marked 'synchronized':
shared_ptr<WordDictionary> WordDictionary::getInstance()
{
  if (singleInstance == nullptr) {
    singleInstance = make_shared<WordDictionary>();
    try {
      singleInstance->load();
    } catch (const IOException &e) {
      wstring wordDictRoot = AnalyzerProfile::ANALYSIS_DATA_DIR;
      singleInstance->load(wordDictRoot);
    } catch (const ClassNotFoundException &e) {
      throw runtime_error(e);
    }
  }
  return singleInstance;
}

void WordDictionary::load(const wstring &dctFileRoot)
{
  wstring dctFilePath = dctFileRoot + L"/coredict.dct";
  shared_ptr<Path> serialObj = Paths->get(dctFileRoot + L"/coredict.mem");

  if (Files::exists(serialObj) && loadFromObj(serialObj)) {

  } else {
    try {
      wordIndexTable = std::deque<short>(PRIME_INDEX_LENGTH);
      charIndexTable = std::deque<wchar_t>(PRIME_INDEX_LENGTH);
      for (int i = 0; i < PRIME_INDEX_LENGTH; i++) {
        charIndexTable[i] = 0;
        wordIndexTable[i] = -1;
      }
      wordItem_charArrayTable =
          std::deque<std::deque<std::deque<wchar_t>>>(GB2312_CHAR_NUM);
      wordItem_frequencyTable = std::deque<std::deque<int>>(GB2312_CHAR_NUM);
      // int total =
      loadMainDataFromFile(dctFilePath);
      expandDelimiterData();
      mergeSameWords();
      sortEachItems();
      // log.info("load dictionary: " + dctFilePath + " total:" + total);
    } catch (const IOException &e) {
      throw runtime_error(e->getMessage());
    }

    saveToObj(serialObj);
  }
}

void WordDictionary::load() 
{
  shared_ptr<InputStream> input =
      this->getClass().getResourceAsStream(L"coredict.mem");
  loadFromObjectInputStream(input);
}

bool WordDictionary::loadFromObj(shared_ptr<Path> serialObj)
{
  try {
    loadFromObjectInputStream(Files::newInputStream(serialObj));
    return true;
  } catch (const runtime_error &e) {
    throw runtime_error(e);
  }
}

void WordDictionary::loadFromObjectInputStream(
    shared_ptr<InputStream>
        serialObjectInputStream) 
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.ObjectInputStream input = new
  // java.io.ObjectInputStream(serialObjectInputStream))
  {
    java::io::ObjectInputStream input =
        java::io::ObjectInputStream(serialObjectInputStream);
    wordIndexTable = static_cast<std::deque<short>>(input.readObject());
    charIndexTable = static_cast<std::deque<wchar_t>>(input.readObject());
    wordItem_charArrayTable =
        static_cast<std::deque<std::deque<std::deque<wchar_t>>>>(
            input.readObject());
    wordItem_frequencyTable =
        static_cast<std::deque<std::deque<int>>>(input.readObject());
    // log.info("load core dict from serialization.");
  }
}

void WordDictionary::saveToObj(shared_ptr<Path> serialObj)
{
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (java.io.ObjectOutputStream output = new
  // java.io.ObjectOutputStream(java.nio.file.Files.newOutputStream(serialObj)))
  {
    java::io::ObjectOutputStream output = java::io::ObjectOutputStream(
        java::nio::file::Files::newOutputStream(serialObj));
    try {
      output.writeObject(wordIndexTable);
      output.writeObject(charIndexTable);
      output.writeObject(wordItem_charArrayTable);
      output.writeObject(wordItem_frequencyTable);
      // log.info("serialize core dict.");
    } catch (const runtime_error &e) {
      // log.warn(e.getMessage());
    }
  }
}

int WordDictionary::loadMainDataFromFile(const wstring &dctFilePath) throw(
    IOException)
{
  int i, cnt, length, total = 0;
  // The file only counted 6763 Chinese characters plus 5 reserved slots
  // 3756~3760. The 3756th is used (as a header) to store information.
  std::deque<int> buffer(3);
  std::deque<char> intBuffer(4);
  wstring tmpword;
  shared_ptr<DataInputStream> dctFile = make_shared<DataInputStream>(
      Files::newInputStream(Paths->get(dctFilePath)));

  // GB2312 characters 0 - 6768
  for (i = GB2312_FIRST_CHAR; i < GB2312_FIRST_CHAR + CHAR_NUM_IN_FILE; i++) {
    // if (i == 5231)
    // System.out.println(i);

    dctFile->read(intBuffer);
    // the dictionary was developed for C, and byte order must be converted to
    // work with Java
    cnt = ByteBuffer::wrap(intBuffer).order(ByteOrder::LITTLE_ENDIAN).getInt();
    if (cnt <= 0) {
      wordItem_charArrayTable[i].smartpointerreset();
      wordItem_frequencyTable[i].smartpointerreset();
      continue;
    }
    wordItem_charArrayTable[i] = std::deque<std::deque<wchar_t>>(cnt);
    wordItem_frequencyTable[i] = std::deque<int>(cnt);
    total += cnt;
    int j = 0;
    while (j < cnt) {
      // wordItemTable[i][j] = new WordItem();
      dctFile->read(intBuffer);
      buffer[0] = ByteBuffer::wrap(intBuffer)
                      .order(ByteOrder::LITTLE_ENDIAN)
                      .getInt(); // frequency
      dctFile->read(intBuffer);
      buffer[1] = ByteBuffer::wrap(intBuffer)
                      .order(ByteOrder::LITTLE_ENDIAN)
                      .getInt(); // length
      dctFile->read(intBuffer);
      buffer[2] = ByteBuffer::wrap(intBuffer)
                      .order(ByteOrder::LITTLE_ENDIAN)
                      .getInt(); // handle

      // wordItemTable[i][j].frequency = buffer[0];
      wordItem_frequencyTable[i][j] = buffer[0];

      length = buffer[1];
      if (length > 0) {
        std::deque<char> lchBuffer(length);
        dctFile->read(lchBuffer);
        tmpword = wstring(lchBuffer, L"GB2312");
        // indexTable[i].wordItems[j].word = tmpword;
        // wordItemTable[i][j].charArray = tmpword.toCharArray();
        wordItem_charArrayTable[i][j] = tmpword.toCharArray();
      } else {
        // wordItemTable[i][j].charArray = null;
        wordItem_charArrayTable[i][j].smartpointerreset();
      }
      // System.out.println(indexTable[i].wordItems[j]);
      j++;
    }

    wstring str = getCCByGB2312Id(i);
    setTableIndex(str[0], i);
  }
  dctFile->close();
  return total;
}

void WordDictionary::expandDelimiterData()
{
  int i;
  int cnt;
  // Punctuation then treating index 3755 as 1,
  // distribute the original punctuation corresponding dictionary into
  int delimiterIndex = 3755 + GB2312_FIRST_CHAR;
  i = 0;
  while (i < wordItem_charArrayTable[delimiterIndex].length) {
    wchar_t c = wordItem_charArrayTable[delimiterIndex][i][0];
    int j = getGB2312Id(c); // the id value of the punctuation
    if (wordItem_charArrayTable[j] == nullptr) {

      int k = i;
      // Starting from i, count the number of the following worditem symbol from
      // j
      while (k < wordItem_charArrayTable[delimiterIndex].length &&
             wordItem_charArrayTable[delimiterIndex][k][0] == c) {
        k++;
      }
      // c is the punctuation character, j is the id value of c
      // k-1 represents the index of the last punctuation character
      cnt = k - i;
      if (cnt != 0) {
        wordItem_charArrayTable[j] = std::deque<std::deque<wchar_t>>(cnt);
        wordItem_frequencyTable[j] = std::deque<int>(cnt);
      }

      // Assign value for each wordItem.
      for (k = 0; k < cnt; k++, i++) {
        // wordItemTable[j][k] = new WordItem();
        wordItem_frequencyTable[j][k] =
            wordItem_frequencyTable[delimiterIndex][i];
        wordItem_charArrayTable[j][k] = std::deque<wchar_t>(
            wordItem_charArrayTable[delimiterIndex][i].length - 1);
        System::arraycopy(wordItem_charArrayTable[delimiterIndex][i], 1,
                          wordItem_charArrayTable[j][k], 0,
                          wordItem_charArrayTable[j][k].length);
      }
      setTableIndex(c, j);
    }
  }
  // Delete the original corresponding symbol array.
  wordItem_charArrayTable[delimiterIndex].smartpointerreset();
  wordItem_frequencyTable[delimiterIndex].smartpointerreset();
}

void WordDictionary::mergeSameWords()
{
  int i;
  for (i = 0; i < GB2312_FIRST_CHAR + CHAR_NUM_IN_FILE; i++) {
    if (wordItem_charArrayTable[i] == nullptr) {
      continue;
    }
    int len = 1;
    for (int j = 1; j < wordItem_charArrayTable[i].length; j++) {
      if (Utility::compareArray(wordItem_charArrayTable[i][j], 0,
                                wordItem_charArrayTable[i][j - 1], 0) != 0) {
        len++;
      }
    }
    if (len < wordItem_charArrayTable[i].length) {
      std::deque<std::deque<wchar_t>> tempArray(len);
      std::deque<int> tempFreq(len);
      int k = 0;
      tempArray[0] = wordItem_charArrayTable[i][0];
      tempFreq[0] = wordItem_frequencyTable[i][0];
      for (int j = 1; j < wordItem_charArrayTable[i].length; j++) {
        if (Utility::compareArray(wordItem_charArrayTable[i][j], 0,
                                  tempArray[k], 0) != 0) {
          k++;
          // temp[k] = wordItemTable[i][j];
          tempArray[k] = wordItem_charArrayTable[i][j];
          tempFreq[k] = wordItem_frequencyTable[i][j];
        } else {
          // temp[k].frequency += wordItemTable[i][j].frequency;
          tempFreq[k] += wordItem_frequencyTable[i][j];
        }
      }
      // wordItemTable[i] = temp;
      wordItem_charArrayTable[i] = tempArray;
      wordItem_frequencyTable[i] = tempFreq;
    }
  }
}

void WordDictionary::sortEachItems()
{
  std::deque<wchar_t> tmpArray;
  int tmpFreq;
  for (int i = 0; i < wordItem_charArrayTable.size(); i++) {
    if (wordItem_charArrayTable[i] != nullptr &&
        wordItem_charArrayTable[i].length > 1) {
      for (int j = 0; j < wordItem_charArrayTable[i].length - 1; j++) {
        for (int j2 = j + 1; j2 < wordItem_charArrayTable[i].length; j2++) {
          if (Utility::compareArray(wordItem_charArrayTable[i][j], 0,
                                    wordItem_charArrayTable[i][j2], 0) > 0) {
            tmpArray = wordItem_charArrayTable[i][j];
            tmpFreq = wordItem_frequencyTable[i][j];
            wordItem_charArrayTable[i][j] = wordItem_charArrayTable[i][j2];
            wordItem_frequencyTable[i][j] = wordItem_frequencyTable[i][j2];
            wordItem_charArrayTable[i][j2] = tmpArray;
            wordItem_frequencyTable[i][j2] = tmpFreq;
          }
        }
      }
    }
  }
}

bool WordDictionary::setTableIndex(wchar_t c, int j)
{
  int index = getAvaliableTableIndex(c);
  if (index != -1) {
    charIndexTable[index] = c;
    wordIndexTable[index] = static_cast<short>(j);
    return true;
  } else {
    return false;
  }
}

short WordDictionary::getAvaliableTableIndex(wchar_t c)
{
  int hash1 = static_cast<int>(this->hash1(c) % PRIME_INDEX_LENGTH);
  int hash2 = this->hash2(c) % PRIME_INDEX_LENGTH;
  if (hash1 < 0) {
    hash1 = PRIME_INDEX_LENGTH + hash1;
  }
  if (hash2 < 0) {
    hash2 = PRIME_INDEX_LENGTH + hash2;
  }
  int index = hash1;
  int i = 1;
  while (charIndexTable[index] != 0 && charIndexTable[index] != c &&
         i < PRIME_INDEX_LENGTH) {
    index = (hash1 + i * hash2) % PRIME_INDEX_LENGTH;
    i++;
  }
  // System.out.println(i - 1);

  if (i < PRIME_INDEX_LENGTH &&
      (charIndexTable[index] == 0 || charIndexTable[index] == c)) {
    return static_cast<short>(index);
  } else {
    return -1;
  }
}

short WordDictionary::getWordItemTableIndex(wchar_t c)
{
  int hash1 = static_cast<int>(this->hash1(c) % PRIME_INDEX_LENGTH);
  int hash2 = this->hash2(c) % PRIME_INDEX_LENGTH;
  if (hash1 < 0) {
    hash1 = PRIME_INDEX_LENGTH + hash1;
  }
  if (hash2 < 0) {
    hash2 = PRIME_INDEX_LENGTH + hash2;
  }
  int index = hash1;
  int i = 1;
  while (charIndexTable[index] != 0 && charIndexTable[index] != c &&
         i < PRIME_INDEX_LENGTH) {
    index = (hash1 + i * hash2) % PRIME_INDEX_LENGTH;
    i++;
  }

  if (i < PRIME_INDEX_LENGTH && charIndexTable[index] == c) {
    return static_cast<short>(index);
  } else {
    return -1;
  }
}

int WordDictionary::findInTable(short knownHashIndex,
                                std::deque<wchar_t> &charArray)
{
  if (charArray.empty() || charArray.empty()) {
    return -1;
  }

  std::deque<std::deque<wchar_t>> items =
      wordItem_charArrayTable[wordIndexTable[knownHashIndex]];
  int start = 0, end = items.size() - 1;
  int mid = (start + end) / 2, cmpResult;

  // Binary search for the index of idArray
  while (start <= end) {
    cmpResult = Utility::compareArray(items[mid], 0, charArray, 1);

    if (cmpResult == 0) {
      return mid; // find it
    } else if (cmpResult < 0) {
      start = mid + 1;
    } else if (cmpResult > 0) {
      end = mid - 1;
    }

    mid = (start + end) / 2;
  }
  return -1;
}

int WordDictionary::getPrefixMatch(std::deque<wchar_t> &charArray)
{
  return getPrefixMatch(charArray, 0);
}

int WordDictionary::getPrefixMatch(std::deque<wchar_t> &charArray,
                                   int knownStart)
{
  short index = getWordItemTableIndex(charArray[0]);
  if (index == -1) {
    return -1;
  }
  std::deque<std::deque<wchar_t>> items =
      wordItem_charArrayTable[wordIndexTable[index]];
  int start = knownStart, end = items.size() - 1;

  int mid = (start + end) / 2, cmpResult;

  // Binary search for the index of idArray
  while (start <= end) {
    cmpResult = Utility::compareArrayByPrefix(charArray, 1, items[mid], 0);
    if (cmpResult == 0) {
      // Get the first item which match the current word
      while (mid >= 0 &&
             Utility::compareArrayByPrefix(charArray, 1, items[mid], 0) == 0) {
        mid--;
      }
      mid++;
      return mid; // Find the first word that uses charArray as prefix.
    } else if (cmpResult < 0) {
      end = mid - 1;
    } else {
      start = mid + 1;
    }
    mid = (start + end) / 2;
  }
  return -1;
}

int WordDictionary::getFrequency(std::deque<wchar_t> &charArray)
{
  short hashIndex = getWordItemTableIndex(charArray[0]);
  if (hashIndex == -1) {
    return 0;
  }
  int itemIndex = findInTable(hashIndex, charArray);
  if (itemIndex != -1) {
    return wordItem_frequencyTable[wordIndexTable[hashIndex]][itemIndex];
  }
  return 0;
}

bool WordDictionary::isEqual(std::deque<wchar_t> &charArray, int itemIndex)
{
  short hashIndex = getWordItemTableIndex(charArray[0]);
  return Utility::compareArray(
             charArray, 1,
             wordItem_charArrayTable[wordIndexTable[hashIndex]][itemIndex],
             0) == 0;
}
} // namespace org::apache::lucene::analysis::cn::smart::hhmm