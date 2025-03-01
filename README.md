# RGB Matrix Train Departure Board
Rather than paying rather a lot of money, you can build you're own RGB matrix train departure board.

Currently configured to show:
* The next train from your favourite station to your favourite destination
* Calling points (with optional estimated time of departure)
* The following two departures
* Optional display of delay reasons and other messages

All configured using a light-weight web-page you can access on any device.

Components are a Raspberry Pi, an interface (Adafruit bonnet), some RGB matrices and a power-supply.

# Motivation
Being a life-long train fan I've always wanted my own departure board, and as a regular commuter one which shows my usual route.

Over the years I've looked at flip-dot displays and various other mechanisms and then saw LED dot-matrix displays become available for purchase.

These look amazing... however the price is staggering, as is the monthly-subscription some require for what is freely available data.

My thought -  _"How hard can it be"_.

And here we are - the Raspberry Pi RGB matrix departure board and companion [Raspberry Pi fork of Huxley2](https://github.com/jonmorrissmith/jonms-Huxley2).

# This documentation is evolving
In this second version (1st March) there's still gaps, however it's a lot more complete than it was.

Happy to help with any questions - best to do that via github so others can share the wisdom.

Updates, changes, modifications and enhancements welcome!

# Hardware
## The TL:DR
* [Three P2, 5V, 128*64 pixel colour modules with a HUB75E interface from Ali Express](https://www.aliexpress.com/item/32913063042.html)
* [A 1GB Raspberry Pi 4 from Pimoroni](https://shop.pimoroni.com/products/raspberry-pi-4?variant=31856486416467)
* [Adafruit RGB Matrix Bonnet for Raspberry Pi from Pimoroni](https://shop.pimoroni.com/products/adafruit-rgb-matrix-bonnet-for-raspberry-pi?variant=2257849155594)
* A 5V power-supply capable of delivering at least 5 Amps

## Some RGB matrix boards.
I purchased [three P2, 5V, 128*64 pixel colour modules with a HUB75E interface from Ali Express](https://www.aliexpress.com/item/32913063042.html).

Three is a good size and three is limit for a chain of panels with the matrix library I used.  However you could have up to three rows of three.

There are a myriad sellers on Ali Express and elsewhere. I suspect there's little to differentiate between offerings.

## A Raspberry Pi
You could purchase [a 1GB Raspberry Pi 4 from Pimoroni](https://shop.pimoroni.com/products/raspberry-pi-4?variant=31856486416467) - I'm not plugging Pimoroni, it's just that they also stock the RGB Matrix Bonnet so you can save on postage.

I used a Raspberry Pi 4 which was unloved and needed a new purpose. 

**Note** that the RGB matrix library doesn't yet work with a Pi 5. I'll update when it does as the increased power of the Pi 5 will be welcome!

## An RGB matrix-driver 
This project was built with an [Adafruit RGB Matrix Bonnet for Raspberry Pi from Pimoroni](https://shop.pimoroni.com/products/adafruit-rgb-matrix-bonnet-for-raspberry-pi?variant=2257849155594)

Specification on the [Adafruit site](https://learn.adafruit.com/adafruit-rgb-matrix-bonnet-for-raspberry-pi).  

If there are other mechanisms which folk can recommend then let me know!  

## 16 way ribbon cable
Most sellers provide these with the matrix panels. If you're making your own then your mantra has to be "Short Is Good".

## Some power!
I used a bench-top adjustable power supply to provide 5v and up to 5A for the matrix boards and the Raspberry Pi.

The nature of the panels means that current can vary quite wildly.  For the departure board with three panels it runs at about 1.5A to 2.0A.

I'd recommend getting something substantial as the "all pixels on" power is just around 3.5A with my configuration... and let's face it, you're going to want to play about with it!

The [Words about power](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/wiring.md#a-word-about-power) in the RGB Matrix documentation is worth a read.

## Anything else on hardware?
I'd highly recommend reading the detail on the [hzeller rpi-rgb-led-matrix - Let's Do It!](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#lets-do-it) documentation as it answers every question you can think of.
It's an awesome resource and fabulous software. More of that below!

## Putting it all together
Details below are for the Adafruit Bonnet

For other adapters see the [RGB Matrix - wiring](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/wiring.md) documentation.

### Improving Flicker ###
As highlighted [here](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#improving-flicker) in the RGB documentation, an optional change to connect pins 4 and 18 on the hat/bonnet.

I did this by soldering a row of pins to the bonnet and using a modified connector to make it easy to add/remove the modification.
![Picture of pins and connector](https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board/blob/main/Bonnet_Jumpers.jpg)

### Address E pad modification ###
As highlighted [here](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#new-adafruit-rgb-matrix-hat-with-address-e-pads) in the RBG documentation, a change required for the size and type of panels I used.
![Picture of soldere blob connecting the two pads](https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board/blob/main/Bonnet_Soldering.jpg)

As a side note, I had to chop some pins off a connector on the Pi as they hit the bonnet - I suspect I may regret that at some point when the Pi gets used for something else.

### Connecting to the panels ###

Only thing to watch for is to make sure that your ribbon cables are short and connect from 'Outputs' to 'Inputs' (should be labelled or have an arrow on the matrix boards

### Connecting the power ###

Noting the [advice about power](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/wiring.md#a-word-about-power) in the RGB Matrix documentation.

Connect to the panels and the 5v screw-connectors on the Adafruit bonnet to power to the Raspberry Pi.

As noted in the documentation, the Raspberry Pi doesn't have enough juice to power the panels.

# Setting up the Raspberry Pi #

## Install the OS ##

There are more tutorials than you can shake a stick at on how to install an OS on a Raspberry Pi.

I went for the 'OS Lite (64bit)' maximise CPU cycles the matrix driver can use by having a cut-down OS. Set up ssh and Wifi in the Raspberry Pi Imager tool.

Once installed there was the usual upgrade/update and disable/uninstall anything unecessary.

I would strongly recommend following [using a minimal raspbian distribution](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#use-minimal-raspbian-distribution) for more detail.

What is described what I did in Feb 2025. Detail in the rpi-rgb-matrix repository will be the most recent recommendations.

* Set `dtparam=audio=off` in `/boot/firmware/config.txt`
* Add `isolcpus=3` **to the end** of the parameters in `/boot/firmware/cmdline.txt` to isolate a CPU.
* Example - `console=serial0,115200 console=tty1 root=PARTUUID=9f23843a-02 rootfstype=ext4 fsck.repair=yes rootwait cfg80211.ieee80211_regdom=GB isolcpus=3`
* Remove unecessary services with `sudo apt-get remove bluez bluez-firmware pi-bluetooth triggerhappy pigpio`

run `lsmod` and to check for the snd_bcm2835 module.

Example:
```
$lsmod | grep snd_bcm2835
snd_bcm2835            24576  0
snd_pcm               139264  5 snd_bcm2835,snd_soc_hdmi_codec,snd_compress,snd_soc_core,snd_pcm_dmaengine
snd                   110592  6 snd_bcm2835,snd_soc_hdmi_codec,snd_timer,snd_compress,snd_soc_core,snd_pcm
```
If it's there then blacklist it:
```
cat <<EOF | sudo tee /etc/modprobe.d/blacklist-rgb-matrix.conf
blacklist snd_bcm2835
EOF

sudo update-initramfs -u
```
Then reboot - `sudo reboot` to get a nice clean and sparkly install.

**Note** At the rsk of stating the obvious, when adding options to `cmdline.txt` put them on the same line as the existing arguments. No newlines allowed.

## Installing packages you'll need
In no particular order:
* Git - `sudo apt install git`.
* JSON for modern C++ - `sudo apt install nlohmann-json3-dev`.
* curl (should be there already) - `sudo apt install libcurl4`.
* curl C++ wrappers - `sudo apt install libcurlpp-dev`.


## Install the RGB Matrix Software ##

Take a clone of the RPI RGB Matrix repository - `git clone https://github.com/hzeller/rpi-rgb-led-matrix`.

Compile the library
```
cd rpi-rgb-matrix
make
```
A pre-emptive read of the [Troubleshooting section](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#troubleshooting) will help you get ahead of issues.

Enjoy the demos in the `examples-api-use` directory.
If you've got the configuration described here then you can start with this:
```
sudo ./demo -D9 --led-rows=64 --led-cols=128 --led-chain=3 --led-gpio-mapping=adafruit-hat
```
Have fun!

# So what about the train data? #
The source of the data is the [Live Departure Boards Web Service (LDBWS / OpenLDBWS)](https://lite.realtime.nationalrail.co.uk/OpenLDBWS/)

You'll need to register to be able to access the data - this is pretty straightforward via the [Open LDBWS Registration page](https://realtime.nationalrail.co.uk/OpenLDBWSRegistration).  Make a note of the key (although they email it to you aswell).

I'll include more links later - there is a wealth of information out there on train data and a mind-boggling amount of information you can get access too (although it's not all free).  The [Open Rail Data wiki](https://wiki.openraildata.com/index.php/Main_Page) is an awesome resource as is the list of [Open Rail data repositories](https://github.com/openraildata) here on Github.

**However** the data is only available from Network Rail via SOAP.  Not my forte.  If you're a Python programmer then you can go to the [Open Rail Data repository](https://github.com/openraildata) however, being an old lag and liking the enhanced performance of a compiled executable, C++ was the way to go.  I'm not about to try to ingest SOAP with C++.

## Huxley2 to the rescue ##
Huxley2 is a cross-platform JSON proxy for the GB railway Live Departure Boards SOAP API and is [here on Github](https://github.com/jpsingleton/Huxley2). More detail on [this site which includes a demo server](https://huxley2.azurewebsites.net).

I've create a fork of Huxley2 with modifications for running locally on a raspberry Pi - [Huxley 2 for Raspberry Pi](https://github.com/jonmorrissmith/jonms-Huxley2).

However you can also install on Azure. The instructions for this are fabulous are are available [on this blog post](https://unop.uk/huxley-2-release).

With the monochrome cofiguration below a local-install doesn't affect performance.

If you go for colour (or increase `led-pwm-bits` from 1) then you may see fickering when updates happen.

# The final Steps #

## Installing the RGB Matrix Train Departure Board software ##
From this repository - `git clone https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board`

Edit the configuration in config.h in to your default settings

_Note_ - these have been optimised for black and white and speed:
```
class Config {
private:
    std::map<std::string, std::string> settings;
    
    const std::map<std::string, std::string> defaults = {
        {"from", "XXX"},                                               // Your default origin
        {"to", "XXX"},                                                 // Your default destination
        {"APIURL", "https://XXX"},                                     // URL for your train data
        {"fontPath", "/home/XXX/rpi-rgb-led-matrix/fonts/9x18.bdf"},   // font directory
        {"scroll_slowdown_sleep_ms", "15"},                            // Speed of scroll for calling points
        {"refresh_interval_seconds", "60"},                            // How often the data refreshes
        {"matrixcols", "128"},                                         // number of LED columns in a matrix panel
        {"matrixrows", "64"},                                          // number of LED rows in a matrix panel
        {"matrixchain_length", "3"},                                   // number of panels chained together
        {"matrixparallel", "1"},                                       // number of rows of panels
        {"matrixhardware_mapping", "adafruit-hat-pwm"},                // hardware configuration
        {"gpio_slowdown", "4"},                                        // basic performance tuning
        {"first_line_y", "18"},                                        // where the first line of text will appear
        {"second_line_y", "38"},                                       // where the second line of text will appear
        {"third_line_y", "58"},                                        // where the third line of text will appear
        {"third_line_refresh_seconds", "10"},                          // How often the 3rd line refreshes
        {"ShowCallingPointETD", "Yes"},                                // show the estimated time of departure for each calling point.
        {"ShowMessages", "Yes"},                                       // show any messages on the third row in adddition to the 2nd and 3rd departures
        
        // RGB Matrix defaults - see https://github.com/hzeller/rpi-rgb-led-matrix for details
        {"led-multiplexing", "0"},
        {"led-pixel-mapper", ""},
        {"led-pwm-bits", "1"},
        {"led-brightness", "100"},
        {"led-scan-mode", "0"},
        {"led-row-addr-type", "0"},
        {"led-show-refresh", "false"},
        {"led-limit-refresh", "0"},
        {"led-inverse", "false"},
        {"led-rgb-sequence", "RGB"},
        {"led-pwm-lsb-nanoseconds", "130"},
        {"led-pwm-dither-bits", "0"},
        {"led-no-hardware-pulse", "false"},
        {"led-panel-type", ""},
        {"led-daemon", "false"},
        {"led-no-drop-privs", "false"},
        {"led-drop-priv-user", "daemon"},
        {"led-drop-priv-group", "daemon"}
    };
```

And compile (may take a while)!

`g++ -O3 -std=c++11 traindisplay.cpp -o traindisplay -lrgbmatrix -lcurl -lpthread -I/home/<your path>/rpi-rgb-led-matrix/include -L/home/<your path>/rpi-rgb-led-matrix/lib`

# And finally Cyril... and finally Esther #
## Permissions for your home directory ##
You need to make your home directory world readable - you need to run the executable as root, and root needs permissions.
```
cd
cd ..
ls
```
and you'll see your home directory.
Make this world-readable `chmod 755 <home directory name>`

## Tweak your configuration file ##
This can be used to over-ride settings in the configuration class and other customisations.

Detail of the RGB Matrix library parameters is available [here in the RGB Matrix documentation](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md#changing-parameters-via-command-line-flags)

```
# Train Display Configuration File
# Lines starting with # are comments

# Station codes
from=XXX
to=XXX

# API Configuration
APIURL=https://XXX

# Display font configuration
fontPath=/home/XXX/rpi-rgb-led-matrix/fonts/9x18.bdf

# Timing parameters (in milliseconds/seconds)
scroll_slowdown_sleep_ms=15
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

# Feature configuration
ShowCallingPointETD=Yes
ShowMessages=Yes

# RGB Matrix Library Configuration Parameters
# Please do not put comments on the same line as values

# Panel type settings
led-multiplexing=0
led-pixel-mapper=
led-panel-type=

# Display quality settings
led-pwm-bits=1
led-brightness=100
led-scan-mode=0
led-row-addr-type=0

# Refresh rate settings
led-show-refresh=false
led-limit-refresh=0

# Color settings
led-inverse=false
led-rgb-sequence=RGB

# Advanced PWM settings
led-pwm-lsb-nanoseconds=130
led-pwm-dither-bits=0
led-no-hardware-pulse=false

# Privilege/daemon settings
led-daemon=false
led-no-drop-privs=false
led-drop-priv-user=daemon
led-drop-priv-group=daemon
```
# Enjoy Your Departure Board #

Two options available

## Command Line Operation ##

Four options all of which also support a '-d' option for debugging information

Use the default configuration in the executable

`sudo ./traindisplay`

Use the default configuration in the executable and specify origin and destination

`sudo ./traindisplay SAC STP`

Use your configuration file

`sudo ./traindisplay -f <config file>`

Combination of the above

`sudo ./traindisplay SAC STP -f <config file>`

## Lightweight User Interface ##

Good for parameter tweaking and easy operation

### Set up the User Interface ###

Edit the `ui-config.txt` file to reflect your installation (probably just need to replace XXX with your home directory)
```
Executable_directory - /home/XXX/RGB_Matrix_Train_Departure_Board
Executable_command_line - sudo /home/XXX/RGB_Matrix_Train_Departure_Board/traindisplay -f /home/XXX/RGB_Matrix_Train_Departure_Board/config.txt
Port - 8080
Defaults_Config_file - /home/XXX/RGB_Matrix_Train_Departure_Board/default-config.txt
```
The `default-config.txt` file gives you something to revert to if your tweaking goes astray!

Just copy your existing configuration file.
```
cp config.txt default-config.txt
```

Finally make sure the `config_server.py` script is executable
```
chmod 755 config_server.py
```
and then start the configuration server
```
./config_server.py
```
You should see something like:
```
Starting server on port 8080...
```
which indicates you can do away with manual configuration editing and enjoy the minimal UI experience!

### Connect to the User Interface ###
With the above configuration you can tweak away to your hearts content at `http://<IP address of your Raspberry Pi>:8080`

Enjoy!

### Troubleshooting ###
I suspect that most of this will centre around the RGB display library:
* [Changing parameters](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md#changing-parameters-via-command-line-flags)
* [Troubleshooting](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md#troubleshooting)

Otherwise feel free to raise an Issue here on github and I'll try to help!

# Huge thanks to... #

[James Singleton's Huxley2](https://github.com/jpsingleton/Huxley2)

[Hzeller RPI RGB LED Matrix](https://github.com/hzeller/rpi-rgb-led-matrix)

[Nlohmann JSON for modern C++](https://github.com/nlohmann/json)
