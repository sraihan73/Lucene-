using namespace std;

#include "GermanStemmer.h"

namespace org::apache::lucene::analysis::de
{

const shared_ptr<java::util::Locale> GermanStemmer::locale =
    make_shared<java::util::Locale>(L"de", L"DE");

wstring GermanStemmer::stem(const wstring &term)
{
  // Use lowercase for medium stemming.
  term = term.toLowerCase(locale);
  if (!isStemmable(term)) {
    return term;
  }
  // Reset the StringBuilder.
  sb->remove(0, sb->length());
  sb->insert(0, term);
  // Stemming starts here...
  substitute(sb);
  strip(sb);
  optimize(sb);
  resubstitute(sb);
  removeParticleDenotion(sb);
  return sb->toString();
}

bool GermanStemmer::isStemmable(const wstring &term)
{
  for (int c = 0; c < term.length(); c++) {
    if (!isalpha(term[c])) {
      return false;
    }
  }
  return true;
}

void GermanStemmer::strip(shared_ptr<StringBuilder> buffer)
{
  bool doMore = true;
  while (doMore && buffer->length() > 3) {
    if ((buffer->length() + substCount > 5) &&
        buffer
            ->substr(buffer->length() - 2,
                     buffer->length() - (buffer->length() - 2))
            ->equals(L"nd")) {
      buffer->remove(buffer->length() - 2, buffer->length());
    } else if ((buffer->length() + substCount > 4) &&
               buffer
                   ->substr(buffer->length() - 2,
                            buffer->length() - (buffer->length() - 2))
                   ->equals(L"em")) {
      buffer->remove(buffer->length() - 2, buffer->length());
    } else if ((buffer->length() + substCount > 4) &&
               buffer
                   ->substr(buffer->length() - 2,
                            buffer->length() - (buffer->length() - 2))
                   ->equals(L"er")) {
      buffer->remove(buffer->length() - 2, buffer->length());
    } else if (buffer->charAt(buffer->length() - 1) == L'e') {
      buffer->deleteCharAt(buffer->length() - 1);
    } else if (buffer->charAt(buffer->length() - 1) == L's') {
      buffer->deleteCharAt(buffer->length() - 1);
    } else if (buffer->charAt(buffer->length() - 1) == L'n') {
      buffer->deleteCharAt(buffer->length() - 1);
    }
    // "t" occurs only as suffix of verbs.
    else if (buffer->charAt(buffer->length() - 1) == L't') {
      buffer->deleteCharAt(buffer->length() - 1);
    } else {
      doMore = false;
    }
  }
}

void GermanStemmer::optimize(shared_ptr<StringBuilder> buffer)
{
  // Additional step for female plurals of professions and inhabitants.
  if (buffer->length() > 5 &&
      buffer
          ->substr(buffer->length() - 5,
                   buffer->length() - (buffer->length() - 5))
          ->equals(L"erin*")) {
    buffer->deleteCharAt(buffer->length() - 1);
    strip(buffer);
  }
  // Additional step for irregular plural nouns like "Matrizen -> Matrix".
  // NOTE: this length constraint is probably not a great value, it's just to
  // prevent AIOOBE on empty terms
  if (buffer->length() > 0 && buffer->charAt(buffer->length() - 1) == (L'z')) {
    buffer->setCharAt(buffer->length() - 1, L'x');
  }
}

void GermanStemmer::removeParticleDenotion(shared_ptr<StringBuilder> buffer)
{
  if (buffer->length() > 4) {
    for (int c = 0; c < buffer->length() - 3; c++) {
      if (buffer->substr(c, 4)->equals(L"gege")) {
        buffer->remove(c, c + 2);
        return;
      }
    }
  }
}

void GermanStemmer::substitute(shared_ptr<StringBuilder> buffer)
{
  substCount = 0;
  for (int c = 0; c < buffer->length(); c++) {
    // Replace the second char of a pair of the equal characters with an
    // asterisk
    if (c > 0 && buffer->charAt(c) == buffer->charAt(c - 1)) {
      buffer->setCharAt(c, L'*');
    }
    // Substitute Umlauts.
    else if (buffer->charAt(c) == L'ä') {
      buffer->setCharAt(c, L'a');
    } else if (buffer->charAt(c) == L'ö') {
      buffer->setCharAt(c, L'o');
    } else if (buffer->charAt(c) == L'ü') {
      buffer->setCharAt(c, L'u');
    }
    // Fix bug so that 'ß' at the end of a word is replaced.
    else if (buffer->charAt(c) == L'ß') {
      buffer->setCharAt(c, L's');
      buffer->insert(c + 1, L's');
      substCount++;
    }
    // Take care that at least one character is left left side from the current
    // one
    if (c < buffer->length() - 1) {
      // Masking several common character combinations with an token
      if ((c < buffer->length() - 2) && buffer->charAt(c) == L's' &&
          buffer->charAt(c + 1) == L'c' && buffer->charAt(c + 2) == L'h') {
        buffer->setCharAt(c, L'$');
        buffer->remove(c + 1, c + 3);
        substCount += 2;
      } else if (buffer->charAt(c) == L'c' && buffer->charAt(c + 1) == L'h') {
        buffer->setCharAt(c, L'§');
        buffer->deleteCharAt(c + 1);
        substCount++;
      } else if (buffer->charAt(c) == L'e' && buffer->charAt(c + 1) == L'i') {
        buffer->setCharAt(c, L'%');
        buffer->deleteCharAt(c + 1);
        substCount++;
      } else if (buffer->charAt(c) == L'i' && buffer->charAt(c + 1) == L'e') {
        buffer->setCharAt(c, L'&');
        buffer->deleteCharAt(c + 1);
        substCount++;
      } else if (buffer->charAt(c) == L'i' && buffer->charAt(c + 1) == L'g') {
        buffer->setCharAt(c, L'#');
        buffer->deleteCharAt(c + 1);
        substCount++;
      } else if (buffer->charAt(c) == L's' && buffer->charAt(c + 1) == L't') {
        buffer->setCharAt(c, L'!');
        buffer->deleteCharAt(c + 1);
        substCount++;
      }
    }
  }
}

void GermanStemmer::resubstitute(shared_ptr<StringBuilder> buffer)
{
  for (int c = 0; c < buffer->length(); c++) {
    if (buffer->charAt(c) == L'*') {
      wchar_t x = buffer->charAt(c - 1);
      buffer->setCharAt(c, x);
    } else if (buffer->charAt(c) == L'$') {
      buffer->setCharAt(c, L's');
      buffer->insert(c + 1, std::deque<wchar_t>{L'c', L'h'}, 0, 2);
    } else if (buffer->charAt(c) == L'§') {
      buffer->setCharAt(c, L'c');
      buffer->insert(c + 1, L'h');
    } else if (buffer->charAt(c) == L'%') {
      buffer->setCharAt(c, L'e');
      buffer->insert(c + 1, L'i');
    } else if (buffer->charAt(c) == L'&') {
      buffer->setCharAt(c, L'i');
      buffer->insert(c + 1, L'e');
    } else if (buffer->charAt(c) == L'#') {
      buffer->setCharAt(c, L'i');
      buffer->insert(c + 1, L'g');
    } else if (buffer->charAt(c) == L'!') {
      buffer->setCharAt(c, L's');
      buffer->insert(c + 1, L't');
    }
  }
}
} // namespace org::apache::lucene::analysis::de