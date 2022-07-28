#ifndef collision_masks_h
#define collision_masks_h

#include <stdlib.h>

#define empty_collision_masks { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

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
                         uint8_t row16[1]);

void set_collision_masks_8(uint16_t *masks,
                           uint8_t row9[8],
                           uint8_t row10[7],
                           uint8_t row11[6],
                           uint8_t row12[5],
                           uint8_t row13[4],
                           uint8_t row14[3],
                           uint8_t row15[2],
                           uint8_t row16[1]);
#endif /* collision_masks_h */
