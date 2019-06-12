/*
 * joy.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "joy.h"

#include "demo.h"
#include "kbd.h"
#include "menu.h"
#include "menu_keyset.h"
#include "ui.h"

#include "joyport/joystick.h"
#include <stdio.h>
#include <string.h>

// Global usb gamepad info.
int joy_num_pads;
int joy_num_buttons[MAX_USB_PADS];
int joy_num_axes[MAX_USB_PADS];
int joy_num_hats[MAX_USB_PADS];

// Holds previous button states for the purpose of
// reporting button functions press/release events.
unsigned joy_prev_buttons[MAX_USB_PADS];

struct joydev_config joydevs[MAX_JOY_PORTS];

// Called from ISR
// Return non-zero if the event was handled.
int joy_key_up(unsigned int device, int key) {
  if (circle_ui_activated()) {
    // When the ui is showing, we want these
    // to turn into navigation for the ui.
    if (joydevs[device].device == JOYDEV_NUMS_1) {
      switch (key) {
      case KEYCODE_KP8:
        circle_ui_key_interrupt(KEYCODE_Up, 0 /* up */);
        return 1;
      case KEYCODE_KP2:
        circle_ui_key_interrupt(KEYCODE_Down, 0 /* up */);
        return 1;
      case KEYCODE_KP4:
        circle_ui_key_interrupt(KEYCODE_Left, 0 /* up */);
        return 1;
      case KEYCODE_KP6:
        circle_ui_key_interrupt(KEYCODE_Right, 0 /* up */);
        return 1;
      case KEYCODE_KP5:
        circle_ui_key_interrupt(KEYCODE_Return, 0 /* up */);
        return 1;
      default:
        return 0;
      }
    } else if (joydevs[device].device == JOYDEV_NUMS_2) {
      switch (key) {
      case KEYCODE_KP9:
        circle_ui_key_interrupt(KEYCODE_Up, 0 /* up */);
        return 1;
      case KEYCODE_KP3:
        circle_ui_key_interrupt(KEYCODE_Down, 0 /* up */);
        return 1;
      case KEYCODE_KP7:
        circle_ui_key_interrupt(KEYCODE_Left, 0 /* up */);
        return 1;
      case KEYCODE_KP1:
        circle_ui_key_interrupt(KEYCODE_Right, 0 /* up */);
        return 1;
      case KEYCODE_KP0:
        circle_ui_key_interrupt(KEYCODE_Return, 0 /* up */);
        return 1;
      default:
        return 0;
      }
    }
    return 0;
  }

  int port = joydevs[device].port;
  if (joydevs[device].device == JOYDEV_NUMS_1) {
    switch (key) {
    case KEYCODE_KP8:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x01);
      return 1;
    case KEYCODE_KP2:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x02);
      return 1;
    case KEYCODE_KP4:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x04);
      return 1;
    case KEYCODE_KP6:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x08);
      return 1;
    case KEYCODE_KP5:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x10);
      return 1;
    default:
      return 0;
    }
  } else if (joydevs[device].device == JOYDEV_NUMS_2) {
    switch (key) {
    case KEYCODE_KP9:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x01);
      return 1;
    case KEYCODE_KP3:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x02);
      return 1;
    case KEYCODE_KP7:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x04);
      return 1;
    case KEYCODE_KP1:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x08);
      return 1;
    case KEYCODE_KP0:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x10);
      return 1;
    default:
      return 0;
    }
  } else if (joydevs[device].device == JOYDEV_CURS_SP ||
             joydevs[device].device == JOYDEV_CURS_LC) {
    switch (key) {
    case KEYCODE_Up:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x01);
      return 1;
    case KEYCODE_Down:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x02);
      return 1;
    case KEYCODE_Left:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x04);
      return 1;
    case KEYCODE_Right:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x08);
      return 1;
    case KEYCODE_Space:
      if (joydevs[device].device == JOYDEV_CURS_SP) {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x10);
        return 1;
      }
      break;
    case KEYCODE_LeftControl:
      if (joydevs[device].device == JOYDEV_CURS_LC) {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x10);
        return 1;
      }
      break;
    default:
      return 0;
    }
  } else if (joydevs[device].device == JOYDEV_KEYSET1) {
    if (keyset_codes[0][KEYSET_UP] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x01);
      return 1;
    } else if (keyset_codes[0][KEYSET_DOWN] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x02);
      return 1;
    } else if (keyset_codes[0][KEYSET_LEFT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x04);
      return 1;
    } else if (keyset_codes[0][KEYSET_RIGHT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x08);
      return 1;
    } else if (keyset_codes[0][KEYSET_FIRE] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x10);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTX] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_x_low_value << 5));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_x_high_value << 5);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTY] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_y_low_value << 13));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_y_high_value << 13);
      return 1;
    }
    return 0;
  } else if (joydevs[device].device == JOYDEV_KEYSET2) {
    if (keyset_codes[1][KEYSET_UP] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x01);
      return 1;
    } else if (keyset_codes[1][KEYSET_DOWN] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x02);
      return 1;
    } else if (keyset_codes[1][KEYSET_LEFT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x04);
      return 1;
    } else if (keyset_codes[1][KEYSET_RIGHT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x08);
      return 1;
    } else if (keyset_codes[1][KEYSET_FIRE] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~0x10);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTX] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_x_low_value << 5));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_x_high_value << 5);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTY] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_y_low_value << 13));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_y_high_value << 13);
      return 1;
    }
  }
  return 0;
}

// Called from ISR
// Return non-zero if the event was handled.
int joy_key_down(unsigned int device, int key) {
  if (circle_ui_activated()) {
    if (joydevs[device].device == JOYDEV_NUMS_1) {
      switch (key) {
      case KEYCODE_KP8:
        circle_ui_key_interrupt(KEYCODE_Up, 1 /* down */);
        return 1;
      case KEYCODE_KP2:
        circle_ui_key_interrupt(KEYCODE_Down, 1 /* down */);
        return 1;
      case KEYCODE_KP4:
        circle_ui_key_interrupt(KEYCODE_Left, 1 /* down */);
        return 1;
      case KEYCODE_KP6:
        circle_ui_key_interrupt(KEYCODE_Right, 1 /* down */);
        return 1;
      case KEYCODE_KP5:
        circle_ui_key_interrupt(KEYCODE_Return, 1 /* down */);
        return 1;
      default:
        return 0;
      }
    } else if (joydevs[device].device == JOYDEV_NUMS_2) {
      switch (key) {
      case KEYCODE_KP9:
        circle_ui_key_interrupt(KEYCODE_Up, 1 /* down */);
        return 1;
      case KEYCODE_KP3:
        circle_ui_key_interrupt(KEYCODE_Down, 1 /* down */);
        return 1;
      case KEYCODE_KP7:
        circle_ui_key_interrupt(KEYCODE_Left, 1 /* down */);
        return 1;
      case KEYCODE_KP1:
        circle_ui_key_interrupt(KEYCODE_Right, 1 /* down */);
        return 1;
      case KEYCODE_KP0:
        circle_ui_key_interrupt(KEYCODE_Return, 1 /* down */);
        return 1;
      default:
        return 0;
      }
    } 
    return 0;
  }

  int port = joydevs[device].port;
  if (joydevs[device].device == JOYDEV_NUMS_1) {
    switch (key) {
    case KEYCODE_KP8:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x01);
      return 1;
    case KEYCODE_KP2:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x02);
      return 1;
    case KEYCODE_KP4:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x04);
      return 1;
    case KEYCODE_KP6:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x08);
      return 1;
    case KEYCODE_KP5:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x10);
      return 1;
    default:
      return 0;
    }
  } else if (joydevs[device].device == JOYDEV_NUMS_2) {
    switch (key) {
    case KEYCODE_KP9:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x01);
      return 1;
    case KEYCODE_KP3:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x02);
      return 1;
    case KEYCODE_KP7:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x04);
      return 1;
    case KEYCODE_KP1:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x08);
      return 1;
    case KEYCODE_KP0:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x10);
      return 1;
    default:
      return 0;
    }
  } else if (joydevs[device].device == JOYDEV_CURS_SP ||
             joydevs[device].device == JOYDEV_CURS_LC) {
    switch (key) {
    case KEYCODE_Up:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x01);
      return 1;
    case KEYCODE_Down:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x02);
      return 1;
    case KEYCODE_Left:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x04);
      return 1;
    case KEYCODE_Right:
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x08);
      return 1;
    case KEYCODE_Space:
      if (joydevs[device].device == JOYDEV_CURS_SP) {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x10);
        return 1;
      }
      break;
    case KEYCODE_LeftControl:
      if (joydevs[device].device == JOYDEV_CURS_LC) {
        circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x10);
        return 1;
      }
      break;
    default:
      return 0;
    }
  } else if (joydevs[device].device == JOYDEV_KEYSET1) {
    if (keyset_codes[0][KEYSET_UP] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x01);
      return 1;
    } else if (keyset_codes[0][KEYSET_DOWN] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x02);
      return 1;
    } else if (keyset_codes[0][KEYSET_LEFT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x04);
      return 1;
    } else if (keyset_codes[0][KEYSET_RIGHT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x08);
      return 1;
    } else if (keyset_codes[0][KEYSET_FIRE] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x10);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTX] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_x_high_value << 5));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_x_low_value << 5);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTY] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_y_high_value << 13));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_y_low_value << 13);
      return 1;
    }
    return 0;
  } else if (joydevs[device].device == JOYDEV_KEYSET2) {
    if (keyset_codes[1][KEYSET_UP] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x01);
      return 1;
    } else if (keyset_codes[1][KEYSET_DOWN] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x02);
      return 1;
    } else if (keyset_codes[1][KEYSET_LEFT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x04);
      return 1;
    } else if (keyset_codes[1][KEYSET_RIGHT] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x08);
      return 1;
    } else if (keyset_codes[1][KEYSET_FIRE] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, 0x10);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTX] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_x_high_value << 5));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_x_low_value << 5);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTY] == key) {
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, ~(pot_y_high_value << 13));
      circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, pot_y_low_value << 13);
      return 1;
    }
  }
  return 0;
}

// NOTE: This is called from the ISR so we can't call into the
// emulator API directly from here. Instead, we have to queue up
// the event for processing on the main loop.
void circle_joy_usb(unsigned int device, int value) {
  if (device == 0 && joydevs[0].device == JOYDEV_USB_0) {
    circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, joydevs[0].port,
                             value);
  } else if (device == 0 && joydevs[1].device == JOYDEV_USB_0) {
    circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, joydevs[1].port,
                             value);
  } else if (device == 1 && joydevs[0].device == JOYDEV_USB_1) {
    circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, joydevs[0].port,
                             value);
  } else if (device == 1 && joydevs[1].device == JOYDEV_USB_1) {
    circle_emu_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, joydevs[1].port,
                             value);
  }
}

// Setup stuff
int joy_arch_init(void) { return 0; }

void joy_set_gamepad_info(int num_pads,
                          int num_buttons[MAX_USB_PADS],
                          int num_axes[MAX_USB_PADS],
                          int num_hats[MAX_USB_PADS]) {
  joy_num_pads = MIN(num_pads , MAX_USB_PADS);
  joy_num_axes[0] = MIN(num_axes[0] , MAX_USB_AXES);
  joy_num_axes[1] = MIN(num_axes[1] , MAX_USB_AXES);
  joy_num_hats[0] = MIN(num_hats[0] , MAX_USB_HATS);
  joy_num_hats[1] = MIN(num_hats[1] , MAX_USB_HATS);
  joy_num_buttons[0] = MIN(num_buttons[0] , MAX_USB_BUTTONS);
  joy_num_buttons[1] = MIN(num_buttons[1] , MAX_USB_BUTTONS);
}

int circle_joy_need_gpio(int device) {
  if (device == 0) {
    return joydevs[0].device == JOYDEV_GPIO_0 ||
           joydevs[1].device == JOYDEV_GPIO_0;
  } else {
    return joydevs[0].device == JOYDEV_GPIO_1 ||
           joydevs[1].device == JOYDEV_GPIO_1;
  }
}

void joystick_close(void) {}
void joystick(void) {}
void old_joystick_init(void) {}
void old_joystick_close(void) {}
void old_joystick(void) {}
void new_joystick_init(void) {}
void new_joystick_close(void) {}
void new_joystick(void) {}

#ifdef HAS_USB_JOYSTICK
int usb_joystick_init(void) { return 0; }
void usb_joystick_close(void) {}
void usb_joystick(void) {}
#endif
