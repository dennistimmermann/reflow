/*
 * Variant implementation for the Reflow Oven Controller board.
 * STM32G0B1CET6, LQFP-48 — 64 MHz from HSI16 + PLL, no external crystal.
 */
#include "pins_arduino.h"

// Digital pin → HAL PinName mapping.
// Order must match the #define indices in variant.h.
const PinName digitalPin[] = {
  PA_0,    // D0/A0
  PA_1,    // D1/A1
  PA_2,    // D2/A2
  PA_3,    // D3/A3
  PA_4,    // D4/A4
  PA_5,    // D5/A5
  PA_6,    // D6/A6
  PA_7,    // D7/A7
  PA_8,    // D8
  PA_9,    // D9
  PA_10,   // D10
  PA_11,   // D11
  PA_12,   // D12
  PA_13,   // D13
  PA_14,   // D14
  PA_15,   // D15
  PB_0,    // D16/A8
  PB_1,    // D17/A9
  PB_2,    // D18/A10
  PB_3,    // D19
  PB_4,    // D20
  PB_5,    // D21
  PB_6,    // D22
  PB_7,    // D23
  PB_8,    // D24
  PB_9,    // D25
  PB_10,   // D26/A11
  PB_11,   // D27/A12
  PB_12,   // D28/A13
  PB_13,   // D29
  PB_14,   // D30
  PB_15,   // D31
  PC_6,    // D32
  PC_7,    // D33
  PC_13,   // D34
  PC_14,   // D35
  PC_15,   // D36
  PD_0,    // D37
  PD_1,    // D38
  PD_2,    // D39
  PD_3,    // D40
  PF_0,    // D41
  PF_1,    // D42
  PF_2,    // D43
  PA_9_R,  // D44
  PA_10_R  // D45
};

// Analog input index → digital pin index mapping.
const uint32_t analogInputPin[] = {
  0,   // A0  PA0
  1,   // A1  PA1
  2,   // A2  PA2
  3,   // A3  PA3
  4,   // A4  PA4
  5,   // A5  PA5
  6,   // A6  PA6
  7,   // A7  PA7
  16,  // A8  PB0
  17,  // A9  PB1
  18,  // A10 PB2
  26,  // A11 PB10
  27,  // A12 PB11
  28   // A13 PB12
};

bool boot0_pin_enabled() {
  return (FLASH->OPTR & FLASH_OPTR_nBOOT_SEL_Msk) == 0;
}

void enable_boot0_pin() {
  HAL_FLASH_Unlock();
  HAL_FLASH_OB_Unlock();
  FLASH_OBProgramInitTypeDef ob = {};
  ob.OptionType = OPTIONBYTE_USER;
  ob.USERType   = OB_USER_nBOOT_SEL;
  ob.USERConfig = OB_BOOT0_FROM_PIN;  // clears nBOOT_SEL to 0 → BOOT0 from PA14 pin
  HAL_FLASHEx_OBProgram(&ob);
  HAL_FLASH_OB_Launch();              // triggers system reset; does not return
}

void initVariant() {
  if (!boot0_pin_enabled()) enable_boot0_pin();
}

// Clock: HSI16 → PLL (M=1, N=8, R=2) → 64 MHz SYSCLK.
// HSI48 enabled for USB.  No external crystal (HSE not used).
WEAK void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSI48State          = RCC_HSI48_ON;
  RCC_OscInitStruct.HSIDiv              = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN            = 8;
  RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                   | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}
