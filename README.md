# Add A New Sensor To mayako

This guide demonstrates how to integrate a new sensor into mayako-node. This should demonstrate the modular structure of the Arduino code. We pick the [VL53L0X Distance Sensor](https://registry.platformio.org/libraries/pololu/VL53L0X) and  work with the example code [Continuous Example](https://registry.platformio.org/libraries/pololu/VL53L0X/examples/Continuous/Continuous.ino) provided on platformio registry.

We assume that you fulfill the requirements of [mayako](https://vairasza.github.io/mayako-core/guides/Requirements.html#what-do-i-need-to-run-mayako-core).


## Step 1: Set Up Python Virtual Environment and Install mayako-core
   
To begin, we’ll create and set up a Python virtual environment for isolating the project dependencies. This is a basic process that should be done with any new Python project.

Ensure that the correct command for activating the virtual environment is provided for your OS (Step 2). The current command works for Linux Fedora. If you're using a different OS, you may need to adjust these commands accordingly.

1. Create a virtual environment:
    ```bash
    python -m venv ./venv
    ```

2. Activate the virutal environment:

    ```bash
    source ./venv/bin/activate
    ```

3. Install the mayako-core library:

    ```bash
    python -m pip install mayako-core
    ```


## Step 2: Create gui.py and Run the mayako GUI

1. Create a Python file named gui.py in your project directory with the following code:

    ```python
    #gui.py

    from mayako import Client

    client = Client()
    client.start_gui()
    ```

2. Run the GUI:

    ```bash
    python gui.py
    ```

3. After running this command, the mayako GUI will open.


## Step 3: Use the GUI to Get the Arduino Project

1. In the GUI, click on the "Get Arduino Project" button.

2. The "Add Device View" page will open. Here, the repository URL for Mayako-node code is already pre-filled. mayako assumes that you want to create and use an arduino folder in the current project directory to manage the mayako-node code.

3. Optional: To avoid API rate limits, paste your personal [Github API Token](docs.github.com/en/rest/repos/contents) into the appropriate field. Consuming all your [unauthenticated requests]([unauthenticated requests](https://docs.github.com/en/rest/using-the-rest-api/rate-limits-for-the-rest-api?apiVersion=2022-11-28)) per hour results in a failed attempt to download the project. A Github API Token can be generated in your accounts settings.

4. Click "Download" and wait for the "Success" message. Once the files are downloaded, they will be available for you to edit.

5. Exit the GUI by pressing the X button in the top-right corner.


## Step 4: Add the VL53L0X Library to the Project

In order to use the [VL53L0X sensor](https://shop.m5stack.com/products/tof-sensor-unit?srsltid=AfmBOorIPMUO_COTpHfhaTaFzAVTp2X966oAekhTJ3qfq9vMSvuYbl0w), we need to include the appropriate library.

1. Open the platformio.ini file located in the arduino folder of your project directory.

2. Add the following line under lib_deps to include the VL53L0X library:

    ```ini
    #platformio.ini
    [env:mayako]
    platform = espressif32
    board = m5stack-fire
    framework = arduino

    lib_deps = 
        pololu/VL53L0X          ; <== only change/add this line
    ```
    This will automatically download and link the library when the project is compiled for you. We recommend the platformio extension installed on VS Code to get autocompletion for the whole Arduino code.


## Step 5: Implement the Sensor Class

We will now create a new sensor class for the VL53L0X in the Arduino code. This involves creating header and implementation files for both the device and the model.

1. Create the Device Header File (DistanceSensor.h)

    1. Open the Arduino project in your favorite editor and navigate to the Devices folder.

    2. Create a new header file named DistanceSensor.h in this directory.

    3. You can copy and paste the content from Gyroscope.h as a template. Make sure to update the following:

        - Change the header guards and class name to match DistanceSensor.

        - Add the include statement for the VL53L0X library:

            ```cpp
            #include <VL53L0X.h>
            ```

    4. Define a private member for the sensor:

        ```cpp
        private:
            VL53L0X sensor;
        ```

2. Create the Model Header (DistanceModel.h)

    1. Navigate to the Models folder and create two new files: DistanceModel.h and DistanceModel.cpp.

    2. As before, use the GyroscopeModel.h and GyroscopeModel.cpp files as a template. Update the class name and modify it to reflect distance data:

        - The distance data will be represented as an integer:

            ```cpp
            int distance;
            ```


## Step 6: Create the Distance Sensor Implementation

With the headers in place, let’s move on to implementing the DistanceSensor class.

1. Initialize the Sensor

    Navigate to the Devices folder associated with your board (e.g., M5) and open your DistanceSensor.cpp file. In the init method, initialize the sensor with the following code:

    ```cpp
    this->sensor.setTimeout(500);
    if (!this->sensor.init()) {
        this->logger->error("Failed to detect or initialize the distance sensor");
    }
    this->sensor.startContinuous();
    ```

    This code sets a timeout, attempts to initialize the sensor, and logs an error if initialization fails. The sensor is set to continuous mode, so it reads data in the background.

    You can verify this step by running the following command to monitor the serial output:

    ```bash
    pio device monitor -b 115200
    ```

2. Fill the readData() Method

    The readData() method is responsible for collecting data from the sensor and returning it to the DeviceManager. Here’s how you can implement it:

    ```cpp

    String DistanceSensor::readData() {
        DistanceModel model = DistanceModel();
        int range = sensor.readRangeSingleMillimeters();
        model.range = range;
        this->appendMetaData(model);
        return this->toJSON(model);
    }
    ```

    This method reads the distance data, fills the model, and returns it in JSON format for easy communication.

3. Implement getModelDefinition()

    To allow the client to understand the capabilities of the sensor, implement the getModelDefinition() method. It passes the sensor's capabilities in a JSON format:

    ```cpp
    void DistanceSensor::getModelDefinition(JsonObject& json) {
        DistanceModel model = DistanceModel();
        model.getModelDefinition(json);
    }
    ```


## Step 7: Add the Sensor to main.cpp

In the main.cpp file, include and instantiate the DistanceSensor class. Don’t forget to assign it a unique identity:

```cpp
//main.cpp
#include "DistanceSensor.h"

// ...

DistanceSensor *dis = new DistanceSensor("DIS01");

//...

dm->addSensor(dis);
```

The identity (DIS01) helps distinguish this sensor from others that are used in this project.


## Step 8: Upload

With the sensor code implemented, the next step is to ensure that the setup correctly reads and transmits sensor data. Follow these steps to build, upload, and monitor the sensor output.

1. Update platformio.ini for Debugging Mode

    In this step, we'll configure the platformio.ini file to ensure that mayako-node runs in debug mode.

    Add or modify the following configuration in your platformio.ini file:

    ```ini
    #platformio.ini
    [platformio]
    default_envs = mayako

    [env:mayako]
    platform = espressif32
    board = m5stack-fire
    framework = arduino
    build_type = release
    monitor_speed = 115200
    lib_deps = 
        m5stack/M5Stack@^0.4.6
        bblanchon/ArduinoJson@^7.1.0
        robtillaart/CRC@^1.0.3
        adafruit/Adafruit NeoPixel@^1.12.3
        oxullo/MAX30100lib@^1.2.1
        pololu/VL53L0X

    build_flags =
        -D MC_NAME=\"NAKO\"
        -D DEBUG_MODE=1
        -D BAUDRATE=115200
        -D SERVICE_UUID=\"5c719eda-d610-49e2-8c3a-cf13af6996ea\"
        -D CHARACTERISTIC_UUID=\"5a3bc3d8-1850-49c6-9039-9a5714d2b05f\"
    ```

2. Connect the Arduino Board to Your Computer
    Using a USB cable, connect your Arduino board (M5Stack in our case) to your computer. Ensure that the board is recognised by your system.

3. Build and Upload the Code

    After connecting the board, run the PlatformIO build and upload command to compile the code and upload it to the microcontroller:

    ```bash
    pio run -t upload
    ```

    PlatformIO will compile the code and upload it to your connected device. If successful, you’ll see a message confirming the upload.


## Step 9:  Monitor Sensor Data

1. Monitor the Serial Output
   
   Once the code is uploaded, you can start monitoring the serial output. Use the following command to open the serial monitor:

    ```bash
    pio device monitor -b 115200
    ```

    For now, you can only see packets that look like this:

    ```
    #NAKO␀␀9␀␂{}
    ```

    These are the heartbeat packets that mayako-node sends to tell the mayako-core Client that the device exists.


2.  Start a Data Recording Session
    
    To observe sensor data, you need to start a record on the microcontroller. For this, you can use our Python test script that sends specific commands to the device. The script can be found in the [Github Gist](https://gist.github.com/vairasza/91539826b95020340e1b5de1e0f26c01). 

    There are various commands to play around. Use following argsparser command to get the list of available commands with their according index.

    ```bash
    pyton send_serial_cmds.py --list-commands
    ```

    Run the selected commands over serial for mayako-node:

    ```bash
    python send_serial_cmds.py -i 0
    ```

    The result in the terminal should look like this:

    ```bash
    micha@fedora:~/Projects/m5-test$ python send_serial_cmds.py -i 0
    Packet sent successfully: bytearray(b'"MC01\x00\x01\x11\x00\x1c{"cmd_name": "RECORD_START"}')
    ```


3.  View the Distance Sensor Data Packets
    
    Now you can see the distance sensor packets from the microcontroller which look simliar like this.

    ```
    !NAKO␀␀␖␀ {"identity":"DIS01","range":473}
    !NAKO␀␀�␀ {"identity":"DIS01","range":351}
    !NAKO␀␀u␀␟{"identity":"DIS01","range":50}
    !NAKO␀␀␙␀␟{"identity":"DIS01","range":44}
    !NAKO␀␀�␀␟{"identity":"DIS01","range":48}
    !NAKO␀␀�␀␟{"identity":"DIS01","range":52}
    !NAKO␀␀�␀␟{"identity":"DIS01","range":91}
    !NAKO␀␀R␀ {"identity":"DIS01","range":341}
    !NAKO␀␀G␀ {"identity":"DIS01","range":288}
    !NAKO␀␀d␀ {"identity":"DIS01","range":365}
    !NAKO␀␀�␀ {"identity":"DIS01","range":655}
    !NAKO␀␀�␀!{"identity":"DIS01","range":8190}
    !NAKO␀␀�␀!{"identity":"DIS01","range":8190}
    ```

    Each packet consists of a 10-byte header followed by the sensor data in JSON format:
    - **!NAKO␀␀�␀!**: The header contains the data method flag (!), the identity of the microcontroller (NAKO) and additional information to ensure the packet is transmitted and received correctly.
    - **"identity":"DIS01"**: The unique identifier assigned to the distance sensor, as defined in main.cpp.
    - **"range":8190**: The measured distance in millimeters, representing the sensor's reading at that moment.


## Step 10: Conclusion
In this guide, we demonstrated how to successfully add a new sensor to the mayako-node and read sensor data. As shown, the process follows a template that simplifies the implementation of both the sensor and its corresponding model. By using existing files as blueprints and making slight modifications, you can easily integrate new sensors into the system. This approach not only ensures consistency but also streamlines the development process, making it more accessible for adding additional sensors in the future. Whether you're working with distance sensors like the VL53L0X or other types of sensors, the same template and process can be adapted, allowing for scalable sensor integration.

