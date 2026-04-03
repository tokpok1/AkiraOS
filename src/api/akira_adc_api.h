/*
 * Copyright (c) 2025 AkiraOS Contributors
 * SPDX-License-Identifier: GPL-3.0-only
 */

/**
 * @file akira_adc_api.h
 * @brief Raw ADC read API for WASM applications
 *
 * Stateless API - no handles. WASM passes adc_id + channel per call.
 * ADC controllers are resolved from the device tree on demand.
 */

#ifndef AKIRA_ADC_API_H
#define AKIRA_ADC_API_H

#include <stdint.h>

#ifdef CONFIG_AKIRA_WASM_RUNTIME
#include <wasm_export.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_AKIRA_WASM_RUNTIME

/**
 * @brief Read value from an ADC channel.
 *
 * @param adc_id  ADC controller ID (0 = adc0 or adc1 fallback, 1 = adc1).
 * @param channel ADC channel number.
 * @param value   Pointer to destination integer in WASM linear memory.
 *
 * @return 0 on success, negative Zephyr errno on failure.
 */
int akira_native_adc_read(wasm_exec_env_t exec_env,
                          int32_t adc_id,
                          int32_t channel,
                          int32_t *value);

#endif /* CONFIG_AKIRA_WASM_RUNTIME */

#ifdef __cplusplus
}
#endif

#endif /* AKIRA_ADC_API_H */