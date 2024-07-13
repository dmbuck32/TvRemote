# TV Remote

This is a simulated low-cost 2-button television remote control.

## Requirements

This remote control has the following requirements and design constraints:

- It is required to only have 2 buttons for user input.
- Must be able to power on and off the TV
- Must be able to turn the volume up and down
- Must be able to turn the brightness up and down
- Must be able to change the channel

### Assumptions

The following assumptions are present in this application:

- The power-off functionality should be accessible at all times. 
- The volume range is from [0, 100] inclusive
- The volume change logic will bound the input to this range
- The brightness range is from [0, 100] inclusive
- The brightness change logic will bound the input to this range
- The channel range is from [1, 256] inclusive
- The channel change logic will wrap around (i.e channel up at 256 will go to 1)
- The long-press timeout is 800 ms
- The debounce timeout is 100 ms

## Functional Description

When the user starts the application, the TV will be in a powered off state. The user has 2 keys that they can press to simulate button actions on the remote. These buttons are `B1` and `B2` which map to `w` and `s` on the keyboard respectively. In order to turn on the TV, the user must long-press the `B1` button which will turn the TV on. This will put the TV in the [volume change](#volume-change) mode. At any point, the user may long-press the `B1` button to turn the TV off.

### Volume Change

In this mode, the user can short-press the `B1` button to increase the volume and short-press the `B2` button to decrease the volume. While in this mode the user can long-press the `B2` button switch to the next mode, which is the [channel select](#channel-select) mode. 

### Channel Select

In this mode, the user can short-press the `B1` button to swap to the next higher channel and short-press the `B2` button to swap to the next lower channel. While in this mode the user can long-press the `B2` button switch to the next mode, which is the [brightness change](#brightness-change) mode. 

### Brightness Change

In this mode, the user can short-press the `B1` button to increase the brightness and short-press the `B2` button to decrease the brightness. While in this mode the user can long-press the `B2` button switch to the next mode, which is the [volume change](#volume-change) mode. 

## Application design diagram

The TV Remote comprises of 2 state machines:

<img src="./TvRemote.drawio.svg">

NOTE: The documentation was crafted to be compatible with the [StateSmith](https://github.com/StateSmith/StateSmith) tool.