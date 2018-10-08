using namespace std;

#include "TestGreekStemmer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/el/GreekAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/el/GreekStemFilter.h"

namespace org::apache::lucene::analysis::el
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestGreekStemmer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<GreekAnalyzer>();
}

void TestGreekStemmer::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestGreekStemmer::testMasculineNouns() 
{
  // -ος
  checkOneTerm(a, L"άνθρωπος", L"ανθρωπ");
  checkOneTerm(a, L"ανθρώπου", L"ανθρωπ");
  checkOneTerm(a, L"άνθρωπο", L"ανθρωπ");
  checkOneTerm(a, L"άνθρωπε", L"ανθρωπ");
  checkOneTerm(a, L"άνθρωποι", L"ανθρωπ");
  checkOneTerm(a, L"ανθρώπων", L"ανθρωπ");
  checkOneTerm(a, L"ανθρώπους", L"ανθρωπ");
  checkOneTerm(a, L"άνθρωποι", L"ανθρωπ");

  // -ης
  checkOneTerm(a, L"πελάτης", L"πελατ");
  checkOneTerm(a, L"πελάτη", L"πελατ");
  checkOneTerm(a, L"πελάτες", L"πελατ");
  checkOneTerm(a, L"πελατών", L"πελατ");

  // -ας/-ες
  checkOneTerm(a, L"ελέφαντας", L"ελεφαντ");
  checkOneTerm(a, L"ελέφαντα", L"ελεφαντ");
  checkOneTerm(a, L"ελέφαντες", L"ελεφαντ");
  checkOneTerm(a, L"ελεφάντων", L"ελεφαντ");

  // -ας/-αδες
  checkOneTerm(a, L"μπαμπάς", L"μπαμπ");
  checkOneTerm(a, L"μπαμπά", L"μπαμπ");
  checkOneTerm(a, L"μπαμπάδες", L"μπαμπ");
  checkOneTerm(a, L"μπαμπάδων", L"μπαμπ");

  // -ης/-ηδες
  checkOneTerm(a, L"μπακάλης", L"μπακαλ");
  checkOneTerm(a, L"μπακάλη", L"μπακαλ");
  checkOneTerm(a, L"μπακάληδες", L"μπακαλ");
  checkOneTerm(a, L"μπακάληδων", L"μπακαλ");

  // -ες
  checkOneTerm(a, L"καφές", L"καφ");
  checkOneTerm(a, L"καφέ", L"καφ");
  checkOneTerm(a, L"καφέδες", L"καφ");
  checkOneTerm(a, L"καφέδων", L"καφ");

  // -έας/είς
  checkOneTerm(a, L"γραμματέας", L"γραμματε");
  checkOneTerm(a, L"γραμματέα", L"γραμματε");
  // plural forms conflate w/ each other, not w/ the sing forms
  checkOneTerm(a, L"γραμματείς", L"γραμματ");
  checkOneTerm(a, L"γραμματέων", L"γραμματ");

  // -ους/οι
  checkOneTerm(a, L"απόπλους", L"αποπλ");
  checkOneTerm(a, L"απόπλου", L"αποπλ");
  checkOneTerm(a, L"απόπλοι", L"αποπλ");
  checkOneTerm(a, L"απόπλων", L"αποπλ");

  // -ους/-ουδες
  checkOneTerm(a, L"παππούς", L"παππ");
  checkOneTerm(a, L"παππού", L"παππ");
  checkOneTerm(a, L"παππούδες", L"παππ");
  checkOneTerm(a, L"παππούδων", L"παππ");

  // -ης/-εις
  checkOneTerm(a, L"λάτρης", L"λατρ");
  checkOneTerm(a, L"λάτρη", L"λατρ");
  checkOneTerm(a, L"λάτρεις", L"λατρ");
  checkOneTerm(a, L"λάτρεων", L"λατρ");

  // -υς
  checkOneTerm(a, L"πέλεκυς", L"πελεκ");
  checkOneTerm(a, L"πέλεκυ", L"πελεκ");
  checkOneTerm(a, L"πελέκεις", L"πελεκ");
  checkOneTerm(a, L"πελέκεων", L"πελεκ");

  // -ωρ
  // note: nom./voc. doesn't conflate w/ the rest
  checkOneTerm(a, L"μέντωρ", L"μεντωρ");
  checkOneTerm(a, L"μέντορος", L"μεντορ");
  checkOneTerm(a, L"μέντορα", L"μεντορ");
  checkOneTerm(a, L"μέντορες", L"μεντορ");
  checkOneTerm(a, L"μεντόρων", L"μεντορ");

  // -ων
  checkOneTerm(a, L"αγώνας", L"αγων");
  checkOneTerm(a, L"αγώνος", L"αγων");
  checkOneTerm(a, L"αγώνα", L"αγων");
  checkOneTerm(a, L"αγώνα", L"αγων");
  checkOneTerm(a, L"αγώνες", L"αγων");
  checkOneTerm(a, L"αγώνων", L"αγων");

  // -ας/-ηδες
  checkOneTerm(a, L"αέρας", L"αερ");
  checkOneTerm(a, L"αέρα", L"αερ");
  checkOneTerm(a, L"αέρηδες", L"αερ");
  checkOneTerm(a, L"αέρηδων", L"αερ");

  // -ης/-ητες
  checkOneTerm(a, L"γόης", L"γο");
  checkOneTerm(a, L"γόη", L"γοη"); // too short
  // the two plural forms conflate
  checkOneTerm(a, L"γόητες", L"γοητ");
  checkOneTerm(a, L"γοήτων", L"γοητ");
}

void TestGreekStemmer::testFeminineNouns() 
{
  // -α/-ες,-ών
  checkOneTerm(a, L"φορά", L"φορ");
  checkOneTerm(a, L"φοράς", L"φορ");
  checkOneTerm(a, L"φορές", L"φορ");
  checkOneTerm(a, L"φορών", L"φορ");

  // -α/-ες,-ων
  checkOneTerm(a, L"αγελάδα", L"αγελαδ");
  checkOneTerm(a, L"αγελάδας", L"αγελαδ");
  checkOneTerm(a, L"αγελάδες", L"αγελαδ");
  checkOneTerm(a, L"αγελάδων", L"αγελαδ");

  // -η/-ες
  checkOneTerm(a, L"ζάχαρη", L"ζαχαρ");
  checkOneTerm(a, L"ζάχαρης", L"ζαχαρ");
  checkOneTerm(a, L"ζάχαρες", L"ζαχαρ");
  checkOneTerm(a, L"ζαχάρεων", L"ζαχαρ");

  // -η/-εις
  checkOneTerm(a, L"τηλεόραση", L"τηλεορασ");
  checkOneTerm(a, L"τηλεόρασης", L"τηλεορασ");
  checkOneTerm(a, L"τηλεοράσεις", L"τηλεορασ");
  checkOneTerm(a, L"τηλεοράσεων", L"τηλεορασ");

  // -α/-αδες
  checkOneTerm(a, L"μαμά", L"μαμ");
  checkOneTerm(a, L"μαμάς", L"μαμ");
  checkOneTerm(a, L"μαμάδες", L"μαμ");
  checkOneTerm(a, L"μαμάδων", L"μαμ");

  // -ος
  checkOneTerm(a, L"λεωφόρος", L"λεωφορ");
  checkOneTerm(a, L"λεωφόρου", L"λεωφορ");
  checkOneTerm(a, L"λεωφόρο", L"λεωφορ");
  checkOneTerm(a, L"λεωφόρε", L"λεωφορ");
  checkOneTerm(a, L"λεωφόροι", L"λεωφορ");
  checkOneTerm(a, L"λεωφόρων", L"λεωφορ");
  checkOneTerm(a, L"λεωφόρους", L"λεωφορ");

  // -ου
  checkOneTerm(a, L"αλεπού", L"αλεπ");
  checkOneTerm(a, L"αλεπούς", L"αλεπ");
  checkOneTerm(a, L"αλεπούδες", L"αλεπ");
  checkOneTerm(a, L"αλεπούδων", L"αλεπ");

  // -έας/είς
  // note: not all forms conflate
  checkOneTerm(a, L"γραμματέας", L"γραμματε");
  checkOneTerm(a, L"γραμματέως", L"γραμματ");
  checkOneTerm(a, L"γραμματέα", L"γραμματε");
  checkOneTerm(a, L"γραμματείς", L"γραμματ");
  checkOneTerm(a, L"γραμματέων", L"γραμματ");
}

void TestGreekStemmer::testNeuterNouns() 
{
  // ending with -ο
  // note: nom doesnt conflate
  checkOneTerm(a, L"βιβλίο", L"βιβλι");
  checkOneTerm(a, L"βιβλίου", L"βιβλ");
  checkOneTerm(a, L"βιβλία", L"βιβλ");
  checkOneTerm(a, L"βιβλίων", L"βιβλ");

  // ending with -ι
  checkOneTerm(a, L"πουλί", L"πουλ");
  checkOneTerm(a, L"πουλιού", L"πουλ");
  checkOneTerm(a, L"πουλιά", L"πουλ");
  checkOneTerm(a, L"πουλιών", L"πουλ");

  // ending with -α
  // note: nom. doesnt conflate
  checkOneTerm(a, L"πρόβλημα", L"προβλημ");
  checkOneTerm(a, L"προβλήματος", L"προβλημα");
  checkOneTerm(a, L"προβλήματα", L"προβλημα");
  checkOneTerm(a, L"προβλημάτων", L"προβλημα");

  // ending with -ος/-ους
  checkOneTerm(a, L"πέλαγος", L"πελαγ");
  checkOneTerm(a, L"πελάγους", L"πελαγ");
  checkOneTerm(a, L"πελάγη", L"πελαγ");
  checkOneTerm(a, L"πελάγων", L"πελαγ");

  // ending with -ός/-ότος
  checkOneTerm(a, L"γεγονός", L"γεγον");
  checkOneTerm(a, L"γεγονότος", L"γεγον");
  checkOneTerm(a, L"γεγονότα", L"γεγον");
  checkOneTerm(a, L"γεγονότων", L"γεγον");

  // ending with -υ/-ιου
  checkOneTerm(a, L"βράδυ", L"βραδ");
  checkOneTerm(a, L"βράδι", L"βραδ");
  checkOneTerm(a, L"βραδιού", L"βραδ");
  checkOneTerm(a, L"βράδια", L"βραδ");
  checkOneTerm(a, L"βραδιών", L"βραδ");

  // ending with -υ/-ατος
  // note: nom. doesnt conflate
  checkOneTerm(a, L"δόρυ", L"δορ");
  checkOneTerm(a, L"δόρατος", L"δορατ");
  checkOneTerm(a, L"δόρατα", L"δορατ");
  checkOneTerm(a, L"δοράτων", L"δορατ");

  // ending with -ας
  checkOneTerm(a, L"κρέας", L"κρε");
  checkOneTerm(a, L"κρέατος", L"κρε");
  checkOneTerm(a, L"κρέατα", L"κρε");
  checkOneTerm(a, L"κρεάτων", L"κρε");

  // ending with -ως
  checkOneTerm(a, L"λυκόφως", L"λυκοφω");
  checkOneTerm(a, L"λυκόφωτος", L"λυκοφω");
  checkOneTerm(a, L"λυκόφωτα", L"λυκοφω");
  checkOneTerm(a, L"λυκοφώτων", L"λυκοφω");

  // ending with -ον/-ου
  // note: nom. doesnt conflate
  checkOneTerm(a, L"μέσον", L"μεσον");
  checkOneTerm(a, L"μέσου", L"μεσ");
  checkOneTerm(a, L"μέσα", L"μεσ");
  checkOneTerm(a, L"μέσων", L"μεσ");

  // ending in -ον/-οντος
  // note: nom. doesnt conflate
  checkOneTerm(a, L"ενδιαφέρον", L"ενδιαφερον");
  checkOneTerm(a, L"ενδιαφέροντος", L"ενδιαφεροντ");
  checkOneTerm(a, L"ενδιαφέροντα", L"ενδιαφεροντ");
  checkOneTerm(a, L"ενδιαφερόντων", L"ενδιαφεροντ");

  // ending with -εν/-εντος
  checkOneTerm(a, L"ανακοινωθέν", L"ανακοινωθεν");
  checkOneTerm(a, L"ανακοινωθέντος", L"ανακοινωθεντ");
  checkOneTerm(a, L"ανακοινωθέντα", L"ανακοινωθεντ");
  checkOneTerm(a, L"ανακοινωθέντων", L"ανακοινωθεντ");

  // ending with -αν/-αντος
  checkOneTerm(a, L"σύμπαν", L"συμπ");
  checkOneTerm(a, L"σύμπαντος", L"συμπαντ");
  checkOneTerm(a, L"σύμπαντα", L"συμπαντ");
  checkOneTerm(a, L"συμπάντων", L"συμπαντ");

  // ending with  -α/-ακτος
  checkOneTerm(a, L"γάλα", L"γαλ");
  checkOneTerm(a, L"γάλακτος", L"γαλακτ");
  checkOneTerm(a, L"γάλατα", L"γαλατ");
  checkOneTerm(a, L"γαλάκτων", L"γαλακτ");
}

void TestGreekStemmer::testAdjectives() 
{
  // ending with -ής, -ές/-είς, -ή
  checkOneTerm(a, L"συνεχής", L"συνεχ");
  checkOneTerm(a, L"συνεχούς", L"συνεχ");
  checkOneTerm(a, L"συνεχή", L"συνεχ");
  checkOneTerm(a, L"συνεχών", L"συνεχ");
  checkOneTerm(a, L"συνεχείς", L"συνεχ");
  checkOneTerm(a, L"συνεχές", L"συνεχ");

  // ending with -ης, -ες/-εις, -η
  checkOneTerm(a, L"συνήθης", L"συνηθ");
  checkOneTerm(a, L"συνήθους", L"συνηθ");
  checkOneTerm(a, L"συνήθη", L"συνηθ");
  // note: doesn't conflate
  checkOneTerm(a, L"συνήθεις", L"συν");
  checkOneTerm(a, L"συνήθων", L"συνηθ");
  checkOneTerm(a, L"σύνηθες", L"συνηθ");

  // ending with -υς, -υ/-εις, -ια
  checkOneTerm(a, L"βαθύς", L"βαθ");
  checkOneTerm(a, L"βαθέος", L"βαθε");
  checkOneTerm(a, L"βαθύ", L"βαθ");
  checkOneTerm(a, L"βαθείς", L"βαθ");
  checkOneTerm(a, L"βαθέων", L"βαθ");

  checkOneTerm(a, L"βαθιά", L"βαθ");
  checkOneTerm(a, L"βαθιάς", L"βαθι");
  checkOneTerm(a, L"βαθιές", L"βαθι");
  checkOneTerm(a, L"βαθιών", L"βαθ");

  checkOneTerm(a, L"βαθέα", L"βαθε");

  // comparative/superlative
  checkOneTerm(a, L"ψηλός", L"ψηλ");
  checkOneTerm(a, L"ψηλότερος", L"ψηλ");
  checkOneTerm(a, L"ψηλότατος", L"ψηλ");

  checkOneTerm(a, L"ωραίος", L"ωραι");
  checkOneTerm(a, L"ωραιότερος", L"ωραι");
  checkOneTerm(a, L"ωραιότατος", L"ωραι");

  checkOneTerm(a, L"επιεικής", L"επιεικ");
  checkOneTerm(a, L"επιεικέστερος", L"επιεικ");
  checkOneTerm(a, L"επιεικέστατος", L"επιεικ");
}

void TestGreekStemmer::testVerbs() 
{
  // note, past/present verb stems will not conflate (from the paper)
  //-ω,-α/-.ω,-.α
  checkOneTerm(a, L"ορίζω", L"οριζ");
  checkOneTerm(a, L"όριζα", L"οριζ");
  checkOneTerm(a, L"όριζε", L"οριζ");
  checkOneTerm(a, L"ορίζοντας", L"οριζ");
  checkOneTerm(a, L"ορίζομαι", L"οριζ");
  checkOneTerm(a, L"οριζόμουν", L"οριζ");
  checkOneTerm(a, L"ορίζεσαι", L"οριζ");

  checkOneTerm(a, L"όρισα", L"ορισ");
  checkOneTerm(a, L"ορίσω", L"ορισ");
  checkOneTerm(a, L"όρισε", L"ορισ");
  checkOneTerm(a, L"ορίσει", L"ορισ");

  checkOneTerm(a, L"ορίστηκα", L"οριστ");
  checkOneTerm(a, L"οριστώ", L"οριστ");
  checkOneTerm(a, L"οριστείς", L"οριστ");
  checkOneTerm(a, L"οριστεί", L"οριστ");

  checkOneTerm(a, L"ορισμένο", L"ορισμεν");
  checkOneTerm(a, L"ορισμένη", L"ορισμεν");
  checkOneTerm(a, L"ορισμένος", L"ορισμεν");

  // -ω,-α/-ξω,-ξα
  checkOneTerm(a, L"ανοίγω", L"ανοιγ");
  checkOneTerm(a, L"άνοιγα", L"ανοιγ");
  checkOneTerm(a, L"άνοιγε", L"ανοιγ");
  checkOneTerm(a, L"ανοίγοντας", L"ανοιγ");
  checkOneTerm(a, L"ανοίγομαι", L"ανοιγ");
  checkOneTerm(a, L"ανοιγόμουν", L"ανοιγ");

  checkOneTerm(a, L"άνοιξα", L"ανοιξ");
  checkOneTerm(a, L"ανοίξω", L"ανοιξ");
  checkOneTerm(a, L"άνοιξε", L"ανοιξ");
  checkOneTerm(a, L"ανοίξει", L"ανοιξ");

  checkOneTerm(a, L"ανοίχτηκα", L"ανοιχτ");
  checkOneTerm(a, L"ανοιχτώ", L"ανοιχτ");
  checkOneTerm(a, L"ανοίχτηκα", L"ανοιχτ");
  checkOneTerm(a, L"ανοιχτείς", L"ανοιχτ");
  checkOneTerm(a, L"ανοιχτεί", L"ανοιχτ");

  checkOneTerm(a, L"ανοίξου", L"ανοιξ");

  //-ώ/-άω,-ούσα/-άσω,-ασα
  checkOneTerm(a, L"περνώ", L"περν");
  checkOneTerm(a, L"περνάω", L"περν");
  checkOneTerm(a, L"περνούσα", L"περν");
  checkOneTerm(a, L"πέρναγα", L"περν");
  checkOneTerm(a, L"πέρνα", L"περν");
  checkOneTerm(a, L"περνώντας", L"περν");

  checkOneTerm(a, L"πέρασα", L"περασ");
  checkOneTerm(a, L"περάσω", L"περασ");
  checkOneTerm(a, L"πέρασε", L"περασ");
  checkOneTerm(a, L"περάσει", L"περασ");

  checkOneTerm(a, L"περνιέμαι", L"περν");
  checkOneTerm(a, L"περνιόμουν", L"περν");

  checkOneTerm(a, L"περάστηκα", L"περαστ");
  checkOneTerm(a, L"περαστώ", L"περαστ");
  checkOneTerm(a, L"περαστείς", L"περαστ");
  checkOneTerm(a, L"περαστεί", L"περαστ");

  checkOneTerm(a, L"περασμένο", L"περασμεν");
  checkOneTerm(a, L"περασμένη", L"περασμεν");
  checkOneTerm(a, L"περασμένος", L"περασμεν");

  // -ώ/-άω,-ούσα/-άξω,-αξα
  checkOneTerm(a, L"πετώ", L"πετ");
  checkOneTerm(a, L"πετάω", L"πετ");
  checkOneTerm(a, L"πετούσα", L"πετ");
  checkOneTerm(a, L"πέταγα", L"πετ");
  checkOneTerm(a, L"πέτα", L"πετ");
  checkOneTerm(a, L"πετώντας", L"πετ");
  checkOneTerm(a, L"πετιέμαι", L"πετ");
  checkOneTerm(a, L"πετιόμουν", L"πετ");

  checkOneTerm(a, L"πέταξα", L"πεταξ");
  checkOneTerm(a, L"πετάξω", L"πεταξ");
  checkOneTerm(a, L"πέταξε", L"πεταξ");
  checkOneTerm(a, L"πετάξει", L"πεταξ");

  checkOneTerm(a, L"πετάχτηκα", L"πεταχτ");
  checkOneTerm(a, L"πεταχτώ", L"πεταχτ");
  checkOneTerm(a, L"πεταχτείς", L"πεταχτ");
  checkOneTerm(a, L"πεταχτεί", L"πεταχτ");

  checkOneTerm(a, L"πεταμένο", L"πεταμεν");
  checkOneTerm(a, L"πεταμένη", L"πεταμεν");
  checkOneTerm(a, L"πεταμένος", L"πεταμεν");

  // -ώ/-άω,-ούσα / -έσω,-εσα
  checkOneTerm(a, L"καλώ", L"καλ");
  checkOneTerm(a, L"καλούσα", L"καλ");
  checkOneTerm(a, L"καλείς", L"καλ");
  checkOneTerm(a, L"καλώντας", L"καλ");

  checkOneTerm(a, L"καλούμαι", L"καλ");
  // pass. imperfect /imp. progressive doesnt conflate
  checkOneTerm(a, L"καλούμουν", L"καλουμ");
  checkOneTerm(a, L"καλείσαι", L"καλεισα");

  checkOneTerm(a, L"καλέστηκα", L"καλεστ");
  checkOneTerm(a, L"καλεστώ", L"καλεστ");
  checkOneTerm(a, L"καλεστείς", L"καλεστ");
  checkOneTerm(a, L"καλεστεί", L"καλεστ");

  checkOneTerm(a, L"καλεσμένο", L"καλεσμεν");
  checkOneTerm(a, L"καλεσμένη", L"καλεσμεν");
  checkOneTerm(a, L"καλεσμένος", L"καλεσμεν");

  checkOneTerm(a, L"φορώ", L"φορ");
  checkOneTerm(a, L"φοράω", L"φορ");
  checkOneTerm(a, L"φορούσα", L"φορ");
  checkOneTerm(a, L"φόραγα", L"φορ");
  checkOneTerm(a, L"φόρα", L"φορ");
  checkOneTerm(a, L"φορώντας", L"φορ");
  checkOneTerm(a, L"φοριέμαι", L"φορ");
  checkOneTerm(a, L"φοριόμουν", L"φορ");
  checkOneTerm(a, L"φοριέσαι", L"φορ");

  checkOneTerm(a, L"φόρεσα", L"φορεσ");
  checkOneTerm(a, L"φορέσω", L"φορεσ");
  checkOneTerm(a, L"φόρεσε", L"φορεσ");
  checkOneTerm(a, L"φορέσει", L"φορεσ");

  checkOneTerm(a, L"φορέθηκα", L"φορεθ");
  checkOneTerm(a, L"φορεθώ", L"φορεθ");
  checkOneTerm(a, L"φορεθείς", L"φορεθ");
  checkOneTerm(a, L"φορεθεί", L"φορεθ");

  checkOneTerm(a, L"φορεμένο", L"φορεμεν");
  checkOneTerm(a, L"φορεμένη", L"φορεμεν");
  checkOneTerm(a, L"φορεμένος", L"φορεμεν");

  // -ώ/-άω,-ούσα / -ήσω,-ησα
  checkOneTerm(a, L"κρατώ", L"κρατ");
  checkOneTerm(a, L"κρατάω", L"κρατ");
  checkOneTerm(a, L"κρατούσα", L"κρατ");
  checkOneTerm(a, L"κράταγα", L"κρατ");
  checkOneTerm(a, L"κράτα", L"κρατ");
  checkOneTerm(a, L"κρατώντας", L"κρατ");

  checkOneTerm(a, L"κράτησα", L"κρατ");
  checkOneTerm(a, L"κρατήσω", L"κρατ");
  checkOneTerm(a, L"κράτησε", L"κρατ");
  checkOneTerm(a, L"κρατήσει", L"κρατ");

  checkOneTerm(a, L"κρατούμαι", L"κρατ");
  checkOneTerm(a, L"κρατιέμαι", L"κρατ");
  // this imperfect form doesnt conflate
  checkOneTerm(a, L"κρατούμουν", L"κρατουμ");
  checkOneTerm(a, L"κρατιόμουν", L"κρατ");
  // this imp. prog form doesnt conflate
  checkOneTerm(a, L"κρατείσαι", L"κρατεισα");

  checkOneTerm(a, L"κρατήθηκα", L"κρατ");
  checkOneTerm(a, L"κρατηθώ", L"κρατ");
  checkOneTerm(a, L"κρατηθείς", L"κρατ");
  checkOneTerm(a, L"κρατηθεί", L"κρατ");
  checkOneTerm(a, L"κρατήσου", L"κρατ");

  checkOneTerm(a, L"κρατημένο", L"κρατημεν");
  checkOneTerm(a, L"κρατημένη", L"κρατημεν");
  checkOneTerm(a, L"κρατημένος", L"κρατημεν");

  // -.μαι,-.μουν / -.ώ,-.ηκα
  checkOneTerm(a, L"κοιμάμαι", L"κοιμ");
  checkOneTerm(a, L"κοιμόμουν", L"κοιμ");
  checkOneTerm(a, L"κοιμάσαι", L"κοιμ");

  checkOneTerm(a, L"κοιμήθηκα", L"κοιμ");
  checkOneTerm(a, L"κοιμηθώ", L"κοιμ");
  checkOneTerm(a, L"κοιμήσου", L"κοιμ");
  checkOneTerm(a, L"κοιμηθεί", L"κοιμ");

  checkOneTerm(a, L"κοιμισμένο", L"κοιμισμεν");
  checkOneTerm(a, L"κοιμισμένη", L"κοιμισμεν");
  checkOneTerm(a, L"κοιμισμένος", L"κοιμισμεν");
}

void TestGreekStemmer::testExceptions() 
{
  checkOneTerm(a, L"καθεστώτα", L"καθεστ");
  checkOneTerm(a, L"καθεστώτος", L"καθεστ");
  checkOneTerm(a, L"καθεστώς", L"καθεστ");
  checkOneTerm(a, L"καθεστώτων", L"καθεστ");

  checkOneTerm(a, L"χουμε", L"χουμ");
  checkOneTerm(a, L"χουμ", L"χουμ");

  checkOneTerm(a, L"υποταγεσ", L"υποταγ");
  checkOneTerm(a, L"υποταγ", L"υποταγ");

  checkOneTerm(a, L"εμετε", L"εμετ");
  checkOneTerm(a, L"εμετ", L"εμετ");

  checkOneTerm(a, L"αρχοντασ", L"αρχοντ");
  checkOneTerm(a, L"αρχοντων", L"αρχοντ");
}

void TestGreekStemmer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestGreekStemmer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestGreekStemmer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestGreekStemmer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<GreekStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::el