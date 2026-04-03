#define LOG_MODULE_NAME akira_adc
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(akira_adc, CONFIG_AKIRA_LOG_LEVEL);

/**
 * @file akira_adc_api.c
 * @brief Stateless raw ADC register read/write API for WASM applications
 *
 */

#include "akira_adc_api.h"
#include <runtime/security.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <errno.h>

#ifdef CONFIG_AKIRA_WASM_RUNTIME

static K_MUTEX_DEFINE(s_adc_lock);

/* -------------------------------------------------------------------------- */
/* Lazy ADC resolve                                                           */
/* -------------------------------------------------------------------------- */

static const struct device *get_adc(int32_t adc_id)
{
    const struct device *dev = NULL;

    switch (adc_id) {
    case 0:
#if DT_NODE_EXISTS(DT_NODELABEL(adc0))
    dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(adc0));
#elif DT_NODE_EXISTS(DT_NODELABEL(adc1))
        /* Some boards expose only adc1; allow adc_id=0 as a fallback. */
    dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(adc1));
#else
        return NULL;
#endif
        break;
    case 1:
#if DT_NODE_EXISTS(DT_NODELABEL(adc1))
    dev = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(adc1));
#else
        return NULL;
#endif
        break;
    default:
        return NULL;
    }

    if (!dev || !device_is_ready(dev)) {
        LOG_ERR("adc%d device not ready", adc_id);
        return NULL;
    }

    return dev;
}

/* -------------------------------------------------------------------------- */
/* WASM native export                                                         */
/* -------------------------------------------------------------------------- */

int akira_native_adc_read(wasm_exec_env_t exec_env,
                          int32_t adc_id,
                          int32_t channel,
                          int32_t *value)
{
    AKIRA_CHECK_CAP_OR_RETURN(exec_env, AKIRA_CAP_ADC, -EPERM);

    if (!value) {
        LOG_ERR("adc_read: value ptr null");
        return -EINVAL;
    }

    if (channel < 0 || channel > 31) {
        LOG_ERR("adc_read: invalid channel=%d", channel);
        return -EINVAL;
    }

    k_mutex_lock(&s_adc_lock, K_FOREVER);

    const struct device *dev = get_adc(adc_id);
    if (!dev) {
        k_mutex_unlock(&s_adc_lock);
        return -ENODEV;
    }

    int16_t sample_buffer;

    struct adc_channel_cfg ch_cfg = {
        .channel_id = channel,
        .gain = ADC_GAIN_1,
        .reference = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
    };

    int ret = adc_channel_setup(dev, &ch_cfg);
    if (ret < 0) {
        k_mutex_unlock(&s_adc_lock);
        LOG_ERR("adc_channel_setup failed: %d", ret);
        return ret;
    }

    struct adc_sequence sequence = {
        .channels = BIT(channel),
        .buffer = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution = 12,
    };

    ret = adc_read(dev, &sequence);

    k_mutex_unlock(&s_adc_lock);

    if (ret < 0) {
        LOG_ERR("adc_read failed: %d", ret);
        return ret;
    }

    *value = sample_buffer;

    return 0;
}

#endif /* CONFIG_AKIRA_WASM_RUNTIME */