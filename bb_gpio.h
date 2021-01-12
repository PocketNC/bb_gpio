#ifndef __BB_GPIO_H__
#define __BB_GPIO_H__

#define GPIO_SIZE 0x2000
#define GPIO_SET_OFFSET 0x194
#define GPIO_CLR_OFFSET 0x190
#define GPIO_DATA_OFFSET 0x138

const off_t bbai_gpio_ports[] = { 0x4AE10000, 0x48055000, 0x48057000, 0x48059000, 0x4805B000, 0x4805D000, 0x48051000, 0x48053000 };
const off_t bbb_gpio_ports[] = { 0x44E07000, 0x4804C000, 0x481AC000, 0x481AE000 };

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
  { 803, 1, 24 }, 
  { 804, 1, 25 }, 
  { 805, 7,  1 }, 
  { 806, 7,  2 }, 
  { 807, 6,  5 }, 
  { 808, 6,  6 }, 
  { 809, 6, 18 }, 
  { 810, 6,  4 }, 
  { 811, 3, 11 },
  { 812, 3, 10 },
  { 813, 4, 11 },
  { 814, 4, 13 },
  { 815, 4,  3 }, { 8151, 4,  3 }, { 8152, 4, 27 },
  { 816, 4, 29 },
  { 817, 8, 18 },
  { 818, 4,  9 },
  { 819, 4, 10 },
  { 820, 6, 30 },
  { 821, 6, 29 },
  { 822, 1, 23 },
  { 823, 1, 22 },
  { 824, 7,  0 },
  { 825, 6, 31 },
  { 826, 4, 28 },
  { 827, 4, 23 }, { 8271, 4, 23 }, { 8272, 8, 19 },
  { 828, 4, 19 }, { 8281, 4, 19 }, { 8282, 8, 20 },
  { 829, 4, 22 }, { 8291, 4, 22 }, { 8292, 8, 21 },
  { 830, 4, 20 }, { 8301, 4, 20 }, { 8302, 8, 22 },
  { 831, 8, 14 },
  { 832, 8, 15 },
  { 833, 8, 13 }, { 8331, 8, 13 }, { 8332, 3,  1 },
  { 834, 8, 11 }, { 8341, 8, 11 }, { 8342, 4,  0 },
  { 835, 8, 12 }, { 8351, 8, 12 }, { 8352, 3,  0 },
  { 836, 8, 10 }, { 8361, 8, 10 }, { 8362, 4,  1 },
  { 837, 8,  8 },
  { 838, 8,  9 },
  { 839, 8,  6 },
  { 840, 8,  7 },
  { 841, 8,  4 },
  { 842, 8,  5 },
  { 843, 8,  2 },
  { 844, 8,  3 },
  { 845, 8,  0 }, { 8451, 8,  0 }, { 8452, 8, 16 },
  { 846, 8,  1 }, { 8461, 8,  1 }, { 8462, 8, 23 },
  { 911, 8, 17 }, { 9112, 8, 17 },
  { 912, 5,  0 },
  { 913, 6, 12 }, { 9132, 6, 12 },
  { 914, 4, 25 },
  { 915, 3, 12 },
  { 916, 4, 26 },
  { 917, 7, 17 }, { 9171, 7, 17 }, { 9172, 5,  3 },
  { 918, 7, 16 }, { 9181, 7, 16 }, { 9182, 5,  2 },
  { 919, 7,  3 }, { 9191, 7,  3 }, { 9192, 4,  6 },
  { 920, 7,  4 }, { 9201, 7,  4 }, { 9202, 4,  5 },
  { 921, 3,  3 }, { 9211, 3,  3 }, { 9212, 7, 15 },
  { 922, 6, 19 }, { 9221, 6, 19 }, { 9222, 7, 14 },
  { 923, 7, 11 },
  { 924, 6, 15 },
  { 925, 6, 17 },
  { 926, 6, 14 }, { 9261, 6, 14 }, { 9262, 3, 24 },
  { 927, 4, 15 }, { 9271, 4, 15 }, { 9272, 5,  1 },
  { 928, 4, 17 },
  { 929, 5, 11 }, { 9291, 5, 11 }, { 9292, 7, 30 },
  { 930, 5, 12 },
  { 931, 5, 10 }, { 9311, 5, 10 }, { 9312, 7, 31 },
  { 941, 6, 20 }, { 9411, 6, 20 }, { 9412, 4,  7 },
  { 942, 4, 18 }, { 9421, 4, 18 }, { 9422, 4, 14 }
};

typedef struct bb_pin {
  hal_bit_t *value;
  hal_bit_t *invert;

  uint8_t line_num;

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
