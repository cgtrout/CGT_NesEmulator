# CGT_NesEmulator

![image](https://user-images.githubusercontent.com/6323654/222131333-edcbfb2c-befa-4635-b027-d0a8c9ff26d2.png)
> (top right panel is "pattern table" (video memory view essentially), and bottom right panel is current palette)

---

## Basic NES Emulator 
#### A learning project not designed to be feature competitive with other emulators.  Written in C++ for Windows.

Much of the code was written several years ago, and does not conform to the latest C++ standards in all cases.  However, recently I have spent some time updating the project to be more conformant in several areas.

The goal for this project was simply to learn how to build an emulator and to get it to run several games for the NES.  I succeeded in this goal, as it runs several games such as:

* Super Mario Bros
* Excite Bike
* Ice Hockey
* Contra
* Megaman
* Metal Gear

Features a full hex viewer and step debugger:

![image](https://user-images.githubusercontent.com/6323654/222130609-10ab1074-1e87-47b7-a39d-82cc2071bd7c.png)

It has limited support for all the "mappers" available, so many games will not even load.  The sound is partially implemented and has some issues in its current state.

#### Build instructions (Visual Studio)
Place SDL2 library files (win32) in CGT_NesEmulator\SDL2
Place imgui files in /src/imgui directory
Place implot files in /src/implot directory