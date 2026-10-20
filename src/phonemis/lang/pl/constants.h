#pragma once

#include <phonemis/base/processor/layers/num2word/types.h>
#include <phonemis/base/processor/common/conditions.h>
#include <phonemis/base/processor/layers/num2word/notation.h>
#include <phonemis/base/processor/layers/term2word/types.h>
#include <phonemis/base/tokenizer/types.h>

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace phonemis::pl::constants {

// --- Sanitization Rules ---
namespace sanitizer {
  // Character replacements for postprocessing (e.g. ʑ → ʒ).
  inline const std::unordered_map<char32_t, char32_t> kCharReplacements = {
    {U'ʑ', U'ʒ'}
  };
} // namespace sanitizer

// --- Number to Word Normalization ---
namespace num2word {
  using ::phonemis::processor::num2word::Case;
  using ::phonemis::processor::num2word::DateFormat;
  using ::phonemis::processor::num2word::Joiners;
  using ::phonemis::processor::num2word::Keys;
  using ::phonemis::processor::num2word::keys_of;
  using ::phonemis::processor::num2word::Notation;
  using ::phonemis::processor::num2word::PhoneFormat;
  using ::phonemis::processor::num2word::SuffixFormat;

  // A noun's forms after a number: singular, then plural, each in the order of Case.
  struct Noun {
    std::array<std::u32string, 6> singular;
    std::array<std::u32string, 6> plural;
  };

  // A noun from its twelve forms, separated by spaces, or by "|" when they are made of several words.
  inline Noun noun(std::u32string_view forms) {
    char32_t separator = forms.find(U'|') != std::u32string_view::npos ? U'|' : U' ';
    Noun result;
    for (size_t i = 0, start = 0; i < 12; ++i) {
      size_t end = std::min(forms.find(separator, start), forms.size());
      (i < 6 ? result.singular[i] : result.plural[i - 6]) = forms.substr(start, end - start);
      start = end + 1;
    }
    return result;
  }

  // A regular masculine noun from its stem and its locative: metr, metrze.
  inline Noun masculine(std::u32string_view stem, std::u32string_view locative) {
    std::u32string s(stem);
    return {{s, s + U"a", s + U"owi", s, s + U"em", std::u32string(locative)},
            {s + U"y", s + U"ów", s + U"om", s + U"y", s + U"ami", s + U"ach"}};
  }

  // Cardinals: nominative/accusative, genitive/locative, dative, instrumental.
  inline const std::map<uint64_t, std::array<std::u32string_view, 4>> kCardinals = {
    {0,   {U"zero", U"zera", U"zeru", U"zerem"}},
    {2,   {U"dwa", U"dwóch", U"dwóm", U"dwoma"}},
    {3,   {U"trzy", U"trzech", U"trzem", U"trzema"}},
    {4,   {U"cztery", U"czterech", U"czterem", U"czterema"}},
    {5,   {U"pięć", U"pięciu", U"pięciu", U"pięcioma"}},
    {6,   {U"sześć", U"sześciu", U"sześciu", U"sześcioma"}},
    {7,   {U"siedem", U"siedmiu", U"siedmiu", U"siedmioma"}},
    {8,   {U"osiem", U"ośmiu", U"ośmiu", U"ośmioma"}},
    {9,   {U"dziewięć", U"dziewięciu", U"dziewięciu", U"dziewięcioma"}},
    {10,  {U"dziesięć", U"dziesięciu", U"dziesięciu", U"dziesięcioma"}},
    {11,  {U"jedenaście", U"jedenastu", U"jedenastu", U"jedenastoma"}},
    {12,  {U"dwanaście", U"dwunastu", U"dwunastu", U"dwunastoma"}},
    {13,  {U"trzynaście", U"trzynastu", U"trzynastu", U"trzynastoma"}},
    {14,  {U"czternaście", U"czternastu", U"czternastu", U"czternastoma"}},
    {15,  {U"piętnaście", U"piętnastu", U"piętnastu", U"piętnastoma"}},
    {16,  {U"szesnaście", U"szesnastu", U"szesnastu", U"szesnastoma"}},
    {17,  {U"siedemnaście", U"siedemnastu", U"siedemnastu", U"siedemnastoma"}},
    {18,  {U"osiemnaście", U"osiemnastu", U"osiemnastu", U"osiemnastoma"}},
    {19,  {U"dziewiętnaście", U"dziewiętnastu", U"dziewiętnastu", U"dziewiętnastoma"}},
    {20,  {U"dwadzieścia", U"dwudziestu", U"dwudziestu", U"dwudziestoma"}},
    {30,  {U"trzydzieści", U"trzydziestu", U"trzydziestu", U"trzydziestoma"}},
    {40,  {U"czterdzieści", U"czterdziestu", U"czterdziestu", U"czterdziestoma"}},
    {50,  {U"pięćdziesiąt", U"pięćdziesięciu", U"pięćdziesięciu", U"pięćdziesięcioma"}},
    {60,  {U"sześćdziesiąt", U"sześćdziesięciu", U"sześćdziesięciu", U"sześćdziesięcioma"}},
    {70,  {U"siedemdziesiąt", U"siedemdziesięciu", U"siedemdziesięciu", U"siedemdziesięcioma"}},
    {80,  {U"osiemdziesiąt", U"osiemdziesięciu", U"osiemdziesięciu", U"osiemdziesięcioma"}},
    {90,  {U"dziewięćdziesiąt", U"dziewięćdziesięciu", U"dziewięćdziesięciu", U"dziewięćdziesięcioma"}},
    {100, {U"sto", U"stu", U"stu", U"stoma"}},
    {200, {U"dwieście", U"dwustu", U"dwustu", U"dwustoma"}},
    {300, {U"trzysta", U"trzystu", U"trzystu", U"trzystoma"}},
    {400, {U"czterysta", U"czterystu", U"czterystu", U"czterystoma"}},
    {500, {U"pięćset", U"pięciuset", U"pięciuset", U"pięciuset"}},
    {600, {U"sześćset", U"sześciuset", U"sześciuset", U"sześciuset"}},
    {700, {U"siedemset", U"siedmiuset", U"siedmiuset", U"siedmiuset"}},
    {800, {U"osiemset", U"ośmiuset", U"ośmiuset", U"ośmiuset"}},
    {900, {U"dziewięćset", U"dziewięciuset", U"dziewięciuset", U"dziewięciuset"}}
  };

  // "1" by gender (masculine, feminine, neuter), each in the order of Case.
  inline const std::array<std::array<std::u32string_view, 6>, 3> kOne = {{
    {U"jeden", U"jednego", U"jednemu", U"jeden", U"jednym", U"jednym"},
    {U"jedna", U"jednej", U"jednej", U"jedną", U"jedną", U"jednej"},
    {U"jedno", U"jednego", U"jednemu", U"jedno", U"jednym", U"jednym"}
  }};

  // Stems of ordinals, which take adjective endings: pierwsz-y, drug-iego.
  inline const std::map<uint64_t, std::u32string_view> kOrdinalStems = {
    {1, U"pierwsz"},  {2, U"drug"},    {3, U"trzec"},    {4, U"czwart"},   {5, U"piąt"},
    {6, U"szóst"},    {7, U"siódm"},   {8, U"ósm"},      {9, U"dziewiąt"}, {10, U"dziesiąt"},
    {11, U"jedenast"}, {12, U"dwunast"}, {13, U"trzynast"}, {14, U"czternast"}, {15, U"piętnast"},
    {16, U"szesnast"}, {17, U"siedemnast"}, {18, U"osiemnast"}, {19, U"dziewiętnast"}, {20, U"dwudziest"},
    {30, U"trzydziest"}, {40, U"czterdziest"}, {50, U"pięćdziesiąt"}, {60, U"sześćdziesiąt"},
    {70, U"siedemdziesiąt"}, {80, U"osiemdziesiąt"}, {90, U"dziewięćdziesiąt"},
    {100, U"setn"}, {200, U"dwusetn"}, {300, U"trzechsetn"}, {400, U"czterechsetn"}, {500, U"pięćsetn"},
    {600, U"sześćsetn"}, {700, U"siedemsetn"}, {800, U"osiemsetn"}, {900, U"dziewięćsetn"}
  };

  // Adjective endings by gender (masculine, feminine, neuter, plural), each in the order of Case.
  inline const std::array<std::array<std::u32string_view, 6>, 4> kAdjectiveEndings = {{
    {U"y", U"ego", U"emu", U"y", U"ym", U"ym"},
    {U"a", U"ej", U"ej", U"ą", U"ą", U"ej"},
    {U"e", U"ego", U"emu", U"e", U"ym", U"ym"},
    {U"e", U"ych", U"ym", U"e", U"ymi", U"ych"}
  }};

  // Forms of numbers inside compounds: 45-metrowe -> czterdziestopięciometrowe.
  inline const std::map<uint64_t, std::u32string_view> kCombining = {
    {1, U"jedno"},     {2, U"dwu"},        {3, U"trzy"},        {4, U"cztero"},      {5, U"pięcio"},
    {6, U"sześcio"},   {7, U"siedmio"},    {8, U"ośmio"},       {9, U"dziewięcio"},  {10, U"dziesięcio"},
    {11, U"jedenasto"}, {12, U"dwunasto"}, {13, U"trzynasto"},  {14, U"czternasto"}, {15, U"piętnasto"},
    {16, U"szesnasto"}, {17, U"siedemnasto"}, {18, U"osiemnasto"}, {19, U"dziewiętnasto"}, {20, U"dwudziesto"},
    {30, U"trzydziesto"}, {40, U"czterdziesto"}, {50, U"pięćdziesięcio"}, {60, U"sześćdziesięcio"},
    {70, U"siedemdziesięcio"}, {80, U"osiemdziesięcio"}, {90, U"dziewięćdziesięcio"},
    {100, U"stu"}, {200, U"dwustu"}, {300, U"trzystu"}, {400, U"czterystu"}, {500, U"pięćset"},
    {600, U"sześćset"}, {700, U"siedemset"}, {800, U"osiemset"}, {900, U"dziewięćset"}
  };

  // Hyphenated endings which make a number an ordinal (lat 70-tych), with the adjective ending they stand for.
  inline const std::unordered_map<std::u32string, std::u32string_view> kOrdinalSuffixes = {
    {U"tych", U"ych"}, {U"ych", U"ych"}, {U"ty", U"y"},    {U"y", U"y"},     {U"szy", U"y"},    {U"gi", U"y"},
    {U"ci", U"y"},     {U"go", U"ego"},  {U"ego", U"ego"}, {U"tego", U"ego"}, {U"ej", U"ej"},    {U"tej", U"ej"},
    {U"ym", U"ym"},    {U"tym", U"ym"},  {U"ta", U"a"},    {U"a", U"a"},     {U"te", U"e"},     {U"e", U"e"},
    {U"mu", U"emu"},   {U"temu", U"emu"}
  };

  // Nouns counted by numbers
  inline const Noun kThousand = noun(U"tysiąc tysiąca tysiącowi tysiąc tysiącem tysiącu tysiące tysięcy tysiącom tysiące tysiącami tysiącach");
  inline const Noun kMillion = masculine(U"milion", U"milionie");
  inline const Noun kBillion = masculine(U"miliard", U"miliardzie");
  inline const Noun kZloty = noun(U"złoty złotego złotemu złoty złotym złotym złote złotych złotym złote złotymi złotych");
  inline const Noun kGrosz = noun(U"grosz grosza groszowi grosz groszem groszu grosze groszy groszom grosze groszami groszach");
  inline const Noun kPercent = noun(U"procent procent procentowi procent procentem procencie procent procent procentom procent procentami procentach");
  inline const Noun kEuro = noun(U"euro euro euro euro euro euro euro euro euro euro euro euro");
  inline const Noun kDollar = masculine(U"dolar", U"dolarze");
  inline constexpr std::u32string_view kDegree = U"stopień stopnia stopniowi stopień stopniem stopniu stopnie stopni stopniom stopnie stopniami stopniach";

  inline const std::unordered_map<std::u32string, Noun> kCurrencies = {
    {U"zł", kZloty}, {U"PLN", kZloty}, {U"€", kEuro}, {U"EUR", kEuro}, {U"$", kDollar}, {U"USD", kDollar},
    {U"£", masculine(U"funt", U"funcie")}, {U"gr", kGrosz}
  };

  inline const std::unordered_map<std::u32string, Noun> kScales = {
    {U"tys.", kThousand}, {U"mln", kMillion}, {U"mld", kBillion}
  };

  inline const std::unordered_map<std::u32string, Noun> kUnits = {
    {U"km", masculine(U"kilometr", U"kilometrze")},   {U"m", masculine(U"metr", U"metrze")},
    {U"cm", masculine(U"centymetr", U"centymetrze")}, {U"mm", masculine(U"milimetr", U"milimetrze")},
    {U"nm", masculine(U"nanometr", U"nanometrze")},   {U"kg", masculine(U"kilogram", U"kilogramie")},
    {U"g", masculine(U"gram", U"gramie")},            {U"mg", masculine(U"miligram", U"miligramie")},
    {U"l", masculine(U"litr", U"litrze")},            {U"ml", masculine(U"mililitr", U"mililitrze")},
    {U"ha", masculine(U"hektar", U"hektarze")},       {U"W", masculine(U"wat", U"wacie")},
    {U"kW", masculine(U"kilowat", U"kilowacie")},     {U"MW", masculine(U"megawat", U"megawacie")},
    {U"V", masculine(U"wolt", U"wolcie")},            {U"GB", masculine(U"gigabajt", U"gigabajcie")},
    {U"MB", masculine(U"megabajt", U"megabajcie")},   {U"TB", masculine(U"terabajt", U"terabajcie")},
    {U"kWh", noun(U"kilowatogodzina kilowatogodziny kilowatogodzinie kilowatogodzinę kilowatogodziną kilowatogodzinie kilowatogodziny kilowatogodzin kilowatogodzinom kilowatogodziny kilowatogodzinami kilowatogodzinach")},
    {U"Hz", noun(U"herc herca hercowi herc hercem hercu herce herców hercom herce hercami hercach")},
    {U"°C", noun(U"stopień Celsjusza|stopnia Celsjusza|stopniowi Celsjusza|stopień Celsjusza|stopniem Celsjusza|stopniu Celsjusza|stopnie Celsjusza|stopni Celsjusza|stopniom Celsjusza|stopnie Celsjusza|stopniami Celsjusza|stopniach Celsjusza")},
    {U"°", noun(kDegree)},
    {U"m²", noun(U"metr kwadratowy|metra kwadratowego|metrowi kwadratowemu|metr kwadratowy|metrem kwadratowym|metrze kwadratowym|metry kwadratowe|metrów kwadratowych|metrom kwadratowym|metry kwadratowe|metrami kwadratowymi|metrach kwadratowych")},
    {U"t", noun(U"tona tony tonie tonę toną tonie tony ton tonom tony tonami tonach")},
    {U"h", noun(U"godzina godziny godzinie godzinę godziną godzinie godziny godzin godzinom godziny godzinami godzinach")},
    {U"min", noun(U"minuta minuty minucie minutę minutą minucie minuty minut minutom minuty minutami minutach")},
    {U"s", noun(U"sekunda sekundy sekundzie sekundę sekundą sekundzie sekundy sekund sekundom sekundy sekundami sekundach")}
  };

  // Months in the genitive, as dates read them: 27 marca
  inline const std::array<std::u32string_view, 12> kMonthsGenitive = {
    U"stycznia", U"lutego", U"marca", U"kwietnia", U"maja", U"czerwca",
    U"lipca", U"sierpnia", U"września", U"października", U"listopada", U"grudnia"
  };

  inline const std::unordered_set<std::u32string> kMonths = {
    U"stycznia", U"lutego", U"marca", U"kwietnia", U"maja", U"czerwca", U"lipca", U"sierpnia", U"września",
    U"października", U"listopada", U"grudnia", U"styczeń", U"luty", U"marzec", U"kwiecień", U"maj", U"czerwiec",
    U"lipiec", U"sierpień", U"wrzesień", U"październik", U"listopad", U"grudzień"
  };

  // The case a preposition gives the number after it: od pięciu, z trzema
  inline const std::unordered_map<std::u32string, Case> kPrepositionCases = {
    {U"od", Case::GENITIVE}, {U"do", Case::GENITIVE}, {U"bez", Case::GENITIVE}, {U"dla", Case::GENITIVE},
    {U"u", Case::GENITIVE}, {U"około", Case::GENITIVE}, {U"koło", Case::GENITIVE}, {U"obok", Case::GENITIVE},
    {U"podczas", Case::GENITIVE}, {U"według", Case::GENITIVE}, {U"wśród", Case::GENITIVE},
    {U"spośród", Case::GENITIVE}, {U"sprzed", Case::GENITIVE}, {U"spod", Case::GENITIVE}, {U"zza", Case::GENITIVE},
    {U"znad", Case::GENITIVE}, {U"oprócz", Case::GENITIVE}, {U"prócz", Case::GENITIVE}, {U"zamiast", Case::GENITIVE},
    {U"wokół", Case::GENITIVE}, {U"dookoła", Case::GENITIVE}, {U"naprzeciwko", Case::GENITIVE},
    {U"blisko", Case::GENITIVE}, {U"niedaleko", Case::GENITIVE}, {U"powyżej", Case::GENITIVE},
    {U"poniżej", Case::GENITIVE},
    {U"dzięki", Case::DATIVE}, {U"przeciw", Case::DATIVE}, {U"przeciwko", Case::DATIVE}, {U"ku", Case::DATIVE},
    {U"wbrew", Case::DATIVE},
    {U"przez", Case::ACCUSATIVE}, {U"ponad", Case::ACCUSATIVE}, {U"na", Case::ACCUSATIVE}, {U"o", Case::ACCUSATIVE},
    {U"za", Case::ACCUSATIVE}, {U"po", Case::ACCUSATIVE}, {U"w", Case::ACCUSATIVE}, {U"we", Case::ACCUSATIVE},
    {U"z", Case::INSTRUMENTAL}, {U"ze", Case::INSTRUMENTAL}, {U"nad", Case::INSTRUMENTAL},
    {U"pod", Case::INSTRUMENTAL}, {U"przed", Case::INSTRUMENTAL}, {U"między", Case::INSTRUMENTAL},
    {U"pomiędzy", Case::INSTRUMENTAL},
    {U"przy", Case::LOCATIVE}
  };

  inline const std::unordered_set<std::u32string> kLocativePrepositions = {U"w", U"we", U"o", U"po", U"przy", U"na"};

  // Prepositional phrases which govern the genitive, with the preposition they need: w ciągu dwóch tygodni
  inline const std::unordered_map<std::u32string, std::u32string> kComplexPrepositions = {
    {U"ciągu", U"w"}, {U"trakcie", U"w"}, {U"czasie", U"w"}, {U"wyniku", U"w"}, {U"przypadku", U"w"},
    {U"zakresie", U"w"}, {U"ramach", U"w"}, {U"celu", U"w"}, {U"postaci", U"w"}, {U"powodu", U"z"}
  };

  // Stems of amount nouns: a number after them is their genitive attribute (od kwoty trzystu tysięcy)
  inline const std::vector<std::u32string> kAmountNouns = {
    U"rzęd", U"wysokoś", U"gruboś", U"średnic", U"długoś", U"szerokoś", U"głębokoś", U"wiek", U"kwot", U"kwoc",
    U"poziom", U"stan", U"strat", U"strac", U"wartoś", U"liczb", U"cen", U"koszt", U"opłat", U"kar", U"moc",
    U"wag", U"wadz", U"pojemnoś", U"powierzchn", U"prędkoś", U"temperatur", U"objętoś", U"odległoś", U"armi",
    U"grup", U"okres", U"limit", U"próg", U"prog", U"budżet", U"deficyt", U"nadwyżk", U"dochod", U"dochód",
    U"przychod", U"przychód", U"wydatk", U"wzrost", U"spad", U"zysk", U"dotacj", U"wynagrodzen", U"pensj"
  };

  inline const std::unordered_set<std::u32string> kNounEndings = {
    U"", U"a", U"ą", U"ę", U"y", U"i", U"u", U"e", U"ie", U"em", U"iem", U"om", U"ów", U"ami", U"ach", U"ć", U"ci",
    U"cią", U"ze", U"ia", U"ię", U"ii", U"ek", U"ku"
  };

  // Stems of verbs whose object is in the genitive: potrzebowali stu czterdziestu gramów
  inline const std::vector<std::u32string> kGenitiveVerbs = {
    U"potrzeb", U"używ", U"użyj", U"użyć", U"wymag", U"szuk", U"poszuk", U"unik", U"brak", U"zabrak", U"życz",
    U"dotycz", U"uczy", U"nauczy", U"słucha", U"pilnu"
  };

  // Words skipped when looking for the verb governing a number: brakuje nam trzech osób
  inline const std::unordered_set<std::u32string> kClitics = {
    U"mi", U"ci", U"mu", U"jej", U"nam", U"wam", U"im", U"się", U"sobie", U"już", U"jeszcze", U"teraz", U"tu",
    U"tam", U"aż", U"nawet"
  };

  inline const std::unordered_set<std::u32string> kCopulas = {
    U"jest", U"są", U"był", U"była", U"było", U"były", U"będzie", U"będą", U"to"
  };

  // Prepositions before a bare year (od 2020 do 2022), with the case of the year ordinal after them
  inline const std::unordered_map<std::u32string, Case> kYearPrepositions = {
    {U"od", Case::GENITIVE}, {U"do", Case::GENITIVE}, {U"z", Case::GENITIVE}, {U"ze", Case::GENITIVE},
    {U"sprzed", Case::GENITIVE}, {U"około", Case::GENITIVE}, {U"przed", Case::INSTRUMENTAL},
    {U"po", Case::LOCATIVE}, {U"na", Case::ACCUSATIVE}
  };

  inline const std::unordered_set<std::u32string> kCountNouns = {
    U"osób", U"lat", U"dni", U"godzin", U"minut", U"sekund", U"razy", U"sztuk", U"ludzi", U"dzieci", U"tygodni",
    U"miesięcy"
  };

  inline const std::unordered_set<std::u32string> kCurrencyWords = {U"zł", U"złoty", U"złote", U"złotych"};

  // Stems of nouns whose numbers are feminine: dwie osoby, dwie godziny
  inline const std::vector<std::u32string> kFemininePlurals = {
    U"osob", U"osób", U"godzin", U"minut", U"sekund", U"złotówk", U"kobiet", U"stron", U"sztuk", U"rzecz", U"częś",
    U"noc", U"dob", U"klas", U"wersj", U"sesj", U"edycj", U"zapał", U"tablic", U"butelk", U"szklank", U"łyżk"
  };

  // Words before a time with a dot, which make it a time: o 18.30
  inline const std::unordered_set<std::u32string> kTimeWords = {
    U"o", U"godzinie", U"godz", U"od", U"do", U"około", U"koło", U"po", U"przed", U"na", U"między", U"za"
  };

  // Stems of headings: Punkt 3., w punkcie 5.; the ones of kOrdinalHeadings read as ordinals even without a dot
  inline const std::vector<std::u32string> kHeadings = {
    U"podpunkt", U"punkt", U"punkc", U"rozdział", U"paragraf", U"artykuł", U"pkt"
  };
  inline const std::vector<std::u32string> kOrdinalHeadings = {U"podpunkt", U"punkt", U"punkc", U"paragraf", U"pkt"};

  // Words before the ordinal of a decade, with its case: z lat 70., w latach 80.
  inline const std::unordered_map<std::u32string, Case> kDecadeWords = {
    {U"lata", Case::NOMINATIVE},     {U"lat", Case::GENITIVE},       {U"latach", Case::LOCATIVE},
    {U"latami", Case::INSTRUMENTAL}, {U"roczników", Case::GENITIVE}, {U"rocznikach", Case::LOCATIVE}
  };

  inline const std::unordered_set<std::u32string> kConjunctions = {U"a", U"i", U"lub", U"albo", U"czy", U"oraz"};

  // --- Notation: how numbers are written ---
  inline const Notation kNotation = {
    .group_separators = U" \u00a0\u202f.", .decimal_separators = U",.", .time_separators = U":.",
    .currencies = keys_of(kCurrencies), .scales = keys_of(kScales), .units = keys_of(kUnits)
  };

  // Words between numbers: 3.8.10, 10.0.0.51, 10-15, 1920x1080, 2:1, -5, 2 + 2
  inline const Joiners kJoiners = {
    .point = U"kropka", .dot = U"kropka", .range = U"-", .by = U"na", .score = U" do ", .minus = U"minus", .plus = U"plus"
  };

  // 15.08.2020 r.: the year has four digits, "r." may follow it
  inline const DateFormat kDateFormat = {.separators = U".", .boundaries = U".,", .short_year = false, .suffixes = {U"r."}};

  // 600 123 456, 22 123 45 67 (after a word of kPhoneWords), 600-123-456, 68-324-22-55
  inline const std::vector<PhoneFormat> kPhoneFormats = {
    {.groups = {{3, 3}, {3, 3}, {3, 3}}, .separators = U" ", .boundaries = U"", .closing = U""},
    {.groups = {{2, 2}, {3, 3}, {2, 2}, {2, 2}}, .separators = U" ", .boundaries = U"", .closing = U""},
    {.groups = {{3, 3}, {3, 3}, {3, 3}}, .separators = U"-", .boundaries = U".,-", .closing = U"-"},
    {.groups = {{2, 2}, {3, 3}, {2, 2}, {2, 2}}, .separators = U"-", .boundaries = U".,-", .closing = U"-"}
  };

  // Words before a phone number written with spaces, which else reads as a count: numer 600 123 456
  inline const std::unordered_set<std::u32string> kPhoneWords = {
    U"numer", U"numerem", U"numeru", U"tel", U"telefon", U"telefonu", U"telefonem", U"infolinia", U"faks", U"fax"
  };

  // 2020 r.: a year with "roku" after it
  inline const Keys kYearSuffixes = {U"r."};
  inline const SuffixFormat kYearFormat = {.space = true, .sentence_dot = true};
} // namespace num2word

// --- Symbols to Word Normalization ---
namespace sym2word {
  using ::phonemis::processor::Readings;

  inline const Readings kSymbols = {
    {U"&", {U"and"}},        {U"+", {U"plus"}},           {U"=", {U"równa się"}}, {U"%", {U"procent"}},  {U"°", {U"stopni"}},
    {U"#", {U""}},           {U"/", {U""}},               {U"²", {U"do kwadratu"}}, {U"³", {U"do sześcianu"}},
    {U"π", {U"pi"}},         {U"×", {U"razy"}},           {U"€", {U"euro"}},     {U"$", {U"dolarów"}},   {U"@", {U"małpa"}}
  };
} // namespace sym2word

// --- Terms to Word Normalization ---
namespace term2word {
  using ::phonemis::processor::before_number;
  using ::phonemis::processor::term2word::Terms;

  inline const Terms kTerms = {
    .abbreviations = {
      // Before a number only: nr 5
      {U"nr", {{U"numer", before_number}}},
      {U"Nr", {{U"numer", before_number}}},
      {U"nr.", {{U"numer", before_number}}},
      {U"ust.", {{U"ustęp", before_number}}},
      {U"pkt", {{U"punkt", before_number}}},
      {U"art.", {{U"artykuł", before_number}}},
      {U"str.", {{U"strona", before_number}}},
      {U"poz.", {{U"pozycja", before_number}}},
      {U"ok.", {{U"około", before_number}}},
      // Anywhere
      {U"np.", {U"na przykład"}},
      {U"itd.", {U"i tak dalej"}},
      {U"itp.", {U"i tym podobne"}},
      {U"tzn.", {U"to znaczy"}},
      {U"tj.", {U"to jest"}},
      {U"m.in.", {U"między innymi"}},
      {U"wg", {U"według"}},
    },
    .web = {.dot = U"kropka", .at = U"małpa", .slash = U"ukośnik"}
  };
} // namespace term2word

// --- Tokenization Rules ---
namespace tokenizer {
  using ::phonemis::tokenizer::split::Rule;
  using ::phonemis::tokenizer::split::Exceptions;

  inline const std::unordered_map<char32_t, Rule> kSpecialCharacters = {
    {U'\'', Rule::KEEP_WITH_RIGHT},
    {U'-',  Rule::TOTAL_DIVIDE},
    {U'.',  Rule::TOTAL_DIVIDE},
    {U':',  Rule::TOTAL_DIVIDE}
  };

  inline const Exceptions kExceptions = {};
} // namespace tokenizer

} // namespace phonemis::pl::constants
