#include "collision_masks.h"
#include "string_builder.h"

void set_collision_masks(uint16_t *masks,
                         uint8_t row1[16],
                         uint8_t row2[15],
                         uint8_t row3[14],
                         uint8_t row4[13],
                         uint8_t row5[12],
                         uint8_t row6[11],
                         uint8_t row7[10],
                         uint8_t row8[9],
                         uint8_t row9[8],
                         uint8_t row10[7],
                         uint8_t row11[6],
                         uint8_t row12[5],
                         uint8_t row13[4],
                         uint8_t row14[3],
                         uint8_t row15[2],
                         uint8_t row16[1])
{
    uint8_t *rows[] = {
        row1, row2, row3, row4, row5, row6, row7, row8,
        row9, row10, row11, row12, row13, row14, row15, row16
    };
    
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16 - i; ++j) {
            int k = 15 - i;
            uint16_t is_set = rows[i][j] > 0;
            masks[j] |= is_set << k;
            masks[k] |= is_set << j;
        }
    }
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "uint16_t *masks = { ");
    for (int i = 0; i < 16; ++i) {
        sb_append_string(sb, "0x");
        sb_append_hex(sb, masks[i]);
        if (i < 15) {
            sb_append_string(sb, ", ");
        }
    }
    sb_append_string(sb, " };\n");
    
    sb_debug_log_to_console(sb);
    destroy(sb);
}

void set_collision_masks_8(uint16_t *masks,
                           uint8_t row9[8],
                           uint8_t row10[7],
                           uint8_t row11[6],
                           uint8_t row12[5],
                           uint8_t row13[4],
                           uint8_t row14[3],
                           uint8_t row15[2],
                           uint8_t row16[1])
{
    uint8_t row1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row3[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row4[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row5[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row6[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row7[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t row8[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    
    uint8_t *rows[] = {
        row1, row2, row3, row4, row5, row6, row7, row8,
        row9, row10, row11, row12, row13, row14, row15, row16
    };
    
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16 - i; ++j) {
            int k = 15 - i;
            uint16_t is_set = rows[i][j] > 0;
            masks[j] |= is_set << k;
            masks[k] |= is_set << j;
        }
    }
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "uint16_t *masks = { ");
    for (int i = 0; i < 16; ++i) {
        sb_append_string(sb, "0x");
        sb_append_hex(sb, masks[i]);
        if (i < 15) {
            sb_append_string(sb, ", ");
        }
    }
    sb_append_string(sb, " };\n");
    
    sb_debug_log_to_console(sb);
    destroy(sb);
}
