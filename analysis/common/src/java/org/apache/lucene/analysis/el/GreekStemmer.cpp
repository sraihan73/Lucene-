using namespace std;

#include "GreekStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"

namespace org::apache::lucene::analysis::el
{
using CharArraySet = org::apache::lucene::analysis::CharArraySet;

int GreekStemmer::stem(std::deque<wchar_t> &s, int len)
{
  if (len < 4) // too short
  {
    return len;
  }

  constexpr int origLen = len;
  // "short rules": if it hits one of these, it skips the "long deque"
  len = rule0(s, len);
  len = rule1(s, len);
  len = rule2(s, len);
  len = rule3(s, len);
  len = rule4(s, len);
  len = rule5(s, len);
  len = rule6(s, len);
  len = rule7(s, len);
  len = rule8(s, len);
  len = rule9(s, len);
  len = rule10(s, len);
  len = rule11(s, len);
  len = rule12(s, len);
  len = rule13(s, len);
  len = rule14(s, len);
  len = rule15(s, len);
  len = rule16(s, len);
  len = rule17(s, len);
  len = rule18(s, len);
  len = rule19(s, len);
  len = rule20(s, len);
  // "long deque"
  if (len == origLen) {
    len = rule21(s, len);
  }

  return rule22(s, len);
}

int GreekStemmer::rule0(std::deque<wchar_t> &s, int len)
{
  if (len > 9 &&
      (endsWith(s, len, L"καθεστωτοσ") || endsWith(s, len, L"καθεστωτων"))) {
    return len - 4;
  }

  if (len > 8 &&
      (endsWith(s, len, L"γεγονοτοσ") || endsWith(s, len, L"γεγονοτων"))) {
    return len - 4;
  }

  if (len > 8 && endsWith(s, len, L"καθεστωτα")) {
    return len - 3;
  }

  if (len > 7 &&
      (endsWith(s, len, L"τατογιου") || endsWith(s, len, L"τατογιων"))) {
    return len - 4;
  }

  if (len > 7 && endsWith(s, len, L"γεγονοτα")) {
    return len - 3;
  }

  if (len > 7 && endsWith(s, len, L"καθεστωσ")) {
    return len - 2;
  }

  if (len > 6 && (endsWith(s, len, L"σκαγιου")) ||
      endsWith(s, len, L"σκαγιων") || endsWith(s, len, L"ολογιου") ||
      endsWith(s, len, L"ολογιων") || endsWith(s, len, L"κρεατοσ") ||
      endsWith(s, len, L"κρεατων") || endsWith(s, len, L"περατοσ") ||
      endsWith(s, len, L"περατων") || endsWith(s, len, L"τερατοσ") ||
      endsWith(s, len, L"τερατων")) {
    return len - 4;
  }

  if (len > 6 && endsWith(s, len, L"τατογια")) {
    return len - 3;
  }

  if (len > 6 && endsWith(s, len, L"γεγονοσ")) {
    return len - 2;
  }

  if (len > 5 && (endsWith(s, len, L"φαγιου") || endsWith(s, len, L"φαγιων") ||
                  endsWith(s, len, L"σογιου") || endsWith(s, len, L"σογιων"))) {
    return len - 4;
  }

  if (len > 5 && (endsWith(s, len, L"σκαγια") || endsWith(s, len, L"ολογια") ||
                  endsWith(s, len, L"κρεατα") || endsWith(s, len, L"περατα") ||
                  endsWith(s, len, L"τερατα"))) {
    return len - 3;
  }

  if (len > 4 && (endsWith(s, len, L"φαγια") || endsWith(s, len, L"σογια") ||
                  endsWith(s, len, L"φωτοσ") || endsWith(s, len, L"φωτων"))) {
    return len - 3;
  }

  if (len > 4 && (endsWith(s, len, L"κρεασ") || endsWith(s, len, L"περασ") ||
                  endsWith(s, len, L"τερασ"))) {
    return len - 2;
  }

  if (len > 3 && endsWith(s, len, L"φωτα")) {
    return len - 2;
  }

  if (len > 2 && endsWith(s, len, L"φωσ")) {
    return len - 1;
  }

  return len;
}

int GreekStemmer::rule1(std::deque<wchar_t> &s, int len)
{
  if (len > 4 && (endsWith(s, len, L"αδεσ") || endsWith(s, len, L"αδων"))) {
    len -= 4;
    if (!(endsWith(s, len, L"οκ") || endsWith(s, len, L"μαμ") ||
          endsWith(s, len, L"μαν") || endsWith(s, len, L"μπαμπ") ||
          endsWith(s, len, L"πατερ") || endsWith(s, len, L"γιαγι") ||
          endsWith(s, len, L"νταντ") || endsWith(s, len, L"κυρ") ||
          endsWith(s, len, L"θει") || endsWith(s, len, L"πεθερ"))) {
      len += 2; // add back -αδ
    }
  }
  return len;
}

int GreekStemmer::rule2(std::deque<wchar_t> &s, int len)
{
  if (len > 4 && (endsWith(s, len, L"εδεσ") || endsWith(s, len, L"εδων"))) {
    len -= 4;
    if (endsWith(s, len, L"οπ") || endsWith(s, len, L"ιπ") ||
        endsWith(s, len, L"εμπ") || endsWith(s, len, L"υπ") ||
        endsWith(s, len, L"γηπ") || endsWith(s, len, L"δαπ") ||
        endsWith(s, len, L"κρασπ") || endsWith(s, len, L"μιλ")) {
      len += 2; // add back -εδ
    }
  }
  return len;
}

int GreekStemmer::rule3(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && (endsWith(s, len, L"ουδεσ") || endsWith(s, len, L"ουδων"))) {
    len -= 5;
    if (endsWith(s, len, L"αρκ") || endsWith(s, len, L"καλιακ") ||
        endsWith(s, len, L"πεταλ") || endsWith(s, len, L"λιχ") ||
        endsWith(s, len, L"πλεξ") || endsWith(s, len, L"σκ") ||
        endsWith(s, len, L"σ") || endsWith(s, len, L"φλ") ||
        endsWith(s, len, L"φρ") || endsWith(s, len, L"βελ") ||
        endsWith(s, len, L"λουλ") || endsWith(s, len, L"χν") ||
        endsWith(s, len, L"σπ") || endsWith(s, len, L"τραγ") ||
        endsWith(s, len, L"φε")) {
      len += 3; // add back -ουδ
    }
  }
  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc4 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"θ", L"δ", L"ελ", L"γαλ", L"ν", L"π",
                                       L"ιδ", L"παρ"),
            false);

int GreekStemmer::rule4(std::deque<wchar_t> &s, int len)
{
  if (len > 3 && (endsWith(s, len, L"εωσ") || endsWith(s, len, L"εων"))) {
    len -= 3;
    if (exc4->contains(s, 0, len)) {
      len++; // add back -ε
    }
  }
  return len;
}

int GreekStemmer::rule5(std::deque<wchar_t> &s, int len)
{
  if (len > 2 && endsWith(s, len, L"ια")) {
    len -= 2;
    if (endsWithVowel(s, len)) {
      len++; // add back -ι
    }
  } else if (len > 3 &&
             (endsWith(s, len, L"ιου") || endsWith(s, len, L"ιων"))) {
    len -= 3;
    if (endsWithVowel(s, len)) {
      len++; // add back -ι
    }
  }
  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc6 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(
                L"αλ", L"αδ", L"ενδ", L"αμαν", L"αμμοχαλ", L"ηθ", L"ανηθ",
                L"αντιδ", L"φυσ", L"βρωμ", L"γερ", L"εξωδ", L"καλπ", L"καλλιν",
                L"καταδ", L"μουλ", L"μπαν", L"μπαγιατ", L"μπολ", L"μποσ",
                L"νιτ", L"ξικ", L"συνομηλ", L"πετσ", L"πιτσ", L"πικαντ",
                L"πλιατσ", L"ποστελν", L"πρωτοδ", L"σερτ", L"συναδ", L"τσαμ",
                L"υποδ", L"φιλον", L"φυλοδ", L"χασ"),
            false);

int GreekStemmer::rule6(std::deque<wchar_t> &s, int len)
{
  bool removed = false;
  if (len > 3 && (endsWith(s, len, L"ικα") || endsWith(s, len, L"ικο"))) {
    len -= 3;
    removed = true;
  } else if (len > 4 &&
             (endsWith(s, len, L"ικου") || endsWith(s, len, L"ικων"))) {
    len -= 4;
    removed = true;
  }

  if (removed) {
    if (endsWithVowel(s, len) || exc6->contains(s, 0, len)) {
      len += 2; // add back -ικ
    }
  }
  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc7 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"αναπ", L"αποθ", L"αποκ", L"αποστ",
                                       L"βουβ", L"ξεθ", L"ουλ", L"πεθ", L"πικρ",
                                       L"ποτ", L"σιχ", L"χ"),
            false);

int GreekStemmer::rule7(std::deque<wchar_t> &s, int len)
{
  if (len == 5 && endsWith(s, len, L"αγαμε")) {
    return len - 1;
  }

  if (len > 7 && endsWith(s, len, L"ηθηκαμε")) {
    len -= 7;
  } else if (len > 6 && endsWith(s, len, L"ουσαμε")) {
    len -= 6;
  } else if (len > 5 &&
             (endsWith(s, len, L"αγαμε") || endsWith(s, len, L"ησαμε") ||
              endsWith(s, len, L"ηκαμε"))) {
    len -= 5;
  }

  if (len > 3 && endsWith(s, len, L"αμε")) {
    len -= 3;
    if (exc7->contains(s, 0, len)) {
      len += 2; // add back -αμ
    }
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc8a =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"τρ", L"τσ"), false);
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc8b =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(
                L"βετερ", L"βουλκ", L"βραχμ", L"γ", L"δραδουμ", L"θ",
                L"καλπουζ", L"καστελ", L"κορμορ", L"λαοπλ", L"μωαμεθ", L"μ",
                L"μουσουλμ", L"ν", L"ουλ", L"π", L"πελεκ", L"πλ", L"πολισ",
                L"πορτολ", L"σαρακατσ", L"σουλτ", L"τσαρλατ", L"ορφ", L"τσιγγ",
                L"τσοπ", L"φωτοστεφ", L"χ", L"ψυχοπλ", L"αγ", L"ορφ", L"γαλ",
                L"γερ", L"δεκ", L"διπλ", L"αμερικαν", L"ουρ", L"πιθ", L"πουριτ",
                L"σ", L"ζωντ", L"ικ", L"καστ", L"κοπ", L"λιχ", L"λουθηρ",
                L"μαιντ", L"μελ", L"σιγ", L"σπ", L"στεγ", L"τραγ", L"τσαγ",
                L"φ", L"ερ", L"αδαπ", L"αθιγγ", L"αμηχ", L"ανικ", L"ανοργ",
                L"απηγ", L"απιθ", L"ατσιγγ", L"βασ", L"βασκ", L"βαθυγαλ",
                L"βιομηχ", L"βραχυκ", L"διατ", L"διαφ", L"ενοργ", L"θυσ",
                L"καπνοβιομηχ", L"καταγαλ", L"κλιβ", L"κοιλαρφ", L"λιβ",
                L"μεγλοβιομηχ", L"μικροβιομηχ", L"νταβ", L"ξηροκλιβ",
                L"ολιγοδαμ", L"ολογαλ", L"πενταρφ", L"περηφ", L"περιτρ",
                L"πλατ", L"πολυδαπ", L"πολυμηχ", L"στεφ", L"ταβ", L"τετ",
                L"υπερηφ", L"υποκοπ", L"χαμηλοδαπ", L"ψηλοταβ"),
            false);

int GreekStemmer::rule8(std::deque<wchar_t> &s, int len)
{
  bool removed = false;

  if (len > 8 && endsWith(s, len, L"ιουντανε")) {
    len -= 8;
    removed = true;
  } else if (len > 7 && endsWith(s, len, L"ιοντανε") ||
             endsWith(s, len, L"ουντανε") || endsWith(s, len, L"ηθηκανε")) {
    len -= 7;
    removed = true;
  } else if (len > 6 && endsWith(s, len, L"ιοτανε") ||
             endsWith(s, len, L"οντανε") || endsWith(s, len, L"ουσανε")) {
    len -= 6;
    removed = true;
  } else if (len > 5 && endsWith(s, len, L"αγανε") ||
             endsWith(s, len, L"ησανε") || endsWith(s, len, L"οτανε") ||
             endsWith(s, len, L"ηκανε")) {
    len -= 5;
    removed = true;
  }

  if (removed && exc8a->contains(s, 0, len)) {
    // add -αγαν (we removed > 4 chars so it's safe)
    len += 4;
    s[len - 4] = L'α';
    s[len - 3] = L'γ';
    s[len - 2] = L'α';
    s[len - 1] = L'ν';
  }

  if (len > 3 && endsWith(s, len, L"ανε")) {
    len -= 3;
    if (endsWithVowelNoY(s, len) || exc8b->contains(s, 0, len)) {
      len += 2; // add back -αν
    }
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc9 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(
                L"αβαρ", L"βεν", L"εναρ", L"αβρ", L"αδ", L"αθ", L"αν", L"απλ",
                L"βαρον", L"ντρ", L"σκ", L"κοπ", L"μπορ", L"νιφ", L"παγ",
                L"παρακαλ", L"σερπ", L"σκελ", L"συρφ", L"τοκ", L"υ", L"δ",
                L"εμ", L"θαρρ", L"θ"),
            false);

int GreekStemmer::rule9(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && endsWith(s, len, L"ησετε")) {
    len -= 5;
  }

  if (len > 3 && endsWith(s, len, L"ετε")) {
    len -= 3;
    if (exc9->contains(s, 0, len) || endsWithVowelNoY(s, len) ||
        endsWith(s, len, L"οδ") || endsWith(s, len, L"αιρ") ||
        endsWith(s, len, L"φορ") || endsWith(s, len, L"ταθ") ||
        endsWith(s, len, L"διαθ") || endsWith(s, len, L"σχ") ||
        endsWith(s, len, L"ενδ") || endsWith(s, len, L"ευρ") ||
        endsWith(s, len, L"τιθ") || endsWith(s, len, L"υπερθ") ||
        endsWith(s, len, L"ραθ") || endsWith(s, len, L"ενθ") ||
        endsWith(s, len, L"ροθ") || endsWith(s, len, L"σθ") ||
        endsWith(s, len, L"πυρ") || endsWith(s, len, L"αιν") ||
        endsWith(s, len, L"συνδ") || endsWith(s, len, L"συν") ||
        endsWith(s, len, L"συνθ") || endsWith(s, len, L"χωρ") ||
        endsWith(s, len, L"πον") || endsWith(s, len, L"βρ") ||
        endsWith(s, len, L"καθ") || endsWith(s, len, L"ευθ") ||
        endsWith(s, len, L"εκθ") || endsWith(s, len, L"νετ") ||
        endsWith(s, len, L"ρον") || endsWith(s, len, L"αρκ") ||
        endsWith(s, len, L"βαρ") || endsWith(s, len, L"βολ") ||
        endsWith(s, len, L"ωφελ")) {
      len += 2; // add back -ετ
    }
  }

  return len;
}

int GreekStemmer::rule10(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && (endsWith(s, len, L"οντασ") || endsWith(s, len, L"ωντασ"))) {
    len -= 5;
    if (len == 3 && endsWith(s, len, L"αρχ")) {
      len += 3; // add back *ντ
      s[len - 3] = L'ο';
    }
    if (endsWith(s, len, L"κρε")) {
      len += 3; // add back *ντ
      s[len - 3] = L'ω';
    }
  }

  return len;
}

int GreekStemmer::rule11(std::deque<wchar_t> &s, int len)
{
  if (len > 6 && endsWith(s, len, L"ομαστε")) {
    len -= 6;
    if (len == 2 && endsWith(s, len, L"ον")) {
      len += 5; // add back -ομαστ
    }
  } else if (len > 7 && endsWith(s, len, L"ιομαστε")) {
    len -= 7;
    if (len == 2 && endsWith(s, len, L"ον")) {
      len += 5;
      s[len - 5] = L'ο';
      s[len - 4] = L'μ';
      s[len - 3] = L'α';
      s[len - 2] = L'σ';
      s[len - 1] = L'τ';
    }
  }
  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc12a =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"π", L"απ", L"συμπ", L"ασυμπ",
                                       L"ακαταπ", L"αμεταμφ"),
            false);
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc12b =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"αλ", L"αρ", L"εκτελ", L"ζ", L"μ", L"ξ",
                                       L"παρακαλ", L"αρ", L"προ", L"νισ"),
            false);

int GreekStemmer::rule12(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && endsWith(s, len, L"ιεστε")) {
    len -= 5;
    if (exc12a->contains(s, 0, len)) {
      len += 4; // add back -ιεστ
    }
  }

  if (len > 4 && endsWith(s, len, L"εστε")) {
    len -= 4;
    if (exc12b->contains(s, 0, len)) {
      len += 3; // add back -εστ
    }
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc13 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"διαθ", L"θ", L"παρακαταθ", L"προσθ",
                                       L"συνθ"),
            false);

int GreekStemmer::rule13(std::deque<wchar_t> &s, int len)
{
  if (len > 6 && endsWith(s, len, L"ηθηκεσ")) {
    len -= 6;
  } else if (len > 5 &&
             (endsWith(s, len, L"ηθηκα") || endsWith(s, len, L"ηθηκε"))) {
    len -= 5;
  }

  bool removed = false;

  if (len > 4 && endsWith(s, len, L"ηκεσ")) {
    len -= 4;
    removed = true;
  } else if (len > 3 &&
             (endsWith(s, len, L"ηκα") || endsWith(s, len, L"ηκε"))) {
    len -= 3;
    removed = true;
  }

  if (removed && (exc13->contains(s, 0, len) || endsWith(s, len, L"σκωλ") ||
                  endsWith(s, len, L"σκουλ") || endsWith(s, len, L"ναρθ") ||
                  endsWith(s, len, L"σφ") || endsWith(s, len, L"οθ") ||
                  endsWith(s, len, L"πιθ"))) {
    len += 2; // add back the -ηκ
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc14 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(
                L"φαρμακ", L"χαδ", L"αγκ", L"αναρρ", L"βρομ", L"εκλιπ",
                L"λαμπιδ", L"λεχ", L"μ", L"πατ", L"ρ", L"λ", L"μεδ", L"μεσαζ",
                L"υποτειν", L"αμ", L"αιθ", L"ανηκ", L"δεσποζ", L"ενδιαφερ",
                L"δε", L"δευτερευ", L"καθαρευ", L"πλε", L"τσα"),
            false);

int GreekStemmer::rule14(std::deque<wchar_t> &s, int len)
{
  bool removed = false;

  if (len > 5 && endsWith(s, len, L"ουσεσ")) {
    len -= 5;
    removed = true;
  } else if (len > 4 &&
             (endsWith(s, len, L"ουσα") || endsWith(s, len, L"ουσε"))) {
    len -= 4;
    removed = true;
  }

  if (removed && (exc14->contains(s, 0, len) || endsWithVowel(s, len) ||
                  endsWith(s, len, L"ποδαρ") || endsWith(s, len, L"βλεπ") ||
                  endsWith(s, len, L"πανταχ") || endsWith(s, len, L"φρυδ") ||
                  endsWith(s, len, L"μαντιλ") || endsWith(s, len, L"μαλλ") ||
                  endsWith(s, len, L"κυματ") || endsWith(s, len, L"λαχ") ||
                  endsWith(s, len, L"ληγ") || endsWith(s, len, L"φαγ") ||
                  endsWith(s, len, L"ομ") || endsWith(s, len, L"πρωτ"))) {
    len += 3; // add back -ουσ
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc15a =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(
                L"αβαστ", L"πολυφ", L"αδηφ", L"παμφ", L"ρ", L"ασπ", L"αφ",
                L"αμαλ", L"αμαλλι", L"ανυστ", L"απερ", L"ασπαρ", L"αχαρ",
                L"δερβεν", L"δροσοπ", L"ξεφ", L"νεοπ", L"νομοτ", L"ολοπ",
                L"ομοτ", L"προστ", L"προσωποπ", L"συμπ", L"συντ", L"τ", L"υποτ",
                L"χαρ", L"αειπ", L"αιμοστ", L"ανυπ", L"αποτ", L"αρτιπ", L"διατ",
                L"εν", L"επιτ", L"κροκαλοπ", L"σιδηροπ", L"λ", L"ναυ", L"ουλαμ",
                L"ουρ", L"π", L"τρ", L"μ"),
            false);
const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc15b =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"ψοφ", L"ναυλοχ"), false);

int GreekStemmer::rule15(std::deque<wchar_t> &s, int len)
{
  bool removed = false;
  if (len > 4 && endsWith(s, len, L"αγεσ")) {
    len -= 4;
    removed = true;
  } else if (len > 3 &&
             (endsWith(s, len, L"αγα") || endsWith(s, len, L"αγε"))) {
    len -= 3;
    removed = true;
  }

  if (removed) {
    constexpr bool cond1 =
        exc15a->contains(s, 0, len) || endsWith(s, len, L"οφ") ||
        endsWith(s, len, L"πελ") || endsWith(s, len, L"χορτ") ||
        endsWith(s, len, L"λλ") || endsWith(s, len, L"σφ") ||
        endsWith(s, len, L"ρπ") || endsWith(s, len, L"φρ") ||
        endsWith(s, len, L"πρ") || endsWith(s, len, L"λοχ") ||
        endsWith(s, len, L"σμην");

    constexpr bool cond2 =
        exc15b->contains(s, 0, len) || endsWith(s, len, L"κολλ");

    if (cond1 && !cond2) {
      len += 2; // add back -αγ
    }
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc16 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"ν", L"χερσον", L"δωδεκαν", L"ερημον",
                                       L"μεγαλον", L"επταν"),
            false);

int GreekStemmer::rule16(std::deque<wchar_t> &s, int len)
{
  bool removed = false;
  if (len > 4 && endsWith(s, len, L"ησου")) {
    len -= 4;
    removed = true;
  } else if (len > 3 &&
             (endsWith(s, len, L"ησε") || endsWith(s, len, L"ησα"))) {
    len -= 3;
    removed = true;
  }

  if (removed && exc16->contains(s, 0, len)) {
    len += 2; // add back -ησ
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc17 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"ασβ", L"σβ", L"αχρ", L"χρ", L"απλ",
                                       L"αειμν", L"δυσχρ", L"ευχρ", L"κοινοχρ",
                                       L"παλιμψ"),
            false);

int GreekStemmer::rule17(std::deque<wchar_t> &s, int len)
{
  if (len > 4 && endsWith(s, len, L"ηστε")) {
    len -= 4;
    if (exc17->contains(s, 0, len)) {
      len += 3; // add back the -ηστ
    }
  }

  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc18 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"ν", L"ρ", L"σπι", L"στραβομουτσ",
                                       L"κακομουτσ", L"εξων"),
            false);

int GreekStemmer::rule18(std::deque<wchar_t> &s, int len)
{
  bool removed = false;

  if (len > 6 && (endsWith(s, len, L"ησουνε") || endsWith(s, len, L"ηθουνε"))) {
    len -= 6;
    removed = true;
  } else if (len > 4 && endsWith(s, len, L"ουνε")) {
    len -= 4;
    removed = true;
  }

  if (removed && exc18->contains(s, 0, len)) {
    len += 3;
    s[len - 3] = L'ο';
    s[len - 2] = L'υ';
    s[len - 1] = L'ν';
  }
  return len;
}

const shared_ptr<org::apache::lucene::analysis::CharArraySet>
    GreekStemmer::exc19 =
        make_shared<org::apache::lucene::analysis::CharArraySet>(
            java::util::Arrays::asList(L"παρασουσ", L"φ", L"χ", L"ωριοπλ",
                                       L"αζ", L"αλλοσουσ", L"ασουσ"),
            false);

int GreekStemmer::rule19(std::deque<wchar_t> &s, int len)
{
  bool removed = false;

  if (len > 6 && (endsWith(s, len, L"ησουμε") || endsWith(s, len, L"ηθουμε"))) {
    len -= 6;
    removed = true;
  } else if (len > 4 && endsWith(s, len, L"ουμε")) {
    len -= 4;
    removed = true;
  }

  if (removed && exc19->contains(s, 0, len)) {
    len += 3;
    s[len - 3] = L'ο';
    s[len - 2] = L'υ';
    s[len - 1] = L'μ';
  }
  return len;
}

int GreekStemmer::rule20(std::deque<wchar_t> &s, int len)
{
  if (len > 5 && (endsWith(s, len, L"ματων") || endsWith(s, len, L"ματοσ"))) {
    len -= 3;
  } else if (len > 4 && endsWith(s, len, L"ματα")) {
    len -= 2;
  }
  return len;
}

int GreekStemmer::rule21(std::deque<wchar_t> &s, int len)
{
  if (len > 9 && endsWith(s, len, L"ιοντουσαν")) {
    return len - 9;
  }

  if (len > 8 &&
      (endsWith(s, len, L"ιομασταν") || endsWith(s, len, L"ιοσασταν") ||
       endsWith(s, len, L"ιουμαστε") || endsWith(s, len, L"οντουσαν"))) {
    return len - 8;
  }

  if (len > 7 &&
      (endsWith(s, len, L"ιεμαστε") || endsWith(s, len, L"ιεσαστε") ||
       endsWith(s, len, L"ιομουνα") || endsWith(s, len, L"ιοσαστε") ||
       endsWith(s, len, L"ιοσουνα") || endsWith(s, len, L"ιουνται") ||
       endsWith(s, len, L"ιουνταν") || endsWith(s, len, L"ηθηκατε") ||
       endsWith(s, len, L"ομασταν") || endsWith(s, len, L"οσασταν") ||
       endsWith(s, len, L"ουμαστε"))) {
    return len - 7;
  }

  if (len > 6 && (endsWith(s, len, L"ιομουν") || endsWith(s, len, L"ιονταν") ||
                  endsWith(s, len, L"ιοσουν") || endsWith(s, len, L"ηθειτε") ||
                  endsWith(s, len, L"ηθηκαν") || endsWith(s, len, L"ομουνα") ||
                  endsWith(s, len, L"οσαστε") || endsWith(s, len, L"οσουνα") ||
                  endsWith(s, len, L"ουνται") || endsWith(s, len, L"ουνταν") ||
                  endsWith(s, len, L"ουσατε"))) {
    return len - 6;
  }

  if (len > 5 && (endsWith(s, len, L"αγατε") || endsWith(s, len, L"ιεμαι") ||
                  endsWith(s, len, L"ιεται") || endsWith(s, len, L"ιεσαι") ||
                  endsWith(s, len, L"ιοταν") || endsWith(s, len, L"ιουμα") ||
                  endsWith(s, len, L"ηθεισ") || endsWith(s, len, L"ηθουν") ||
                  endsWith(s, len, L"ηκατε") || endsWith(s, len, L"ησατε") ||
                  endsWith(s, len, L"ησουν") || endsWith(s, len, L"ομουν") ||
                  endsWith(s, len, L"ονται") || endsWith(s, len, L"ονταν") ||
                  endsWith(s, len, L"οσουν") || endsWith(s, len, L"ουμαι") ||
                  endsWith(s, len, L"ουσαν"))) {
    return len - 5;
  }

  if (len > 4 && (endsWith(s, len, L"αγαν") || endsWith(s, len, L"αμαι") ||
                  endsWith(s, len, L"ασαι") || endsWith(s, len, L"αται") ||
                  endsWith(s, len, L"ειτε") || endsWith(s, len, L"εσαι") ||
                  endsWith(s, len, L"εται") || endsWith(s, len, L"ηδεσ") ||
                  endsWith(s, len, L"ηδων") || endsWith(s, len, L"ηθει") ||
                  endsWith(s, len, L"ηκαν") || endsWith(s, len, L"ησαν") ||
                  endsWith(s, len, L"ησει") || endsWith(s, len, L"ησεσ") ||
                  endsWith(s, len, L"ομαι") || endsWith(s, len, L"οταν"))) {
    return len - 4;
  }

  if (len > 3 && (endsWith(s, len, L"αει") || endsWith(s, len, L"εισ") ||
                  endsWith(s, len, L"ηθω") || endsWith(s, len, L"ησω") ||
                  endsWith(s, len, L"ουν") || endsWith(s, len, L"ουσ"))) {
    return len - 3;
  }

  if (len > 2 && (endsWith(s, len, L"αν") || endsWith(s, len, L"ασ") ||
                  endsWith(s, len, L"αω") || endsWith(s, len, L"ει") ||
                  endsWith(s, len, L"εσ") || endsWith(s, len, L"ησ") ||
                  endsWith(s, len, L"οι") || endsWith(s, len, L"οσ") ||
                  endsWith(s, len, L"ου") || endsWith(s, len, L"υσ") ||
                  endsWith(s, len, L"ων"))) {
    return len - 2;
  }

  if (len > 1 && endsWithVowel(s, len)) {
    return len - 1;
  }

  return len;
}

int GreekStemmer::rule22(std::deque<wchar_t> &s, int len)
{
  if (endsWith(s, len, L"εστερ") || endsWith(s, len, L"εστατ")) {
    return len - 5;
  }

  if (endsWith(s, len, L"οτερ") || endsWith(s, len, L"οτατ") ||
      endsWith(s, len, L"υτερ") || endsWith(s, len, L"υτατ") ||
      endsWith(s, len, L"ωτερ") || endsWith(s, len, L"ωτατ")) {
    return len - 4;
  }

  return len;
}

bool GreekStemmer::endsWith(std::deque<wchar_t> &s, int len,
                            const wstring &suffix)
{
  constexpr int suffixLen = suffix.length();
  if (suffixLen > len) {
    return false;
  }
  for (int i = suffixLen - 1; i >= 0; i--) {
    if (s[len - (suffixLen - i)] != suffix[i]) {
      return false;
    }
  }

  return true;
}

bool GreekStemmer::endsWithVowel(std::deque<wchar_t> &s, int len)
{
  if (len == 0) {
    return false;
  }
  switch (s[len - 1]) {
  case L'α':
  case L'ε':
  case L'η':
  case L'ι':
  case L'ο':
  case L'υ':
  case L'ω':
    return true;
  default:
    return false;
  }
}

bool GreekStemmer::endsWithVowelNoY(std::deque<wchar_t> &s, int len)
{
  if (len == 0) {
    return false;
  }
  switch (s[len - 1]) {
  case L'α':
  case L'ε':
  case L'η':
  case L'ι':
  case L'ο':
  case L'ω':
    return true;
  default:
    return false;
  }
}
} // namespace org::apache::lucene::analysis::el