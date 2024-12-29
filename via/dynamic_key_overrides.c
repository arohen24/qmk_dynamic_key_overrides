#include QMK_KEYBOARD_H

#ifdef VIA_ENABLE
#    ifdef VKO_CUSTOM_CHANNEL
#        define CHANNEL_VKO VKO_CUSTOM_CHANNEL
#    else
#        define CHANNEL_VKO id_custom_channel
#    endif

#define VIA_EEPROM_VKO_ADDR VIA_EEPROM_CUSTOM_CONFIG_ADDR

// to store key_override on EEPROM
typedef struct {
    uint16_t      trigger;
    uint8_t       trigger_mods;
    layer_state_t layers;
    uint8_t       negative_mod_mask;
    uint8_t       suppressed_mods;
    uint16_t      replacement;
    ko_option_t   options;
} vko_key_override_entry_t;

// to store enabled in options
typedef enum {
    vko_option_enabled = (1 << 7), // referenced vial-qmk
} vko_option_t;

static bool            vko_disabled                     = false; // enabled points this when disabled
static key_override_t *vkos[VKO_ENTRIES]                = {0};   // key_overrides points this
static key_override_t  key_override_buffer[VKO_ENTRIES] = {0};   // values of vkos

// implementation for process_key_override.h
const key_override_t **key_overrides = (const key_override_t **)&vkos[0];

// referenced dynamic_keymap_get_key_override of vial-qmk
int keymap_get_key_override(uint8_t index, vko_key_override_entry_t *entry) {
    if (index >= VKO_ENTRIES) return -1;

    void *address = (void *)(VIA_EEPROM_VKO_ADDR + index * sizeof(vko_key_override_entry_t));
    eeprom_read_block(entry, address, sizeof(vko_key_override_entry_t));

    return 0;
}

// referenced dynamic_keymap_set_key_override of vial-qmk
int keymap_set_key_override(uint8_t index, const vko_key_override_entry_t *entry) {
    if (index >= VKO_ENTRIES) return -1;

    void *address = (void *)(VIA_EEPROM_VKO_ADDR + index * sizeof(vko_key_override_entry_t));
    eeprom_update_block(entry, address, sizeof(vko_key_override_entry_t));

    return 0;
}

// refernced vial_get_key_override of vial-qmk
static int vko_get_key_override(uint8_t index, key_override_t *out) {
    vko_key_override_entry_t entry;
    int                      ret;
    if ((ret = keymap_get_key_override(index, &entry)) != 0) return ret;

    memset(out, 0, sizeof(*out));
    out->trigger           = entry.trigger;
    out->trigger_mods      = entry.trigger_mods;
    out->layers            = entry.layers;
    out->negative_mod_mask = entry.negative_mod_mask;
    out->suppressed_mods   = entry.suppressed_mods;
    out->replacement       = entry.replacement;
    uint8_t opt            = entry.options;
    if (opt & vko_option_enabled)
        out->enabled = NULL;
    else
        out->enabled = &vko_disabled;
    out->options = entry.options;

    return 0;
}

// make vkos' element point key_override_buffer's element
static void initialize_vkos(void) {
    for (int i = 0; i < VKO_ENTRIES; ++i) {
        vkos[i] = &key_override_buffer[i];
    }

    for (size_t i = 0; i < VKO_ENTRIES; ++i) {
        vko_get_key_override(i, vkos[i]);

        // reset if needed
        if (!vkos[i]->trigger && !vkos[i]->trigger_mods && !vkos[i]->layers && !vkos[i]->negative_mod_mask && !vkos[i]->suppressed_mods && !vkos[i]->replacement && !vkos[i]->options) {
            vkos[i]->trigger           = KC_NO;
            vkos[i]->trigger_mods      = 0;
            vkos[i]->layers            = ~0;
            vkos[i]->negative_mod_mask = 0;
            vkos[i]->suppressed_mods   = 0;
            vkos[i]->replacement       = KC_NO;
            vkos[i]->options           = ko_options_default;
            vkos[i]->enabled           = &vko_disabled;
        }
    }
}

enum via_qmk_vko_value {
    id_vko_enabled = ID_VKO_START,
    id_vko_layers,
    id_vko_trigger,
    id_vko_trigger_mods,
    id_vko_negative_mod_mask,
    id_vko_suppressed_mods,
    id_vko_replacement,
    id_vko_options,
    id_vko_dummy = ID_VKO_END, // for dropdown
};

void vko_set_value(uint8_t *data) {
    // data = [ value_id, vko_number - 1, trigger | replacement ]
    // data = [ value_id, vko_number - 1, layer | mod | option, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t  idx        = data[1];
    uint8_t *value_data = &(data[2]);

    switch (*value_id) {
        case id_vko_enabled: {
            if (*value_data) {
                vkos[idx]->options |= vko_option_enabled;
                vkos[idx]->enabled = NULL;
            } else {
                vkos[idx]->options &= ~vko_option_enabled;
                vkos[idx]->enabled = &vko_disabled;
            }
            break;
        }
        case id_vko_layers: {
            uint8_t layer_idx = *value_data;
            uint8_t layer_on  = *(value_data + 1);
            if (layer_on)
                vkos[idx]->layers |= (layer_state_t)1 << layer_idx;
            else
                vkos[idx]->layers &= ~((layer_state_t)1 << layer_idx);
            break;
        }
        case id_vko_trigger: {
            vkos[idx]->trigger = value_data[0] << 8 | value_data[1];
            break;
        }
        case id_vko_trigger_mods: {
            uint8_t mod_idx = *value_data;
            uint8_t mod_on  = *(value_data + 1);
            if (mod_on)
                vkos[idx]->trigger_mods |= (uint8_t)1 << mod_idx;
            else
                vkos[idx]->trigger_mods &= ~((uint8_t)1 << mod_idx);
            break;
        }
        case id_vko_negative_mod_mask: {
            uint8_t mod_idx = *value_data;
            uint8_t mod_on  = *(value_data + 1);
            if (mod_on)
                vkos[idx]->negative_mod_mask |= (uint8_t)1 << mod_idx;
            else
                vkos[idx]->negative_mod_mask &= ~((uint8_t)1 << mod_idx);
            break;
        }
        case id_vko_suppressed_mods: {
            uint8_t mod_idx = *value_data;
            uint8_t mod_on  = *(value_data + 1);
            if (mod_on)
                vkos[idx]->suppressed_mods |= (uint8_t)1 << mod_idx;
            else
                vkos[idx]->suppressed_mods &= ~((uint8_t)1 << mod_idx);
            break;
        }
        case id_vko_replacement: {
            vkos[idx]->replacement = value_data[0] << 8 | value_data[1];
            break;
        }
        case id_vko_options: {
            uint8_t opt_idx = *value_data;
            uint8_t opt_on  = *(value_data + 1);
            if (opt_on)
                vkos[idx]->options |= (uint8_t)1 << opt_idx;
            else
                vkos[idx]->options &= ~((uint8_t)1 << opt_idx);
            break;
        }
        default:
            break;
    }
    keymap_set_key_override(idx, (vko_key_override_entry_t *)vkos[idx]);
}

void vko_get_value(uint8_t *data) {
    // data = [ value_id, vko_number - 1, value_data ]
    uint8_t *value_id   = &(data[0]);
    uint8_t  idx        = data[1];
    uint8_t *value_data = &(data[2]);

    switch (*value_id) {
        case id_vko_enabled: {
            *value_data = vkos[idx]->enabled == NULL;
            break;
        }
        case id_vko_layers: {
            uint8_t layer_idx = *value_data;
            uint8_t layer_on  = (vkos[idx]->layers & ((layer_state_t)1 << layer_idx)) != 0;
            *(value_data + 1) = layer_on;
            break;
        }
        case id_vko_trigger: {
            value_data[0] = vkos[idx]->trigger >> 8;
            value_data[1] = vkos[idx]->trigger & 0xFF;
            break;
        }
        case id_vko_trigger_mods: {
            uint8_t mod_idx   = *value_data;
            uint8_t mod_on    = (vkos[idx]->trigger_mods & ((uint8_t)1 << mod_idx)) != 0;
            *(value_data + 1) = mod_on;
            break;
        }
        case id_vko_negative_mod_mask: {
            uint8_t mod_idx   = *value_data;
            uint8_t mod_on    = (vkos[idx]->negative_mod_mask & ((uint8_t)1 << mod_idx)) != 0;
            *(value_data + 1) = mod_on;
            break;
        }
        case id_vko_suppressed_mods: {
            uint8_t mod_idx   = *value_data;
            uint8_t mod_on    = (vkos[idx]->suppressed_mods & ((uint8_t)1 << mod_idx)) != 0;
            *(value_data + 1) = mod_on;
            break;
        }
        case id_vko_replacement: {
            value_data[0] = vkos[idx]->replacement >> 8;
            value_data[1] = vkos[idx]->replacement & 0xFF;
            break;
        }
        case id_vko_options: {
            uint8_t opt_idx   = *value_data;
            uint8_t opt_on    = (vkos[idx]->options & ((uint8_t)1 << opt_idx)) != 0;
            *(value_data + 1) = opt_on;
            break;
        }
        default:
            break;
    }
}

void vko_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, vko_number, value_data ]
    uint8_t *command_id = &(data[0]);
    uint8_t *value_id   = &(data[2]);

    switch (*command_id) {
        case id_custom_set_value: {
            if (*value_id != ID_VKO_END) {
                vko_set_value(value_id);
            }
            break;
        }
        case id_custom_get_value: {
            if (*value_id != ID_VKO_END) {
                vko_get_value(value_id);
            }
            break;
        }
        case id_custom_save: {
            break;
        }
        default: {
            // Unhandled message.
            *command_id = id_unhandled;
            break;
        }
    }
}

// example from https://www.caniusevia.com/docs/custom_ui#command-handlers
void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    // data = [ command_id, channel_id, value_id, value_data ]
    uint8_t *command_id        = &(data[0]);
    uint8_t *channel_id        = &(data[1]);
    uint8_t *value_id_and_data = &(data[2]);

    // condition check for vko_command_kb
    if (*channel_id == CHANNEL_VKO && ID_VKO_START <= *value_id_and_data && *value_id_and_data <= ID_VKO_END) {
        vko_command_kb(data, length);
        return;
    } // end of code block

    if (*channel_id == id_custom_channel) {
        switch (*command_id) {
            case id_custom_set_value: {
                break;
            }
            case id_custom_get_value: {
                break;
            }
            case id_custom_save: {
                break;
            }
            default: {
                // Unhandled message.
                *command_id = id_unhandled;
                break;
            }
        }
        return;
    }

    // Return the unhandled state
    *command_id = id_unhandled;

    // DO NOT call raw_hid_send(data,length) here, let caller do this
}

void keyboard_post_init_user(void) {
    initialize_vkos();
}
#endif
