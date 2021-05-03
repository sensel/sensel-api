The "SenselDisableSerialBlock" executables are simple apps that allow you to use the API and "regular" output like MIDI from the Morph at the same time. We have found this particularly useful when using the Max objects to do cool visualizations while playing overlays with soft-synths. 

Basic Instructions:

* Connect your Morph via USB to your computer. 
* Launch the program from the Finder or Explorer with a double-click. This opens a terminal window.
* You'll get feedback that a device is found (or not). 

Once the device is found, you'll know things are in good shape if the output reads:
```
Sensel Device: SM01174914963
Firmware Version: 0.19.298
Width: 230.000000mm
Height: 130.000000mm
Cols: 185
Rows: 105
Turn off Serial Block
Press Enter to exit example

```

Wait about 3 seconds or so before you press Enter, and you should now be able to use the API at the same time as the Overlays. 

On a Mac, you'll likely get a warning that the app is from an unidentified developer, since this is not signed. 
You can open your System Preferences -> Security and Privacy and allow the app to open.

This can be a bit of a finicky process and take a few tries. It's best to run this with the SenselApp closed and any patches with the 'sensel' objects in Max closed.

[Here is an example](https://youtu.be/m5euqiQ_4xk) of using the DisableSerialBlock in combination with Bitwig Studio and Cycling 74 Max (using jit.sensel).
