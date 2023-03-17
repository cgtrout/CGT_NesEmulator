# CGT_NesEmulator

![image](https://user-images.githubusercontent.com/6323654/225933069-df532592-6ee0-4d40-a3db-af317ef6c888.png)
---

## Basic NES Emulator 
#### A learning project not designed to be feature competitive with other emulators.  Written in C++ for Windows.  Recently just ported to SDL, so eventually a cross platform version will be released.

Much of the code was written several years ago, and does not conform to the latest C++ standards in all cases.  However, recently I have spent some time updating the project to be more conformant in several areas.

The goal for this project was simply to learn how to build an emulator and to get it to run several games for the NES.  I succeeded in this goal, as it runs several games such as:

* Super Mario Bros
* Excite Bike
* Ice Hockey
* Contra
* Megaman

Features a full hex viewer and step debugger:

![image](https://user-images.githubusercontent.com/6323654/225933415-f7a3688e-91ac-40e0-a062-c4e4e876312a.png)

It has limited support for all the "mappers" available, so many games will not even load.  The sound is partially implemented and has some issues in its current state.

#### Build instructions (Visual Studio)
Place SDL2 library files (win32) in CGT_NesEmulator\SDL2
Place all imgui files directly in /src/imgui directory (no subdirs)
Place implot files in /src/implot directory
