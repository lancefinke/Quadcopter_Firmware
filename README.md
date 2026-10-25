# Quadcopter_Firmware

#### Firmware code for 2 separate ESP32 modules that are used to control a remote controller and quadcopter drone.
---

## Requirements

In order run the project on Visual Studio Code, the PlatformIO IDE extension for Visual Studio Code needs to be installed on your machine.
Instructions for how to install the link the PlatformIO IDE extenstion can be found [here](https://platformio.org/platformio-ide).

The extension can be found under the extension tab as shown below.

<img width="274" height="269" alt="image" src="https://github.com/user-attachments/assets/fcef9e2f-e884-4a98-816f-c143d376f472" />

Once the PlatfromIO IDE has been installed, a new tab will show up on the left side bar. Click this tab and a drop down menu will appear.

Select **Open**

<img width="179" height="223" alt="image" src="https://github.com/user-attachments/assets/c5ca9462-85f9-4eef-a57c-b82415ec76c3" />


The home page of PlatformIO will open. Then within the PlatformIO home page, select the *Platforms* tab and then Select the *Embedded* tab.

In the search Bar search for *Espressif 32* the hardware used for this firmware runs on an ESP32 Devkit V1 and requires this package.

<img width="1009" height="593" alt="image" src="https://github.com/user-attachments/assets/01769e29-8469-4a01-8d70-4082b4424c6b" />

Once the package has been installed, the Github Repository can be cloned through the PlatfromIO IDE.

## Cloning the Repository

In the PlatfromIO IDE drop down menu, select the option *Clone Git Project*.

<img width="503" height="764" alt="image" src="https://github.com/user-attachments/assets/4792ef4b-d289-43b4-80fe-07994bea577e" />
<br><br>
<br><br>
A search bar on the top will appear. Copy and paste the http url for this repository.


<img width="1278" height="89" alt="image" src="https://github.com/user-attachments/assets/3e02863f-33f3-45c6-ab53-698a1a1eea9d" />

You will then be prompted to select a destination folder for this repository. Select your desired repository Location. 

#### IMPORTANT NOTE

If you select to open the file after cloning the repository, the project will not recognize the Espressif header files within the `src` files. The PlatfromIO project is 
located within the `Firmware` folder and this needs to be the project directory for the VS Code project. To resolve this issue, re-open the `Firmware` folder as the project directory.

Select `File` -> `Open Folder` -> `Navigate to the Repository` -> `Select the Firmware folder` -> `Open`

## Building and Uploading the project

To upload or build the `Drone_ESP.cpp` or `Controller_ESP.cpp` files, select the PlatformIO tab and find the `Porject Tasks` drop down.
Select either the `Drone` or `Controller` folder, open then `General` folder and select `build` to verify the file compiles without errors,
or `upload` to upload the code to the ESP32 microcontroller.


<img width="320" height="761" alt="image" src="https://github.com/user-attachments/assets/3a8527a0-dea5-444c-ba74-07e07068b331" />



