#ifndef _BOARD_CONFIG_H
#define _BOARD_CONFIG_H

// PLLCLK = (N * source) / (M * P)
// 8MHz * 336 / (8 * 2) = 168MHz
#define PLL_M 8 // match HSE (8MHz)
#define PLL_N 336
#define PLL_P 2

#define LED_GREEN 12
#define LED_RED 14

#endif // _BOARD_CONFIG_H
