# RGB Matrix Train Departure Board
You can build you're own train departure board showing the next train from your favourite station to your favourite destination, where it's calling, and the following two departures

# Motivation
Being a life-long train fan I've always wanted my own departure board... and as a regular commuter I wanted one which shows my usual route.
Over the years I've looked at flip-dot displays and various other mechanisms and then saw LED dot-matrix displays become available for purchase.

These look amazing... however the price and monthly-subscription was offputting so I thought "I wonder if I could build one for less".

And here we are.

# This documentation is evolving
In this second version (24th Feb 2025) there's still gaps, however it's a lot more complete than it was!
Happy to help with any questions - best to do that via github so others can share the wisdom!

# Hardware
## The TL:DR
* [three P2, 5V, 128*64 pixel colour modules with a HUB75E interface from Ali Express](https://www.aliexpress.com/item/32913063042.html)
* [a 1GB Raspberry Pi 4 from Pimoroni](https://shop.pimoroni.com/products/raspberry-pi-4?variant=31856486416467)
* [Adafruit RGB Matrix Bonnet for Raspberry Pi from Pimoroni](https://shop.pimoroni.com/products/adafruit-rgb-matrix-bonnet-for-raspberry-pi?variant=2257849155594)
* A 5V power-supply capable of delivering at least 5 Amps

## Some RGB matrix boards.
I purchased [three P2, 5V, 128*64 pixel colour modules with a HUB75E interface from Ali Express](https://www.aliexpress.com/item/32913063042.html).

Three is a good size and three is limit for a chain of panels with the matrix library I used.  However you could have up to three rows - which might be nice (note to self - explore this option. It might "just work")
There are a myriad sellers on Ali Express and elsewhere. I suspect there's little to differentiate between offerings.

## A Raspberry Pi
You could purchase [a 1GB Raspberry Pi 4 from Pimoroni](https://shop.pimoroni.com/products/raspberry-pi-4?variant=31856486416467) - I'm not plugging Pimoroni, it's just that they also stock the RGB Matrix Bonnet so you can save on postage.

I used a Raspberry Pi 4 which was unloved and needed a new purpose. 

**Note** that the RGB matrix library doesn't yet work with a Pi 5.

## An RGB matrix-driver 
This project was built with an [Adafruit RGB Matrix Bonnet for Raspberry Pi from Pimoroni](https://shop.pimoroni.com/products/adafruit-rgb-matrix-bonnet-for-raspberry-pi?variant=2257849155594)

Specification on the [Adafruit site](https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi).  If there are other mechanisms which folk can recommend then let me know!

## 16 way ribbon cable
Most sellers provide these with the matrix panels. If you're making your own then your mantra has to be "Short Is Good".

## Some power!
I used a bench-top adjustable power supply to provide 5v for the matrix boards and the Raspberry Pi.

The nature of the panels means that current can vary quite wildly.  For the departure board with three panels it runs at about 1.5 to 2.0 Amps.

I'd recommend getting something substantial as the "all pixels on" power is likely to be more... and let's face it, you're going to want to play about with it!

## Anything else on hardware?
I'd highly recommend reading the detail on the [hzeller rpi-rgb-led-matrix - Let's Do It!](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#lets-do-it) documentation as it answers every question you can think of.
It's an awesome resource and fabulous software. More of that below!

## Putting it all together
I'll provde more detail on this - the important bit (for my configuration) was the modifications to:
* Connect pins 4 and 18 on the Adafruit bonnet
* Melt a blob of solder between the center “E” pad and the “8” pad just above it on the bottom of the bonnet
* I had to chop some pins off a connector on the Pi as it hit the bonnet - I suspect I may regret that at some point when the Pi gets used for something else
Have a read of the [if you have and Adafruit Hat or Bonnet](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#if-you-have-an-adafruit-hat-or-bonnet) section of the RPI RGB LED Matrix documentation.

# Setting up the Raspberry Pi #

## Install the OS ##

There are more tutorials than you can shake a stick at on how to install an OS on a Raspberry Pi.

I went for the 'standard build' - although a cut-down OS may be an option - and configured Wifi and ssh connectivity as build options. The aim here is to maximise CPU cycles the matrix driver can use.

Once installed there was the usual upgrade/update to make sure everything is current and shiny.

## Install the RGB Matrix Software ##

Log in, download and install the goodness from the [hzeller rpi-rgb-led-matrix repository](https://github.com/hzeller/rpi-rgb-led-matrix).

I can't recall the options at installation time, but I went for the 'performance' option (or similar named).

Build and run the demos and have fun! 

A pre-emptive read of the [Troubleshooting section)](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#troubleshooting) will help you get ahead of issues.

# So what about the train data? #
The source of the data is the [Live Departure Boards Web Service (LDBWS / OpenLDBWS)](https://lite.realtime.nationalrail.co.uk/OpenLDBWS/)

You'll need to register to be able to access the data - this is pretty straightforward via the [Open LDBWS Registration page](https://realtime.nationalrail.co.uk/OpenLDBWSRegistration).  Make a note of the key (although they email it to you aswell).

I'll include more links later - there is a wealth of information out there on train data and a mind-boggling amount of information you can get access too (although it's not all free).  The [Open Rail Data wiki](https://wiki.openraildata.com/index.php/Main_Page) is an awesome resource as is the list of [Open Rail data repositories](https://github.com/openraildata) here on Github.

**However** the data is only available from Network Rail via SOAP.  Not my forte.  If you're a Python programmer then you can go to the [Open Rail Data repository](https://github.com/openraildata) however, being an old lag and liking the enhanced performance of a compiled executable, C++ was the way to go.  I'm not about to try to ingest SOAP with C++.

## Huxley2 to the rescue ##
Huxley2 is a cross-platform JSON proxy for the GB railway Live Departure Boards SOAP API and is [here on Github](https://github.com/jpsingleton/Huxley2). More detail on [this site which includes a demo server](https://huxley2.azurewebsites.net)

I've create a fork of Huxley2 with modifications for running locally on a raspberry Pi - [Huxley 2 for Raspberry Pi](https://github.com/jonmorrissmith/jonms-Huxley2)

However you can also install on Azure. The instructions for this are fabulous are are available [on this blog post](https://unop.uk/huxley-2-release).

I went for the local install as it means a free data-source... not that the cost of running on Azure is in any way punitive given the microscopic workload.

# The final Steps #

## Installing the RGB Matrix Train Departure Board software ##
The biggest dependency here is the [JSON for modern C++](https://json.nlohmann.me/) which you can find [here on github](https://github.com/nlohmann/json)

Easiest to install via your [favourite package manager](https://github.com/nlohmann/json?tab=readme-ov-file#package-managers) - I used homebrew.

## Install the Train Display Software ##

Download train_service_display.cpp from this repository and edit the configuration class to provide default settings:
```// Configuration class
class Config {
private:
    std::map<std::string, std::string> settings;

    const std::map<std::string, std::string> defaults = {
        {"from", "<your default departure - use the three letter station code>"},
        {"to", "<your default destination point - use the three letter station code>"},
        {"APIURL", "<URL for your train info API>"},
        {"fontPath", "/home/<your path>/rpi-rgb-led-matrix/fonts/9x18.bdf"},
        {"scroll_slowdown_sleep_ms", "50"},
        {"refresh_interval_seconds", "60"},
        {"matrixcols", "128"},
        {"matrixrows", "64"},
        {"matrixchain_length", "3"},
        {"matrixparallel", "1"},
        {"matrixhardware_mapping", "adafruit-hat-pwm"},
        {"gpio_slowdown", "4"},
        {"first_line_y", "18"},
        {"second_line_y", "38"},
        {"third_line_y", "58"},
        {"third_line_refresh_seconds", "10"}
    };
```

And compile!

`g++ -std=c++11 train_service_display.cpp -o train_service_display -lrgbmatrix -lcurl -lpthread -I/home/<your path>/rpi-rgb-led-matrix/include -L/<your path>/display/rpi-rgb-led-matrix/lib`

# And finally Cyril... and finally Esther #

## Create your configuration file ##
This can be used to over-ride settings in the configuration class and other customisations

```# Train Display Configuration File
# Lines starting with # are comments

# Station codes
from=<your default departure - use the three letter station code>
to=<your default destination - use the three letter station code>

# API Configuration
APIURL=<Your API URL>

# Display font configuration
fontPath=/home/<yourpath>/rpi-rgb-led-matrix/fonts/9x18.bdf

# Timing parameters (in milliseconds/seconds)
scroll_slowdown_sleep_ms=35
refresh_interval_seconds=60
third_line_refresh_seconds=10

# Matrix hardware configuration
matrixcols=128
matrixrows=64
matrixchain_length=3
matrixparallel=1
matrixhardware_mapping=adafruit-hat-pwm
gpio_slowdown=2

# Display layout configuration (vertical positions)
first_line_y=18
second_line_y=38
third_line_y=58
```
### Run the executable ###

You've got four options all of which also support a '-d' option for debugging information

Use the default configuration in the executable

`sudo ./train_service_display`

Use the default configuration in the executable and specify origin and/or destination

`sudo ./train_service_display SAC STP`

Use your configuration file

`sudo ./train_service_display -f <config file>`

Combination of the above

`sudo ./train_service_display SAC STP -f <config file>`

Enjoy!

# Huge thanks to... #

[James Singleton's Huxley2](https://github.com/jpsingleton/Huxley2)

[Hzeller RPI RGB LED Matrix](https://github.com/hzeller/rpi-rgb-led-matrix)

[Nlohmann JSON for modern C++](https://github.com/nlohmann/json)
