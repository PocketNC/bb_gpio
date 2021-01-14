#ifndef __BB_GPIO_H__
#define __BB_GPIO_H__

#define GPIO_SIZE 0x2000
#define GPIO_SET_OFFSET 0x194
#define GPIO_CLR_OFFSET 0x190
#define GPIO_DATA_OFFSET 0x138

const off_t bbai_gpio_ports[] = { 0x4AE10000, 0x48055000, 0x48057000, 0x48059000, 0x4805B000, 0x4805D000, 0x48051000, 0x48053000 };
const off_t bbb_gpio_ports[] = { 0x44E07000, 0x4804C000, 0x481AC000, 0x481AE000 };

typedef enum {
  UNKNOWN_BOARD,
  BBAI,
  BBB
} bb_gpio_board_t;

typedef enum {
  UNKNOWN_DIRECTION,
  INPUT,
  OUTPUT
} bb_gpio_direction_t;

typedef struct {
  uint16_t header_pin; // 8XX or 9xx for header P8.XX or P9.XX pins
  uint8_t port_num;    // GPIO port number 1-8 on the BBAI, 1-4 on the BBB
  uint8_t line_num;    // Line number of the pin (represents which bit controls the pin in the clear and set registers), 0-31
} bb_header_pin_t;

const bb_header_pin_t UNKNOWN_PIN = { -1, -1, -1 };

// From https://github.com/beagleboard/beaglebone-ai/wiki/System-Reference-Manual#expansion-connectors
// Mode 14 in each of the tables for groups of pins (will list gpio<PORT>_<LINE>)
// If multiple ports are connected to the same header pin, A will be the default unless there is only a B (listed as 2nd in the link above).
// To differentiate them, an extra 1 can be used to specifically choose A and an extra 2 will choose B.
// Examples:
// 827 and 8271 refer to p8.27a or GPIO4_23, but 8272 refers to p8.27b or GPIO8_19
// 911 and 9112 refer to p9.11b or GPIO8_17 (p9.11a doesn't map to a GPIO port)

const bb_header_pin_t bbai_pin_map[] = {
  { 803, 0, 24 }, 
  { 804, 0, 25 }, 
  { 805, 6,  1 }, 
  { 806, 6,  2 }, 
  { 807, 5,  5 }, 
  { 808, 5,  6 }, 
  { 809, 5, 18 }, 
  { 810, 5,  4 }, 
  { 811, 2, 11 },
  { 812, 2, 10 },
  { 813, 3, 11 },
  { 814, 3, 13 },
  { 815, 3,  3 }, { 8151, 3,  3 }, { 8152, 3, 27 },
  { 816, 3, 29 },
  { 817, 7, 18 },
  { 818, 3,  9 },
  { 819, 3, 10 },
  { 820, 5, 30 },
  { 821, 5, 29 },
  { 822, 0, 23 },
  { 823, 0, 22 },
  { 824, 6,  0 },
  { 825, 5, 31 },
  { 826, 3, 28 },
  { 827, 3, 23 }, { 8271, 3, 23 }, { 8272, 7, 19 },
  { 828, 3, 19 }, { 8281, 3, 19 }, { 8282, 7, 20 },
  { 829, 3, 22 }, { 8291, 3, 22 }, { 8292, 7, 21 },
  { 830, 3, 20 }, { 8301, 3, 20 }, { 8302, 7, 22 },
  { 831, 7, 14 },
  { 832, 7, 15 },
  { 833, 7, 13 }, { 8331, 7, 13 }, { 8332, 2,  1 },
  { 834, 7, 11 }, { 8341, 7, 11 }, { 8342, 3,  0 },
  { 835, 7, 12 }, { 8351, 7, 12 }, { 8352, 2,  0 },
  { 836, 7, 10 }, { 8361, 7, 10 }, { 8362, 3,  1 },
  { 837, 7,  8 },
  { 838, 7,  9 },
  { 839, 7,  6 },
  { 840, 7,  7 },
  { 841, 7,  4 },
  { 842, 7,  5 },
  { 843, 7,  2 },
  { 844, 7,  3 },
  { 845, 7,  0 }, { 8451, 7,  0 }, { 8452, 7, 16 },
  { 846, 7,  1 }, { 8461, 7,  1 }, { 8462, 7, 23 },
  { 911, 7, 17 }, { 9112, 7, 17 },
  { 912, 4,  0 },
  { 913, 5, 12 }, { 9132, 5, 12 },
  { 914, 3, 25 },
  { 915, 2, 12 },
  { 916, 3, 26 },
  { 917, 6, 17 }, { 9171, 6, 17 }, { 9172, 4,  3 },
  { 918, 6, 16 }, { 9181, 6, 16 }, { 9182, 4,  2 },
  { 919, 6,  3 }, { 9191, 6,  3 }, { 9192, 3,  6 },
  { 920, 6,  4 }, { 9201, 6,  4 }, { 9202, 3,  5 },
  { 921, 2,  3 }, { 9211, 2,  3 }, { 9212, 6, 15 },
  { 922, 5, 19 }, { 9221, 5, 19 }, { 9222, 6, 14 },
  { 923, 6, 11 },
  { 924, 5, 15 },
  { 925, 5, 17 },
  { 926, 5, 14 }, { 9261, 5, 14 }, { 9262, 2, 24 },
  { 927, 3, 15 }, { 9271, 3, 15 }, { 9272, 4,  1 },
  { 928, 3, 17 },
  { 929, 4, 11 }, { 9291, 4, 11 }, { 9292, 6, 30 },
  { 930, 4, 12 },
  { 931, 4, 10 }, { 9311, 4, 10 }, { 9312, 6, 31 },
  { 941, 5, 20 }, { 9411, 5, 20 }, { 9412, 3,  7 },
  { 942, 3, 18 }, { 9421, 3, 18 }, { 9422, 3, 14 }
};

const bb_header_pin_t bbb_pin_map[] = {
  { 803, 1,  6 }, 
  { 804, 1,  7 }, 
  { 805, 1,  2 }, 
  { 806, 1,  3 }, 
  { 807, 2,  2 }, 
  { 808, 2,  3 }, 
  { 809, 2,  5 }, 
  { 810, 2,  4 }, 
  { 811, 1, 13 },
  { 812, 1, 12 },
  { 813, 0, 23 },
  { 814, 0, 26 },
  { 815, 1, 15 },
  { 816, 1, 14 },
  { 817, 0, 27 },
  { 818, 2,  1 },
  { 819, 0, 22 },
  { 820, 1, 31 },
  { 821, 1, 30 },
  { 822, 1,  5 },
  { 823, 1,  4 },
  { 824, 1,  1 },
  { 825, 1,  0 },
  { 826, 1, 29 },
  { 827, 2, 22 },
  { 828, 2, 24 },
  { 829, 2, 23 },
  { 830, 2, 25 },
  { 831, 0, 10 },
  { 832, 0, 11 },
  { 833, 0,  9 },
  { 834, 2, 17 },
  { 835, 0,  8 },
  { 836, 2, 16 },
  { 837, 2, 14 },
  { 838, 2, 15 },
  { 839, 2, 12 },
  { 840, 2, 13 },
  { 841, 2, 10 },
  { 842, 2, 11 },
  { 843, 2,  8 },
  { 844, 2,  9 },
  { 845, 2,  6 },
  { 846, 2,  7 },
  { 911, 0, 30 },
  { 912, 1, 28 },
  { 913, 0, 31 },
  { 914, 1, 18 },
  { 915, 1, 16 },
  { 916, 1, 19 },
  { 917, 0,  5 },
  { 918, 0,  4 },
  { 919, 0, 13 },
  { 920, 0, 12 },
  { 921, 0,  3 },
  { 922, 0,  2 },
  { 923, 1, 17 },
  { 924, 0, 15 },
  { 925, 3, 21 },
  { 926, 0, 14 },
  { 927, 3, 19 },
  { 928, 3, 17 },
  { 929, 3, 15 },
  { 930, 3, 16 },
  { 931, 3, 14 },
  { 941, 0, 20 }, { 9411, 0, 20 }, { 9412, 3, 20 },
  { 942, 0,  7 }, { 9421, 0,  7 }, { 9422, 3, 18 }
};

typedef struct bb_pin {
  hal_bit_t *value;
  hal_bit_t *invert;

  uint8_t line_num;
  int inst_id;

  struct bb_pin *next;
} bb_pin_t;


typedef struct bb_gpio_port {
  volatile void *addr;     // start address for this GPIO port
  volatile uint32_t *set;  // set data register sets output pins high
  volatile uint32_t *clr;  // clr data register sets output pins low
  volatile uint32_t *data; // data register reads the state of input pins

  uint8_t port_num;    // GPIO port number 1-8 on the BBAI, 1-4 on the BBB

  bb_pin_t *input_pin;
  bb_pin_t *output_pin;

  struct bb_gpio_port *next;
} bb_gpio_port_t;

#endif
