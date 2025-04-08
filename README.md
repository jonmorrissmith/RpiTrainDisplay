# RGB Matrix Train Departure Board
A way to build your own RGB matrix train departure board powered by a Raspberry Pi.

Configurable via a light-weight web-page or the command line.

You can select:
* All trains from a station
* All trains from a station on a specified platform
* All trains from a station to a specified destination
* Or combinations of the above
* Whether to show message.
* Frequency of display update
* Many other options...
  
Default display:
* The next train with calling points, operator and formation
* The following two departures - destination and departure times
* Any delay-related messages and cancellation reasons
* Operator and number of coaches for 1st departure
  
Options to display
* Location
* Departure platform
* Estinated departure time for calling points
* Any National Rail messages for the station

Components are a Raspberry Pi 4, a matrix-adapter (Adafruit RGB matrix bonnet), some RGB matrices and a power-supply.

There are some limitations with using this hardware which I suspect don't apply to commercially-available units.

However for the cost it's a great result!

# Motivation
Being a life-long train fan I've always wanted my own departure board, and as a regular commuter one which shows my usual route.

Over the years I've looked at flip-dot displays and various other types and saw LED dot-matrix displays become available for purchase.

These look amazing... however the price is offputting, as is the monthly-subscription some require.

My thought -  _"How hard can it be"_.

And here we are.

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
You could purchase [a 1GB Raspberry Pi 4 from Pimoroni](https://shop.pimoroni.com/products/raspberry-pi-4?variant=31856486416467) - I'm not plugging Pimoroni, it's just they also stock the Bonnet.

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
![Picture of pins and connector](https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board/blob/main/Images/Bonnet_Jumpers.jpg)

### Address E pad modification ###
As highlighted [here](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#new-adafruit-rgb-matrix-hat-with-address-e-pads) in the RBG documentation, a change required for the size and type of panels I used.
![Picture of soldere blob connecting the two pads](https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board/blob/main/Images/Bonnet_Soldering.jpg)

As a side note, I had to chop some pins off a connector on the Pi as they hit the bonnet - I suspect I may regret that at some point when the Pi gets used for something else.

### Connecting to the panels ###

Only thing to watch for is to make sure that your ribbon cables are short and connect from 'Outputs' to 'Inputs' (should be labelled or have an arrow on the matrix boards

### Connecting the power ###

Noting the [advice about power](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/wiring.md#a-word-about-power) in the RGB Matrix documentation.

Connect to the panels and the 5v screw-connectors on the Adafruit bonnet to power to the Raspberry Pi.

As noted in the documentation, the Raspberry Pi doesn't have enough juice to power the panels.

### Joining the panels together ###

A chance to be creative!  

To get you started I've provided a very basic solution to 3D print joiners.

You can set the key parameters to create a joiner specific to your panels.

More detail in the [Joiner Directory](https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board/blob/main/Joiner/Readme.md)

# Setting up the Raspberry Pi #

## Install the OS ##

There are more tutorials than you can shake a stick at on how to install an OS on a Raspberry Pi.

Use the 'OS Lite (64bit)' to maximise CPU cycles the matrix driver can use. Set up ssh and Wifi in the Raspberry Pi Imager tool.

Once installed there was the usual upgrade/update and disable/uninstall anything unecessary.

I would strongly recommend following [using a minimal raspbian distribution](https://github.com/hzeller/rpi-rgb-led-matrix?tab=readme-ov-file#use-minimal-raspbian-distribution) for more detail.

What is described what I did in March 2025. Detail in the rpi-rgb-matrix repository will be the most recent recommendations.

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
There are two options - as far as I can tell the data available via both feeds is the same.

# Rail Data Marketplace
This is the easiest of the two options. 

## Sign up 
Go to [raildata.org.uk](https://raildata.org.uk) and [register](https://raildata.org.uk/registerPartner).  

It's likely you'll fall into the **Individual not affiliated with a company** category.

Once you've got your acccount set up you need to subscribe to the data feed.

## Subscribe to Live Departure Board data
The easiest way to find this is via the [Data Products Catalogue](https://raildata.org.uk/dashboard/dataProducts).  From here search for **Live Departure Board**.

The one you need is published by the Rail Delivery Group.  

Don't use the 'Staff Version' as that isn't compatible with this software.

## Get your API Key
The landing page for the Live Departure Board data has a **Specifications tab**.

On that tab you'll find the **API access credentials** - you need the **Consumer key**.  

Press the `Copy` button and drop it into the `APIKey` field in `config.txt`.

**Note** Make sure there are no spaces in the configuration - `APIkey=123456aVeryLongString`.

**Note** Be sure to set `Rail_Data_Marketplace=Yes` in the configuration file. 

# Network Rail
This is more complex as the API uses SOAP - not ideal for C++ - so a proxy is required.

[Huxley2](https://github.com/jpsingleton/Huxley2) is a cross-platform JSON proxy for the GB railway Live Departure Boards SOAP API. 

More detail on [this site which includes a demo server](https://huxley2.azurewebsites.net).

I've created a fork of Huxley2 for running locally on a raspberry Pi - [Huxley 2 for Raspberry Pi](https://github.com/jonmorrissmith/jonms-Huxley2) - or you can run [on Azure](https://unop.uk/huxley-2-release).

With the monochrome configuration below a install on the Raspberry Pi used for the RGB matrix doesn't affect performance.

# The final Steps #

## Installing the RGB Matrix Train Departure Board software ##
Download and build from this repository
```
git clone https://github.com/jonmorrissmith/RpiTrainDisplay
cd RpiTrainDisplay
chmod 755 setup.sh
./setup.sh
```
This will build the software, create config files and ensure permissions are correctly set on directories.

### And finally Cyril... and finally Esther ###
Start the UI server
```
./run.sh
```
You should see:
```
Starting server on port 80...
```
Which means you can go to `http://<IP address of your Raspberry Pi>` and start using your display!

# First Time Use - Basic Configuration
Set the following in the UI

## Location and Destination
```
from       \\ The station whose departures you want to show
to         \\ Leave blank for all departures or populate for a specific destination
platform   \\ Leave blank for all platforms or populate for a specific platform
```
## Additional Information
```
ShowCallingPointETD   \\ If set to Yes will display departure times after each calling point
ShowMessages          \\ If set to Yes will display Network Rail message for your departure station
ShowPlatforms         \\ If set to Yes will display the platform for the departures
ShowLocation          \\ If set ('from') will display at the bottom (alternate with Messages)
```
## API and Font Configuration
```
APIURL                   \\ full URL for Network Rail data (i.e with the https:// header)
APIkey                   \\ Any API key you need to use (applied using x-apikey:)
Rail_Data_Marketplace    \\ If set to Yes will use the Rail Data Marketplace URL (and over-ride APIURL).
```
## Font configuration
```
fontPath  \\ Path to fonts - you can use the matrix package (/home/<your username>/rpi-rgb-led-matrix/fonts/7x14.bdf)
```
## Timing Configuration
```
scroll_slowdown_sleep_ms=15     \\ Lower the number, the faster the scroll
refresh_interval_seconds=60     \\ How often the API is called to refresh the train data
third_line_refresh_seconds=10   \\ How often the third line switches between 2nd and 3rd departure
Message_Refresh_interval=20     \\ How often any Network Rail messages are shown
ETD_coach_refresh_seconds=4     \\ How often the top right switches between ETD and number of coaches
```

## Hardware Configuration
```
matrixcols=128                             \\ Number of columns in an LED matrix panel
matrixrows=64                              \\ Number of rows in an LED matrix panel
matrixchain_length=3                       \\ Number of panels you've got chained together
matrixparallel=1                           \\ Number of chains you've got running in parallel
matrixhardware_mapping=adafruit-hat-pwm    \\ The hardware adapter you're using to connect the Pi to the LED matrix
gpio_slowdown=2                            \\ Sometimes the Pi is too fast for the matrix.  Fiddle with this to get the right setting.
```

## Display layout configuration (vertical positions)
```
first_line_y=12     \\ pixel-row for the first line of text
second_line_y=29    \\ pixel-row for the second line of text
third_line_y=46     \\ pixel-row for the third line of text
fourth_line_y=62    \\ pixel-row for the fourth line of text
```

## Once you're happy with your configuration
Scroll to the bottom and click on **Save and Restart**.

This saves your configuration (to 'config.txt') and (re)starts the display.

You can **Save as Default** and **Rest to Default** to allow you to revert changes.

# Advanced Configuration
It's unlikely that you'll need to change these, but they're available if you need to.

Detail of the parameters is available [in the RGB Matrix documentation](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md#changing-parameters-via-command-line-flags).
```
led-multiplexing
led-pixel-mapper
led-pwm-bits
led-brightness
led-scan-mode
led-row-addr-type
led-show-refresh
led-limit-refresh
led-inverse
led-rgb-sequence
led-pwm-lsb-nanoseconds
led-pwm-dither-bits
led-no-hardware-pulse
led-panel-type
led-daemon
led-no-drop-privs
led-drop-priv-user
led-drop-priv-group
```

# Additional Information

## Making output less verbose

Simply remove the `-d` option from the `Executable_command_line` line in `ui-config.txt`

## Running on another port

Change the port in `ui-config.txt`.

Note that the executable needs root to access the RGB matrix.

## Setting your configuration in the code 

You can edit `config.h` to hard-code values for parameters which are in `config.txt`

If you do this then it's easiest to run
```
make clean
make
```
which will recompile the executable with your defaults.

## Command Line Operation ##

Five options all of which also support a '-d' option for debugging information

Use the hard-coded configuration in the executable

`sudo ./traindisplay`

Use the hard-coded configuration in the executable and specify origin

`sudo ./traindisplay SAC`

Use the hard-coded configuration in the executable and specify origin and destination

`sudo ./traindisplay SAC STP`

Use the configuration file

`sudo ./traindisplay -f <config file>`

Combination of the above

`sudo ./traindisplay SAC STP -f <config file> -d`

# Troubleshooting #
**RGB Matrix Issues**
* [Changing parameters](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md#changing-parameters-via-command-line-flags)
* [Troubleshooting](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/README.md#troubleshooting)

**Software Issues**

The most fragile part of the software is the parser.  

You can test this using software included in the distribution built using `make parser_test`.

Running `traindisplay` with the debug flag dumps results from API calls into the tmp directory.

You can test the parser against this data using `./parser_test -data /tmp/traindisplay_payload.json`.

Other options are available:
```
Usage: ./parser_test -data <string> [-platform <string>] [-clean <string>] [-f <string>] [-debug <string>]
-data <filename.json>   json data file
-platform <string>      select a platform
-clean <y/n>            remove whitespace
-f <filename.txt>       file (not currently in use)
-debug <y/n>            switch on debug info in the parser code
```
Feel free to raise an Issue here and I'll try to help - attach your `config.txt` and `debug.txt` created using 
```
./parser_test -data /tmp/traindisplay_payload.json -debug y > debug.txt
```


# Huge thanks to... #

[James Singleton's Huxley2](https://github.com/jpsingleton/Huxley2)

[Hzeller RPI RGB LED Matrix](https://github.com/hzeller/rpi-rgb-led-matrix)

[Nlohmann JSON for modern C++](https://github.com/nlohmann/json)
