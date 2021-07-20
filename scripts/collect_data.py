# Adapted from: https://github.com/Ladvien/arduino_ble_sense
# IMPORTANT: Use PowerShell instead of WSL to connect with BLE

import os
import sys
import asyncio
import platform
from datetime import datetime
from typing import Callable, Any, List

from aioconsole import ainput
from bleak import BleakClient, discover

folder_path = "../data-collection/mama"

selected_device = []
currentAction = ""

class DataToFile:

    def __init__(self, write_path):
        self.folder_path = write_path

    def write_to_csv(self, data_values: List[str]):
        file_path = f"{self.folder_path}/{currentAction}.csv"
        with open(file_path, "a+") as f:            
            for i in range(len(data_values)):
                f.write(f"{data_values[i]}\n")

class Connection:

    client: BleakClient = None

    def __init__(
        self,
        loop: asyncio.AbstractEventLoop,
        read_characteristic: str,
        write_characteristic: str,
        data_dump_handler: Callable[[str, Any], None],
        data_dump_size: int = 256,
    ):
        self.loop = loop
        self.read_characteristic = read_characteristic
        self.write_characteristic = write_characteristic
        self.data_dump_handler = data_dump_handler

        self.dump_size = data_dump_size
        self.connected = False
        self.connected_device = None

        self.data = []

    def on_disconnect(self, client: BleakClient):
        self.connected = False
        # Put code here to handle what happens on disconnect.
        print(f"Disconnected from {self.connected_device.name}!")

    async def cleanup(self):
        if self.client:
            await self.client.stop_notify(read_characteristic)
            await self.client.disconnect()

    async def manager(self):
        print("Starting connection manager.")
        while True:
            if self.client:
                await self.connect()
            else:
                await self.select_device()
                await asyncio.sleep(15.0)

    async def connect(self):
        if self.connected:
            return
        try:
            await self.client.connect()
            self.connected = self.client.is_connected
            if self.connected:
                print(F"Connected to {self.connected_device.name}")
                self.client.set_disconnected_callback(self.on_disconnect)
                await self.client.start_notify(
                    self.read_characteristic, self.notification_handler,
                )
                while True:
                    if not self.connected:
                        break
                    await asyncio.sleep(3.0)
            else:
                print(f"Failed to connect to {self.connected_device.name}")
        except Exception as e:
            print(e)

    async def select_device(self):
        print("Bluetooh LE hardware warming up...")
        await asyncio.sleep(2.0)  # Wait for BLE to initialize.
        devices = await discover()

        print("Please select device: ")
        for i, device in enumerate(devices):
            print(f"{i}: {device.name}")

        response = -1
        while True:
            response = await ainput("Select device: ")
            try:
                response = int(response.strip())
            except:
                print("Please make valid selection.")

            if response > -1 and response < len(devices):
                break
            else:
                print("Please make valid selection.")

        print(f"Connecting to {devices[response].name}")
        self.connected_device = devices[response]
        self.client = BleakClient(devices[response].address, loop=self.loop)

    def clear_lists(self):
        self.data.clear()

    def notification_handler(self, sender: str, data_bytes: bytearray):
        self.data.append(data_bytes.decode())
        if len(self.data) >= self.dump_size:
            self.data_dump_handler(self.data)
            self.clear_lists()


#############
# Loops
#############
async def user_console_manager(connection: Connection):
    while True:
        if connection.client and connection.connected:
            input_str = await ainput("Enter command: ")
            global currentAction
            currentAction = input_str.lower()

            if currentAction == "STOP":
                connection.clear_lists()
                
            bytes_to_send = bytearray(map(ord, input_str))
            await connection.client.write_gatt_char(write_characteristic, bytes_to_send)
        else:
            await asyncio.sleep(2.0)


async def main():
    while True:
        # YOUR APP CODE WOULD GO HERE.
        await asyncio.sleep(5)


#############
# App Main
#############
read_characteristic = "00001143-0000-1000-8000-00805f9b34fb"
write_characteristic = "00001142-0000-1000-8000-00805f9b34fb"

if __name__ == "__main__":

    # Create the event loop.
    loop = asyncio.get_event_loop()

    data_to_file = DataToFile(folder_path)
    connection = Connection(
        loop, read_characteristic, write_characteristic, data_to_file.write_to_csv
    )
    try:
        asyncio.ensure_future(connection.manager())
        asyncio.ensure_future(user_console_manager(connection))
        asyncio.ensure_future(main())
        loop.run_forever()
    except KeyboardInterrupt:
        print()
        print("User stopped program.")
    finally:
        print("Disconnecting...")
        loop.run_until_complete(connection.cleanup())
        exit()