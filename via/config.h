#pragma once

#define VKO_ENTRIES 4                 // how many key_overrides to use
// #define VKO_CUSTOM_CHANNEL 0       // when undefined, use 0 as default
#define ID_VKO_START 1                // id_vko_enabled, default: 1
#define ID_VKO_END (ID_VKO_START + 8) // id_vko_dummy, default: 9

#ifndef __ASSEMBLER__
// __dummy_key_override_t exists only to count the size of key_override_t
typedef struct {
    __UINT16_TYPE__ trigger;
    __UINT8_TYPE__  trigger_mods;
#    if !defined(DYNAMIC_KEYMAP_LAYER_COUNT) || DYNAMIC_KEYMAP_LAYER_COUNT > 16
    __UINT32_TYPE__ layers;
#    elif DYNAMIC_KEYMAP_LAYER_COUNT <= 8
    __UINT8_TYPE__ layers;
#    else
    __UINT16_TYPE__ layers;
#    endif
    __UINT8_TYPE__  negative_mod_mask;
    __UINT8_TYPE__  suppressed_mods;
    __UINT16_TYPE__ replacement;
    __UINT8_TYPE__  options;
} __dummy_key_override_t;
#endif

#define VIA_EEPROM_CUSTOM_CONFIG_SIZE (sizeof(__dummy_key_override_t) * VKO_ENTRIES)
