/********************************************************************
* Description:  bb_gpio
*               This file, 'bb_gpio.c', is a HAL component that 
*               provides access to gpio pins using /dev/mem.
*
* Author: John Allwine <john@pocketnc.com>
* License: GPL Version 2
*    
* Copyright (c) 2021 Pocket NC Company All rights reserved.
*
********************************************************************/

#include "rtapi.h"              /* RTAPI realtime OS API */
#include "rtapi_app.h"          /* RTAPI realtime module decls */
#include "rtapi_errno.h"        /* EINVAL etc */
#include "hal.h"                /* HAL public API decls */
#include "hal_priv.h"
#include "bb_gpio.h"
#include <sys/mman.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

MODULE_AUTHOR("John Allwine");
MODULE_DESCRIPTION("Driver for GPIO pins on Beaglebone Boards using /dev/mem");
MODULE_LICENSE("GPL");

static const char *modname = "bb_gpio";
static int comp_id;

//sysfs_board_t board_id;
//static char *board;
//RTAPI_MP_STRING(board, "BBB, BBAI, OTHER (default)");

static int pin;
RTAPI_IP_INT(pin, "pin");

static char* direction;
RTAPI_IP_STRING(direction, "direction of the GPIO (input or output)");

static void read_input_pins(void *arg, long period) {
}

static void write_output_pins(void *arg, long period) {
}

bb_header_pin_t find_header_pin(uint16_t header_pin, const bb_header_pin_t *pin_map) {
  const size_t num_pins = sizeof(bbai_pin_map)/sizeof(bb_header_pin_t);
  for(size_t i = 0; i < num_pins; i++) {
    if(pin_map[i].header_pin == header_pin) {
      return pin_map[i];
    }
  }

  return UNKNOWN_PIN;
}

bb_gpio_port_t *root_port = NULL;

// finds a previously created port for the provided pin
bb_gpio_port_t* find_port(bb_gpio_port_t *root, bb_header_pin_t pin, const off_t* port_addresses) {
  bb_gpio_port_t *next = root;
  bb_gpio_port_t *port = NULL;

  while(next != NULL && port == NULL) {
    if(next->port_num == pin.port_num) {
      port = next;
    }
    next = next->next;
  }

  return port;
}

bb_gpio_direction_t check_pin_direction(bb_gpio_port_t *port, bb_header_pin_t pin) {
  bb_pin_t *next = port->input_pin;
  while(next != NULL) {
    if(next->line_num == pin.line_num) {
      return INPUT;
    }
    next = next->next;
  }

  next = port->output_pin;
  while(next != NULL) {
    if(next->line_num == pin.line_num) {
      return OUTPUT;
    }
    next = next->next;
  }

  return UNKNOWN;
}

int create_pin(bb_gpio_port_t *port, bb_header_pin_t header_pin, bb_gpio_direction_t dir, const char* name) {
  bb_pin_t *pin;

  int inst_id;
  if((inst_id = hal_inst_create(name, comp_id, sizeof(bb_pin_t), (void**)&pin)) < 0) {
    rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: failed to create hal instance for %s\n", modname, name);
    return -1;
  }
  pin->inst_id = inst_id;
  pin->line_num = header_pin.line_num;

  if(hal_pin_bit_newf(HAL_IN, &(pin->invert), inst_id, "%s.invert", name) < 0) {
    hal_exit(comp_id);
    return -1;
  }

  if(dir == INPUT) {
    if(hal_pin_bit_newf(HAL_OUT, &(pin->value), inst_id, "%s.in", name) < 0) {
      hal_exit(comp_id);
      return -1;
    }
    pin->next = port->input_pin;
    port->input_pin = pin;
  } else {
    if(hal_pin_bit_newf(HAL_IN, &(pin->value), inst_id, "%s.out", name) < 0) {
      hal_exit(comp_id);
      return -1;
    }
    pin->next = port->output_pin;
    port->output_pin = pin;
  }

  return inst_id;
}

//static int write_ports(void *arg, const hal_funct_args_t *fa) {
static void write_ports(void *arg, long period) {
  bb_gpio_port_t *port = root_port;
  while(port != NULL) {
    bb_pin_t *pin = port->output_pin;
    uint32_t setData = 0;
    uint32_t clrData = 0;
    while(pin != NULL) {
      if(*(pin->value) ^ *(pin->invert)) {
        setData |= 1 << pin->line_num;
      } else {
        clrData |= 1 << pin->line_num;
      }
      pin = pin->next;
    }
    *(port->set) = setData;
    *(port->clr) = clrData;
    port = port->next;
  }
};

static void read_ports(void *arg, long period) {
  bb_gpio_port_t *port = root_port;
  while(port != NULL) {
    if(port->input_pin) {
      uint32_t data = *(port->data);
      rtapi_print_msg(RTAPI_MSG_ERR, "%s: %u\n", modname, data);
      bb_pin_t *pin = port->input_pin;
      while(pin != NULL) {
        *(pin->value) = ((data & (1 << pin->line_num)) >> pin->line_num) ^ *(pin->invert);
        pin = pin->next;
      }
    }
    port = port->next;
  }
};

static int instantiate_bb_gpio(const int argc, char* const *argv) {
  const char* instname = argv[1];

  if(strcmp(instname, "init") == 0) {
  } else {
    // TODO - choose proper pin_map and port_addresses based on detected or specified device
    const bb_header_pin_t *pin_map = bbai_pin_map;
    const off_t *port_addresses = bbai_gpio_ports;

    const bb_header_pin_t instpin = find_header_pin((uint16_t)pin, pin_map);
    if(instpin.header_pin == -1) {
      rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: unknown pin %d.\n", modname, pin);
    }

    bb_gpio_port_t *port = find_port(root_port, instpin, port_addresses);
    if(port == NULL) {
      port = hal_malloc(sizeof(bb_gpio_port_t));
      if(port == 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: hal_malloc() failed\n", modname);
        return -1;
      }

      int fd = open("/dev/mem", O_RDWR);
      port->addr = mmap(0, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, port_addresses[instpin.port_num-1]);
      close(fd);
      if(port->addr < 0) {
        rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: mmap failed\n", modname);
        return -1;
      }

      port->set = port->addr + GPIO_SET_OFFSET;
      port->clr = port->addr + GPIO_CLR_OFFSET;
      port->data = port->addr + GPIO_DATA_OFFSET;
      if(root_port != NULL) {
        port->next = root_port->next;
      }
      root_port = port;
    }

    bb_gpio_direction_t dir;

    if(strcmp(direction, "output") == 0) {
      dir = OUTPUT;
    } else if(strcmp(direction, "input") == 0) {
      dir = INPUT;
    } else {
      rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: unknown direction %s\n", modname, direction);
      return -1;
    }

    bb_gpio_direction_t existing_dir = check_pin_direction(port, instpin);
    if(existing_dir == UNKNOWN) {
      int inst_id = create_pin(port, instpin, dir, instname);
    } else {
      rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: pin already created as %s\n", modname, existing_dir == INPUT ? "input" : "output");
      return -1;
    }
  }
  return 0;
}
           

int rtapi_app_main(void) {
  comp_id = hal_xinit(TYPE_RT, 0, 0, instantiate_bb_gpio, NULL, modname);
  if(comp_id < 0) {
    rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: hal_init() failed\n", modname);
    return -1;
  }

  if(hal_export_funct("bb_gpio.read", read_ports, NULL, 0, 0, comp_id) < 0) {
    rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: failed to export function %s.read\n", modname, modname);
    hal_exit(comp_id);
    return -1;
  }

  if(hal_export_funct("bb_gpio.write", write_ports, NULL, 0, 0, comp_id) < 0) {
    rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: failed to export function %s.write\n", modname, modname);
    hal_exit(comp_id);
    return -1;
  }


  hal_ready(comp_id);
  return 0;
}

void rtapi_app_exit(void) {
  hal_exit(comp_id);
}
