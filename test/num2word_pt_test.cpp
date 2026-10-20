#include "processing.h"
#include "test.h"
#include <phonemis/lang/pt/num2word.h>
#include <phonemis/utils/io.h>

namespace phonemis::test {

// Portuguese readings follow the Echo verbalizer; ISO dates are read by phonemis only.
REGISTER_TEST(num2word_pt_test) {
  pt::Num2Word layer;

  ASSERT_EQUALS(U"No dia vinte e quatro de março de dois mil e vinte e dois e sete de novembro de dois mil e dezoito, às catorze e trinta e às vinte e uma horas.",
                apply(layer, U"No dia 24/03/2022 e 07-11-2018, às 14h30 e às 21h."));
  ASSERT_EQUALS(U"Ligue para zero oitocentos, um dois três quatro cinco seis ou três quatro cinco seis, sete oito nove zero, de dois mil e vinte e um a dois mil e vinte e dois.",
                apply(layer, U"Ligue para 0800-123456 ou 3456-7890, de 2021-2022."));
  ASSERT_EQUALS(U"São vinte e duas mil pessoas, dois mil reais e um vírgula cinco milhão de reais.",
                apply(layer, U"São 22 mil pessoas, 2 mil reais e R$ 1,5 milhão."));
  ASSERT_EQUALS(U"Custa vinte e nove reais e noventa centavos e um real e cinquenta centavos, trinta e sete mil quinhentos e dezessete reais e cinquenta reais.",
                apply(layer, U"Custa R$ 29,90 e R$ 1,00 e R$ 0,50, R$ 37,517 e R$ 50 reais."));
  ASSERT_EQUALS(U"Tem vinte e um anos, vinte e uma casas e primeiro de maio, às duas da tarde.",
                apply(layer, U"Tem 21 anos, 21 casas e 1 de maio, às 2 da tarde."));
  ASSERT_EQUALS(U"O primeiro e a segunda, artigo oitenta e o décimo primeiro lugar.",
                apply(layer, U"O 1º e a 2ª, artigo 80º e o 11º lugar."));
  ASSERT_EQUALS(U"Entre dois mil e dezesseis a dois mil e vinte e dois e dez a quinze quilômetros, o placar quatro-três-três e o A trezentos e vinte duzentos.",
                apply(layer, U"Entre 2016-2022 e 10-15 km, o placar 4-3-3 e o A320-200."));
  ASSERT_EQUALS(U"São três milhões de habitantes e um bilhão, quatro K e quinhentos mil seguidores.",
                apply(layer, U"São 3 mi de habitantes e 1 bi, 4K e 500k seguidores."));
  ASSERT_EQUALS(U"Subiu vinte e cinco por cento e três vírgula cinco por cento ao ano, um vírgula cinco quilômetro e dois quilos e uma grama.",
                apply(layer, U"Subiu 25% e 3,5% ao ano, 1,5 km e 2 kg e 1 g."));
  ASSERT_EQUALS(U"São um milhão de pessoas, dois mil e trezentos e um milhão e duzentos mil, dois mil e um e mil quinhentos e vinte.",
                apply(layer, U"São 1.000.000 de pessoas, 2.300 e 1.200.000, 2.001 e 1.520."));
  ASSERT_EQUALS(U"Às treze horas, nove horas e uma hora.",
                apply(layer, U"Às 13:00, 9h e 1h."));
  ASSERT_EQUALS(U"três bilhões de dólares e dois vírgula cinco milhões de euros, uma libra e duas libras.",
                apply(layer, U"US$ 3 bilhões e € 2,5 milhões, £ 1 e £ 2."));
  ASSERT_EQUALS(U"O evento de trinta de janeiro de dois mil e vinte e três.",
                apply(layer, U"O evento de 2023-01-30."));

  return true;
}

} // namespace phonemis::test
