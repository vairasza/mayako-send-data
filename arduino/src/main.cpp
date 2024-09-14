#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <memory>

#include "PreferencesStorage.h"
#include "AccelerometerSensor.h"
#include "TemperatureSensor.h"
#include "HeartrateSensor.h"
#include "AHRSSensor.h"
#include "ButtonSensor.h"
#include "GyroscopeSensor.h"
#include "DistanceSensor.h"
#include "LEDActuator.h"
#include "BoardM5Stack.h"
#include "CommandManager.h"
#include "DeviceManager.h"
#include "Config.h"
#include "Logger.h"
#include "Definitions.h"
#include "Capabilities.h"

#if WIRELESS_MODE == BLE
#include "BluetoothProtocol.h"
#elif WIRELESS_MODE == WIFI
#include "WifiProfile.h"
#include "NetworkManager.h"
#endif


Storage *store;
NetworkManager *nm;
BoardM5Stack *board;
DeviceManager *dm;
CommandManager *cm;
Logger *logger;

void setup() {
	//init networkmanager, commandmanager and devicemanager plus assisting classes
	store = new PreferencesStorage(MC_NAME);
	nm = new NetworkManager(store);
	board = new BoardM5Stack();
	dm = new DeviceManager(board);
	cm = new CommandManager(*dm, *nm);
	
	//create logger for error and debug messages
	logger = Logger::getInstance();

	logger->debug(prefix("setup: initialising sensors and actuators"));
	//initialise sensors and actuators
	AccelerometerSensor *acc = new AccelerometerSensor("ACC01");
	TemperatureSensor *temp = new TemperatureSensor("TMP01");
	HeartrateSensor *heart = new HeartrateSensor("HEART01");
	AHRSSensor *ahrs = new AHRSSensor("AHR01");
	GyroscopeSensor *gyr = new GyroscopeSensor("GYR01");
	DistanceSensor *dis = new DistanceSensor("DIS01");
	ButtonSensor *btn1 = new ButtonSensor("BTN01", BUTT::ON_A);
	ButtonSensor *btn2 = new ButtonSensor("BTN02", BUTT::ON_B);
	ButtonSensor *btn3 = new ButtonSensor("BTN03", BUTT::ON_C);
	LEDActuator *led = new LEDActuator("LED01", PIN::M5::PORT_A, ACT::SK6812::NUM_PIXELS); //TODO: hinweisen dass hier die richtigen ports angeschlsosen werden müssen
	
	//add sensors and actuators to the device manager which helps read sensor data
	
	dm->addSensor(dis);
	dm->addActuator(led);

	//the actuator used in the command MUST be added to the devicemanager, otherwise the commands can not be redirected to the according actuator class
	//the command for the custom command must be unique in this project!
	cm->addCommand(CUSTOM_CMD::SWITCH_ON, led);
	cm->addCommand(CUSTOM_CMD::SWITCH_OFF, led);
	
	logger->debug(prefix("setup finished"));
}

void loop() {
	nm->upgradeProtocol();

	if (nm->isConnected()) {		
		nm->sendHeartbeatToClient();
		
		std::vector<JsonDocument> commands = nm->readIncomingData();

		for (auto command : commands) {
			cm->executeCommand(&command);			
		}

		if (dm->isRecordInProgress()) {
			dm->updateSensors();

			std::vector<std::shared_ptr<Packet>> sensorData = dm->readSensors();

			//work with sensor data if needed			

			nm->addSensorDataToOutput(sensorData);
			dm->isRecordComplete();
		}		
		nm->writeOutgoingData();		
	}	
	delay(MC::LOOP_WAIT_TIME_DEFAULT);
}