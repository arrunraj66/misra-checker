#pragma once

#include <array>

#include "misra/rule.hpp"

namespace misra::c2012 {

RulePtr make_rule_01_01();
RulePtr make_rule_01_02();
RulePtr make_rule_01_03();
RulePtr make_rule_02_01();
RulePtr make_rule_02_02();
RulePtr make_rule_02_03();
RulePtr make_rule_02_04();
RulePtr make_rule_02_05();
RulePtr make_rule_02_06();
RulePtr make_rule_02_07();
RulePtr make_rule_03_01();
RulePtr make_rule_03_02();
RulePtr make_rule_04_01();
RulePtr make_rule_04_02();
RulePtr make_rule_05_01();
RulePtr make_rule_05_02();
RulePtr make_rule_05_03();
RulePtr make_rule_05_04();
RulePtr make_rule_05_05();
RulePtr make_rule_05_06();
RulePtr make_rule_05_07();
RulePtr make_rule_05_08();
RulePtr make_rule_05_09();
RulePtr make_rule_06_01();
RulePtr make_rule_06_02();
RulePtr make_rule_07_01();
RulePtr make_rule_07_02();
RulePtr make_rule_07_03();
RulePtr make_rule_07_04();
RulePtr make_rule_08_01();
RulePtr make_rule_08_02();
RulePtr make_rule_08_03();
RulePtr make_rule_08_04();
RulePtr make_rule_08_05();
RulePtr make_rule_08_06();
RulePtr make_rule_08_07();
RulePtr make_rule_08_08();
RulePtr make_rule_08_09();
RulePtr make_rule_08_10();
RulePtr make_rule_08_11();
RulePtr make_rule_08_12();
RulePtr make_rule_08_13();
RulePtr make_rule_08_14();
RulePtr make_rule_09_01();
RulePtr make_rule_09_02();
RulePtr make_rule_09_03();
RulePtr make_rule_09_04();
RulePtr make_rule_09_05();
RulePtr make_rule_10_01();
RulePtr make_rule_10_02();
RulePtr make_rule_10_03();
RulePtr make_rule_10_04();
RulePtr make_rule_10_05();
RulePtr make_rule_10_06();
RulePtr make_rule_10_07();
RulePtr make_rule_10_08();
RulePtr make_rule_11_01();
RulePtr make_rule_11_02();
RulePtr make_rule_11_03();
RulePtr make_rule_11_04();
RulePtr make_rule_11_05();
RulePtr make_rule_11_06();
RulePtr make_rule_11_07();
RulePtr make_rule_11_08();
RulePtr make_rule_11_09();
RulePtr make_rule_12_01();
RulePtr make_rule_12_02();
RulePtr make_rule_12_03();
RulePtr make_rule_12_04();
RulePtr make_rule_13_01();
RulePtr make_rule_13_02();
RulePtr make_rule_13_03();
RulePtr make_rule_13_04();
RulePtr make_rule_13_05();
RulePtr make_rule_13_06();
RulePtr make_rule_14_01();
RulePtr make_rule_14_02();
RulePtr make_rule_14_03();
RulePtr make_rule_14_04();
RulePtr make_rule_15_01();
RulePtr make_rule_15_02();
RulePtr make_rule_15_03();
RulePtr make_rule_15_04();
RulePtr make_rule_15_05();
RulePtr make_rule_15_06();
RulePtr make_rule_15_07();
RulePtr make_rule_16_01();
RulePtr make_rule_16_02();
RulePtr make_rule_16_03();
RulePtr make_rule_16_04();
RulePtr make_rule_16_05();
RulePtr make_rule_16_06();
RulePtr make_rule_16_07();
RulePtr make_rule_17_01();
RulePtr make_rule_17_02();
RulePtr make_rule_17_03();
RulePtr make_rule_17_04();
RulePtr make_rule_17_05();
RulePtr make_rule_17_06();
RulePtr make_rule_17_07();
RulePtr make_rule_17_08();
RulePtr make_rule_18_01();
RulePtr make_rule_18_02();
RulePtr make_rule_18_03();
RulePtr make_rule_18_04();
RulePtr make_rule_18_05();
RulePtr make_rule_18_06();
RulePtr make_rule_18_07();
RulePtr make_rule_18_08();
RulePtr make_rule_19_01();
RulePtr make_rule_19_02();
RulePtr make_rule_20_01();
RulePtr make_rule_20_02();
RulePtr make_rule_20_03();
RulePtr make_rule_20_04();
RulePtr make_rule_20_05();
RulePtr make_rule_20_06();
RulePtr make_rule_20_07();
RulePtr make_rule_20_08();
RulePtr make_rule_20_09();
RulePtr make_rule_20_10();
RulePtr make_rule_20_11();
RulePtr make_rule_20_12();
RulePtr make_rule_20_13();
RulePtr make_rule_20_14();
RulePtr make_rule_21_01();
RulePtr make_rule_21_02();
RulePtr make_rule_21_03();
RulePtr make_rule_21_04();
RulePtr make_rule_21_05();
RulePtr make_rule_21_06();
RulePtr make_rule_21_07();
RulePtr make_rule_21_08();
RulePtr make_rule_21_09();
RulePtr make_rule_21_10();
RulePtr make_rule_21_11();
RulePtr make_rule_21_12();
RulePtr make_rule_22_01();
RulePtr make_rule_22_02();
RulePtr make_rule_22_03();
RulePtr make_rule_22_04();
RulePtr make_rule_22_05();
RulePtr make_rule_22_06();

[[nodiscard]] inline std::array<RuleFactory, 143> rule_factories() noexcept {
  return {
      &make_rule_01_01,
      &make_rule_01_02,
      &make_rule_01_03,
      &make_rule_02_01,
      &make_rule_02_02,
      &make_rule_02_03,
      &make_rule_02_04,
      &make_rule_02_05,
      &make_rule_02_06,
      &make_rule_02_07,
      &make_rule_03_01,
      &make_rule_03_02,
      &make_rule_04_01,
      &make_rule_04_02,
      &make_rule_05_01,
      &make_rule_05_02,
      &make_rule_05_03,
      &make_rule_05_04,
      &make_rule_05_05,
      &make_rule_05_06,
      &make_rule_05_07,
      &make_rule_05_08,
      &make_rule_05_09,
      &make_rule_06_01,
      &make_rule_06_02,
      &make_rule_07_01,
      &make_rule_07_02,
      &make_rule_07_03,
      &make_rule_07_04,
      &make_rule_08_01,
      &make_rule_08_02,
      &make_rule_08_03,
      &make_rule_08_04,
      &make_rule_08_05,
      &make_rule_08_06,
      &make_rule_08_07,
      &make_rule_08_08,
      &make_rule_08_09,
      &make_rule_08_10,
      &make_rule_08_11,
      &make_rule_08_12,
      &make_rule_08_13,
      &make_rule_08_14,
      &make_rule_09_01,
      &make_rule_09_02,
      &make_rule_09_03,
      &make_rule_09_04,
      &make_rule_09_05,
      &make_rule_10_01,
      &make_rule_10_02,
      &make_rule_10_03,
      &make_rule_10_04,
      &make_rule_10_05,
      &make_rule_10_06,
      &make_rule_10_07,
      &make_rule_10_08,
      &make_rule_11_01,
      &make_rule_11_02,
      &make_rule_11_03,
      &make_rule_11_04,
      &make_rule_11_05,
      &make_rule_11_06,
      &make_rule_11_07,
      &make_rule_11_08,
      &make_rule_11_09,
      &make_rule_12_01,
      &make_rule_12_02,
      &make_rule_12_03,
      &make_rule_12_04,
      &make_rule_13_01,
      &make_rule_13_02,
      &make_rule_13_03,
      &make_rule_13_04,
      &make_rule_13_05,
      &make_rule_13_06,
      &make_rule_14_01,
      &make_rule_14_02,
      &make_rule_14_03,
      &make_rule_14_04,
      &make_rule_15_01,
      &make_rule_15_02,
      &make_rule_15_03,
      &make_rule_15_04,
      &make_rule_15_05,
      &make_rule_15_06,
      &make_rule_15_07,
      &make_rule_16_01,
      &make_rule_16_02,
      &make_rule_16_03,
      &make_rule_16_04,
      &make_rule_16_05,
      &make_rule_16_06,
      &make_rule_16_07,
      &make_rule_17_01,
      &make_rule_17_02,
      &make_rule_17_03,
      &make_rule_17_04,
      &make_rule_17_05,
      &make_rule_17_06,
      &make_rule_17_07,
      &make_rule_17_08,
      &make_rule_18_01,
      &make_rule_18_02,
      &make_rule_18_03,
      &make_rule_18_04,
      &make_rule_18_05,
      &make_rule_18_06,
      &make_rule_18_07,
      &make_rule_18_08,
      &make_rule_19_01,
      &make_rule_19_02,
      &make_rule_20_01,
      &make_rule_20_02,
      &make_rule_20_03,
      &make_rule_20_04,
      &make_rule_20_05,
      &make_rule_20_06,
      &make_rule_20_07,
      &make_rule_20_08,
      &make_rule_20_09,
      &make_rule_20_10,
      &make_rule_20_11,
      &make_rule_20_12,
      &make_rule_20_13,
      &make_rule_20_14,
      &make_rule_21_01,
      &make_rule_21_02,
      &make_rule_21_03,
      &make_rule_21_04,
      &make_rule_21_05,
      &make_rule_21_06,
      &make_rule_21_07,
      &make_rule_21_08,
      &make_rule_21_09,
      &make_rule_21_10,
      &make_rule_21_11,
      &make_rule_21_12,
      &make_rule_22_01,
      &make_rule_22_02,
      &make_rule_22_03,
      &make_rule_22_04,
      &make_rule_22_05,
      &make_rule_22_06,
  };
}

}  // namespace misra::c2012
