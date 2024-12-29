import json
import argparse


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-l", "--num_layer", type=int, default=4, help="number of layers. ")
    parser.add_argument("-n", "--num_ko", type=int, default=4, help="number of key overrides to add")
    parser.add_argument("--channel_ko", type=int, default=0, help="required if VKO_CUSTOM_CHANNEL is defined as not 0")
    parser.add_argument("--id_vko_start", type=int, default=1, help="required if ID_VKO_START is defined as not 1")
    parser.add_argument("--id_vko_end", type=int, default=9, help="required if ID_VKO_END is defined as not 9")
    args = parser.parse_args()

    channel_ko = args.channel_ko
    num_ko = args.num_ko
    num_layer = args.num_layer
    id_vko_start = args.id_vko_start
    id_vko_end = args.id_vko_end

    control_order = ('enabled', 'layer', 'trigger', 'trigger_mods', 'negative_mod_mask', 'suppressed_mods', 'replacement', 'options')


    id_vko_number = "id_vko_number"
    id_vko_enabled_format = "id_vko%d_enabled"
    id_vko_layer = "id_vko_layer"
    id_vko_layer_format = "id_vko%d_layer%d"
    id_vko_trigger_format = "id_vko%d_trigger"
    id_vko_trigger_mods = "id_vko_trigger_mods"
    id_vko_trigger_mods_format = "id_vko%d_trigger_mods_%s"
    id_vko_negative_mod_mask = "id_vko_negative_mod_mask"
    id_vko_negative_mod_mask_format = "id_vko%d_negative_mod_mask_%s"
    id_vko_suppressed_mods = "id_vko_suppressed_mods"
    id_vko_suppressed_mods_format = "id_vko%d_suppressed_mods_%s"
    id_vko_replacement_format = "id_vko%d_replacement"
    id_vko_options = "id_vko_options"
    id_vko_options_format = "id_vko%d_options_%s"

    label_top = "Key Overrides"
    label_ko_format = "Key Overrides %d"
    label_ko_number = "Key Override Number"
    label_enable = "Enable"
    label_layer_toggle = "Layer"
    label_layer_format = f"| {label_layer_toggle} %d"
    label_trigger = "Trigger"
    label_trigger_mods_toggle = "Trigger mods"
    label_trigger_mods_format = f"| {label_trigger_mods_toggle} - %s"
    label_negative_mod_mask_toggle = "Negative mods"
    label_negative_mod_mask_format = f"| {label_negative_mod_mask_toggle} - %s"
    label_suppressed_mods_toggle = "Suppressed mods"
    label_suppressed_mods_format = f"| {label_suppressed_mods_toggle} - %s"
    label_replacement = "Replacement"
    label_options_toggle = "Options"
    label_options_format = f"| {label_options_toggle} - %s"

    type_dropdown = "dropdown"
    type_toggle = "toggle"
    type_keycode = "keycode"

    options_hide_show = [["hide", 0], ["show", 1]]

    # the order of the mods must be fixed
    mods = list()
    for lr in ('L', 'R'):
        for mod in ('Ctrl', 'Shift', 'Alt', 'Gui'):
            mods.append(lr + mod)

    # the order of the options must be fixed
    options = [
        'activation_trigger_down', 'activation_required_mod_down', 'activation_negative_mod_up', 
        'one_mod', 'no_reregister_trigger', 'no_unregister_on_other_key_down'
    ]

    def generate_controls(i):
        items = list()
        for idx, control in enumerate(control_order):
            if control == 'enabled':
                items.append({
                    "label": label_enable,
                    "type": type_toggle,
                    "content": [id_vko_enabled_format % i, channel_ko, idx + 1, i]
                })
            elif control == 'layer':
                items.append([{
                    "label": label_layer_toggle,
                    "type": type_dropdown,
                    "options": options_hide_show,
                    "content": [id_vko_layer, channel_ko, id_vko_end]
                }] + [{
                    "showIf": f"{{{id_vko_layer}}} == 1",
                    "label": label_layer_format % j,
                    "type": type_toggle,
                    "content": [id_vko_layer_format % (i, j), channel_ko, idx + 1, i, j]
                } for j in range(num_layer)])
            elif control == 'trigger':
                items.append({
                    "label": label_trigger,
                    "type": type_keycode,
                    "content": [id_vko_trigger_format % i, channel_ko, idx + 1, i]
                })
            elif control == 'trigger_mods':
                items.append([{
                    "label": label_trigger_mods_toggle,
                    "type": type_dropdown,
                    "options": options_hide_show,
                    "content": [id_vko_trigger_mods, channel_ko, id_vko_end]
                }] + [{
                    "showIf": f"{{{id_vko_trigger_mods}}} == 1",
                    "label": label_trigger_mods_format % mod,
                    "type": type_toggle,
                    "content": [id_vko_trigger_mods_format % (i, mod), channel_ko, idx + 1, i, j]
                } for j, mod in enumerate(mods)])
            elif control == 'negative_mod_mask':
                items.append([{
                    "label": label_negative_mod_mask_toggle,
                    "type": type_dropdown,
                    "options": options_hide_show,
                    "content": [id_vko_negative_mod_mask, channel_ko, id_vko_end]
                }] + [{
                    "showIf": f"{{{id_vko_negative_mod_mask}}} == 1",
                    "label": label_negative_mod_mask_format % mod,
                    "type": type_toggle,
                    "content": [id_vko_negative_mod_mask_format % (i, mod), channel_ko, idx + 1, i, j]
                } for j, mod in enumerate(mods)])
            elif control == 'suppressed_mods':
                items.append([{
                    "label": label_suppressed_mods_toggle,
                    "type": type_dropdown,
                    "options": options_hide_show,
                    "content": [id_vko_suppressed_mods, channel_ko, id_vko_end]
                }] + [{
                    "showIf": f"{{{id_vko_suppressed_mods}}} == 1",
                    "label": label_suppressed_mods_format % mod,
                    "type": type_toggle,
                    "content": [id_vko_suppressed_mods_format % (i, mod), channel_ko, idx + 1, i, j]
                } for j, mod in enumerate(mods)])
            elif control == 'replacement':
                items.append({
                    "label": label_replacement,
                    "type": type_keycode,
                    "content": [id_vko_replacement_format % i, channel_ko, idx + 1, i]
                })
            elif control == 'options':
                items.append([{
                    "label": label_options_toggle,
                    "type": type_dropdown,
                    "options": options_hide_show,
                    "content": [id_vko_options, channel_ko, id_vko_end]
                }] + [{
                    "showIf": f"{{{id_vko_options}}} == 1",
                    "label": label_options_format % option,
                    "type": type_toggle,
                    "content": [id_vko_options_format % (i, option), channel_ko, idx + 1, i, j]
                } for j, option in enumerate(options)])
        return items


    obj = {
        "label": label_top,
        "content": [{"label": label_ko_format % (i + 1), "content": []} for i in range(num_ko)]
    }
    for i in range(num_ko):
        base = obj["content"][i]["content"]
        for item in generate_controls(i):
            if type(item) is list:
                base.extend(item)
            else:
                base.append(item)


    print(f'{json.dumps(obj)},')


if __name__ == '__main__':
    main()