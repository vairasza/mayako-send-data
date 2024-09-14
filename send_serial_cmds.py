import serial
from crc import Calculator, Configuration
from serial.tools.list_ports import comports
from typing import List
from dataclasses import dataclass, asdict
from typing import Dict, Self
from argparse import ArgumentParser

@dataclass
class SerialInfo:
    """
    the is a model class representing serial information retrieved with mayako/Service/SerialScanner.py

    Attributes:
        port (str): the serial port on the target machine; Example: /dev/ttyUSB0
        serial_number (str): a 8 byte hex; may be empty
        description (str): describes the connected device e.g. information about the technology, the kind of device, etc.; may be empty
    """
    port: str
    serial_number: str
    description: str

    def format_string(self) -> str:
        """
        makes a string from the properties of the class.
        Returns:
            str: a visual representation of the class used in Views
        """
        return f"Port: {self.port} -- Serial Number: {self.serial_number}"
    
    def to_dict(self) -> Dict:
        """makes a dictionary from the class"""
        return asdict(self)
    
    @classmethod
    def from_dict(cls, data: Dict) -> Self:
        """creates an instance of this class with information from a dictionary"""
        return cls(**data)
    

def scan_serial_ports(device_names: List[str]) -> List[SerialInfo]:
    """
    This function scans all serial ports on the machine (WIN, MACOS, LINUX). Arduino boards often have a description containing the name of the USB-to-UART bridge controller. Therefore we can filter the scanned ports by looking for keywords defined in Config.h > CNETWORK > SERIAL_ARDUINO_WORDS. We stored following information in a dataclass: the port of the found arduino board, the serial number to identify the board in another iteration, the description which contains the keywords.

    Source:
        https://github.com/pyserial/pyserial

    Args:
        device_names (List[str]):
            a list of words that are typical for the arduino serial port description

    Returns:
        list of ports which match our criteria for an arduino board
    """
    available_ports = comports()
    arduino_ports: List[SerialInfo] = []

    for port in available_ports:
        if len(device_names) != 0 and any(keyword in port.description for keyword in device_names):
            serial_number = getattr(port, "serial_number", "unknown")

            arduino_ports.append(SerialInfo(port.device, serial_number, port.description))

    return arduino_ports


COMMANDS = [
            b'{"cmd_name": "RECORD_START"}',
            b'{"cmd_name": "RECORD_STOP"}',
            b'{"cmd_name": "RECORD_READ"}',
            b'{"cmd_name": "BATTERY_READ"}',
            b'{"cmd_name": "CONNECTION_READ"}',
            b'{"cmd_name": "SWITCH_ON","index": 0,"red": 0,"green": 255,"blue": 0}',
            b'{"cmd_name": "SWITCH_OFF"}'
        ]

class Application:

    def __init__(self, serial_port: str, command_index: int, baudrate: int) -> None:
        self._serial_port = serial_port
        self._command_index = command_index
        self._baud_rate = baudrate
                
    @staticmethod
    def _calculate_checksum(payload):
        config = Configuration(width=8, polynomial=0xa7, init_value=0x00, final_xor_value=0x00, reverse_input=True, reverse_output=True)
        calculator = Calculator(configuration=config)
        return calculator.checksum(payload)

    @staticmethod
    def _create_packet(payload):
        # Header components
        header = bytearray([
            0x22,                # First byte
            0x4d, 0x43, 0x30, 0x31,  # Magic sequence
            0x00, 0x01,          # Sequence number
        ])
        # Payload
        
        # Calculate the checksum for the header + size
        checksum = Application._calculate_checksum(payload)
        #exit()
        # Append checksum to the header
        header.append(checksum)
        
        # Size of the payload
        payload_size = len(payload)
        
        # Add the payload size to the header
        #does not work for large data!!! > 255
        header.append(0x00)
        header.append(payload_size)

        # Combine header and payload
        packet = header + payload
        
        return packet
    
    @staticmethod
    def _print_commands():
        print("available commands")
        for key, command in enumerate(COMMANDS):
            print(f"{key}: {command}")

    def send_packet_via_serial(self):
        if self._command_index < 0 or self._command_index > len(COMMANDS):
            print(f"invalid command index: {self._command_index}")
        packet = Application._create_packet(COMMANDS[self._command_index])
        try:
            # Open serial connection
            with serial.Serial(self._serial_port, self._baud_rate) as ser:
                # Send packet
                output = ser.write(packet)
                print(f"Packet sent successfully: {packet}")

        except serial.SerialException as e:
            print(f"Error: {e}")


if __name__ == "__main__":
    parser = ArgumentParser(prog="mayako serial command helper", description="sends commands to mayako-core over serial")
    parser.add_argument("-i", default=0, type=int, help="index of commands")
    parser.add_argument("-b", default=115200, type=int, help="baudate")
    parser.add_argument("--list-commands", action="store_true", help="list available commands with their index")

    args = parser.parse_args()

    if args.list_commands:
        Application._print_commands()
        exit(0)

    ports = scan_serial_ports(["CP2104", "Arduino"])
    if not ports:
        print("No Arduino devices found.")
        exit(1)
    if len(ports) > 1:
        print("Too many Arduino devices found.")
        exit(1)

    serial_port = ports[0].port

    application = Application(serial_port=serial_port, command_index=args.i, baudrate=args.b)
    application.send_packet_via_serial()
