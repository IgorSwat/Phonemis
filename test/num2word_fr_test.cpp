#include "processing.h"
#include "test.h"
#include <phonemis/lang/fr/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// French readings follow the Echo verbalizer; ISO dates are read by phonemis only.
REGISTER_TEST(num2word_fr_test) {
  fr::Num2Word layer;

  ASSERT_EQUALS(U"Le premier janvier deux mille vingt à dix-huit heures trente, vingt et une personnes, trois euros cinquante.",
                apply(layer, U"Le 1er janvier 2020 à 18h30, 21 personnes, 3,50 €."));
  ASSERT_EQUALS(U"Au vingtième siècle, au zéro six douze trente-quatre cinquante-six soixante-dix-huit.",
                apply(layer, U"Au XXe siècle, au 06 12 34 56 78."));
  ASSERT_EQUALS(U"Elle a vingt et un ans et trente et une heures de travail.",
                apply(layer, U"Elle a 21 ans et 31 heures de travail."));
  ASSERT_EQUALS(U"Les années quatre-vingt et deux cent mille habitants, quatre-vingts pages.",
                apply(layer, U"Les années 80 et 200 000 habitants, 80 pages."));
  ASSERT_EQUALS(U"Le cinq novembre et le trois quarts du gâteau, puis un demi litre.",
                apply(layer, U"Le 05/11 et le 3/4 du gâteau, puis 1/2 litre."));
  ASSERT_EQUALS(U"Rendez-vous à dix-huit heures trente ou quatorze heures, pas un virgule treize mètres.",
                apply(layer, U"Rendez-vous à 18.30 ou 14h, pas 1.13 mètres."));
  ASSERT_EQUALS(U"Il a gagné trois millions d'euros et dix mille abonnés, quatre mille vidéo.",
                apply(layer, U"Il a gagné 3 millions d'euros et 10k abonnés, 4K vidéo."));
  ASSERT_EQUALS(U"Une pièce de douze fois quatre = quarante-huit et un écran de mille neuf cent vingt par mille quatre-vingts.",
                apply(layer, U"Une pièce de 12 x 4 = 48 et un écran de 1920x1080."));
  ASSERT_EQUALS(U"Il est arrivé deuxième, puis première et seconde, les troisièmes.",
                apply(layer, U"Il est arrivé 2e, puis 1re et 2nde, les 3èmes."));
  ASSERT_EQUALS(U"Il fait moins cinq degrés Celsius, vingt-cinq kilomètres par heure et un virgule cinq kilogramme de farine.",
                apply(layer, U"Il fait -5 °C, 25 km/h et 1,5 kg de farine."));
  ASSERT_EQUALS(U"soixante-quinze centimes et quatre-vingt-dix-neuf centimes, une livre et deux virgule cinq livres, trois millions d'euros.",
                apply(layer, U"0,75 € et 99 centimes, 1 £ et 2,5 £, 3M€."));
  ASSERT_EQUALS(U"Le quatorze juillet mille sept cent quatre-vingt-neuf et le premier mai deux mille vingt-quatre.",
                apply(layer, U"Le 14 juillet 1789 et le 1.5.2024."));
  ASSERT_EQUALS(U"Il a cent un ans et soixante et onze euros, quatre-vingt-onze pommes, quatre-vingt-une voitures.",
                apply(layer, U"Il a 101 ans et 71 euros, 91 pommes, 81 voitures."));
  ASSERT_EQUALS(U"Il était quatorze heures vingt ce mercredi.",
                apply(layer, U"Il était 14 h 20 ce mercredi."));

  return true;
}

} // namespace phonemis::test
