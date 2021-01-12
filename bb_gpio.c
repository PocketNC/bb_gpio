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
#include "bb_gpio.h"

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

static int instantiate_bb_gpio(const int argc, char* const *argv) {
  const char* instname = argv[1];

  // TODO - choose proper pin_map and port_addresses based on detected or specified device
  const bb_header_pin_t *pin_map = bbai_pin_map;
  const off_t *port_addresses = bbai_gpio_ports;

  const bb_header_pin_t instpin = find_header_pin((uint16_t)pin, pin_map);
  if(instpin == UNKNOWN_PIN) {
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
    port->addr = mmap(0, GPIO_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, port_addresses[pin.port_num-1]);
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

  create_pin(port, instpin);
}
           

int rtapi_app_main(void) {
  comp_id = hal_xinit(TYPE_RT, 0, 0, instantiate_bb_gpio, NULL, modname);
  if(comp_id < 0) {
    rtapi_print_msg(RTAPI_MSG_ERR, "%s: ERROR: hal_init() failed\n", modname);
    return -1;
  }

  hal_ready(comp_id);
  return 0;
}

void rtapi_app_exit(void) {
  hal_exit(comp_id);
}
