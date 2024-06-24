# fronius-goecontroller-adapter

This little command line tool will fetch the solar power values from a fronius inverter and apply it via MQTT to a go-e controller.

The idea is that it will add the power to category "HOME" and "SOLAR".

## Build instructions
At the moment the IP of the inverter and the hostname of the MQTT server and the serial number of the go-e controller are all hardcoded. Patch these before compiling!

Tested on a raspberry pi with a proper OS (not dumbian).

```
git clone https://github.com/0xFEEDC0DE64/fronius-goecontroller-adapter.git
cd fronius-goecontroller-adapter
qmake6
make
./fronius-goecontroller-adapter
```