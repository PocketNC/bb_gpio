bb_gpio
=======

MachineKit-HAL component similar to hal_bb_gpio that provides access to a Beaglebone's GPIO pins that is 
compatible with the Beaglebone Black and the Beaglebone AI. The main different between bb_gpio
and hal_bb_gpio is that bb_gpio is an instantiable component that is instantiated once per pin
rather than instantiating a single HAL component with all pins specified at once. This makes
using bb_gpio a little more verbose, but provides extra flexibility to where pin instantiation
happens. This flexiblity allows for better encapsulation of functionality, which leads to better
readability and maintainability of code. 

On the Beaglebone Black, where pin muxing is allowed in user space, pins could be added on the 
fly after properly configuring them with config pin. Pins can be added on the fly on the Beaglebone
AI as well, but since pin muxing isn't allowed in user space the pins would still need to be configured
via a device tree overlay ahead of time. Either way, pins can be instantiated in your HAL file
where they are needed rather than all at once.

Usage
=====

Install dependencies:

    sudo apt-get install libgpiod-dev

To install use instcomp (may be integrated into machinekit-hal eventually):

    # from the bb_gpio directory
    sudo EXTRA_LDFLAGS="-lgpiod" instcomp --install bb_gpio.c

Here's an example HAL file that could be used with an LED wired to P8.17 and
a button wired up to short GND to P8.14 (P8.14 configured to be an input pull up).

    newthread servo 1000000
    newinst bb_gpio init
    addf bb_gpio.read servo
    addf bb_gpio.write servo
    start
    newinst bb_gpio myled pin=817 direction=output
    newinst bb_gpio mybutton pin=814 direction=input

    setp mybutton.invert 1

    net control-led-with-button mybutton.in
    net control-led-with-button myled.out

To run:

    realtime start
    halcmd -f example.hal

License
=======

bb_gpio is licensed under the GNU Public License version 2.
