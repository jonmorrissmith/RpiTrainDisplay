# RGB Matrix Train Departure Board
You can build you're own train departure board showing the next train, where it's calling, and the following two departures

# Motivation
Being a life-long train fan I've always wanted my own departure board... and as a regular commuter I wanted one which shows my usual route.
Over the years I've looked at flip-dot displays and various other mechanisms and then saw dot-matrix displays become available for purchase.
However the price was offputting and I thought "I wonder if I could build one for less".
And here we are.

# This documentation is evolving
In this initial version (10th Feb 2025) I'll just give the high-level view and will give more detail in due course.
Happy to help with any questions - best to do that via github so others can share the wisdom!

# Hardware
## Some RGB matrix boards.  
Ali Express is your friend.  I purchased three P2, 5V, 128*64 pixel colour modules with a HUB75E interface.
There are a myriad selles on Ali Express and elsewhere. I suspect there's little to differentiate between offerings.

## A Raspberry Pi
I used a Raspberry Pi 4 which was unloved and needed a new purpose. Note that the RGB matrix library doesn't yet work with a Pi 5.
I put a regular install on it and ran it headless, although I'm aware there's benefit from using a cut-down OS.

## An RGB matrix-driver 
I used an [Adafruit RGB Matrix Bonnet for Raspberry Pi](https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi) which certainly did the trick.
If there are other mechanisms which folk can recommend then let me know!

## 16 way ribbon cable
Some sellers provide this with the matrix panels. If you're making your own then the mantra is "Short Is Good".

## Some power!
I used a bench-top adjustable power supply to provide 5v for the matrix boards and the Raspberry Pi (which you can power via the bonnet).
The nature of the panels means that current can vary quite wildly.  For the departure board it runs at about 1.5 to 2.0 Amps.
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

## Software for the Pi ##
I'm going to assume you've got the Raspberry Pi up and running. 

Log in, download and install the goodness from the [hzeller rpi-rgb-led-matrix repository](https://github.com/hzeller/rpi-rgb-led-matrix).

I can't recall the options at installation time, but I went for the 'performance' option (or similar named).

Build and run the demos and have fun! 

A pre-emptive read of the [Troubleshooting section)](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#troubleshooting) will help you get ahead of issues.

# So what about the train data? #
The source of the data is the [Live Departure Boards Web Service (LDBWS / OpenLDBWS)](https://lite.realtime.nationalrail.co.uk/OpenLDBWS/)

You'll need to register to be able to access the data - this is pretty straightforward via the [Open LDBWS Registration page](https://realtime.nationalrail.co.uk/OpenLDBWSRegistration).  Make a note of the key (although they email it to you aswell).

I'll include more links later - there is a wealth of information out there on train data and a mind-boggling amount of information you can get access too (although it's not all free).  The [Open Rail Data wiki](https://wiki.openraildata.com/index.php/Main_Page) is an awesome resource as is the list of [Open Rail data repositories](https://github.com/openraildata) here on Github.

**However** the data is only available from Network Rail via SOAP.  Not my forte.  If you're a Python programmer then you can go to the [Open Rail Data repository](https://github.com/openraildata) however, being an old lag, C++ is my thing.

## Huxley2 to the rescue ##
Huxley2 is a cross-platform JSON proxy for the GB railway Live Departure Boards SOAP API and is [here on Github](https://github.com/jpsingleton/Huxley2). More detail on [this site which includes a demo server](https://huxley2.azurewebsites.net)

I've been working on getting this to run locally on the Pi - a work in progress - however the instructions for installing this on Azure are fabulous are are available [on this blog post](https://unop.uk/huxley-2-release).

After installing this I created and API endpoint URL so have my own personal API server for the data.  James Singleton - the author of Huxley2 - is a god.

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
fontPath=/home/display/rpi-rgb-led-matrix/fonts/9x18.bdf

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

You've got four options

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
With thanks to:
[James Singleton's Huxley2](https://github.com/jpsingleton/Huxley2)
[Hzeller RPI RGB LED Matrix](https://github.com/hzeller/rpi-rgb-led-matrix)
[Nlohmann JSON for modern C++](https://github.com/nlohmann/json)
