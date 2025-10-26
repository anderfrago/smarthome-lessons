import serial
import time
from flask import Flask, render_template, request, jsonify

app = Flask(__name__)

# --- Serial Communication Setup ---
# NOTE: Please replace '/dev/ttyACM0' with the correct serial port for your Arduino.
# On Raspberry Pi, it is typically /dev/ttyACM0 or /dev/ttyUSB0.
# You can find the correct port by checking the output of `dmesg | grep tty` after plugging in your Arduino.
SERIAL_PORT = '/dev/ttyACM0' 
BAUD_RATE = 9600

ser = None
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for the serial connection to initialize
except serial.SerialException as e:
    print(f"Error: Could not open serial port '{SERIAL_PORT}'. {e}")
    print("Please ensure the Arduino is connected and the correct serial port is specified in app.py.")

def send_command(command):
    """Sends a command to the Arduino and reads the response."""
    if not ser or not ser.is_open:
        return ["Serial port is not available."]
    
    print(f"Sending command: {command}")
    ser.write((command + '\n').encode('utf-8'))
    
    # Wait a moment for the Arduino to process and respond
    time.sleep(0.5) 
    
    responses = []
    while ser.in_waiting > 0:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line:
                responses.append(line)
        except UnicodeDecodeError:
            pass # Ignore occasional decoding errors
            
    print(f"Received response: {responses}")
    return responses if responses else ["No response from device."]

# --- Web Routes ---

@app.route('/')
def index():
    """Renders the main control panel page."""
    return render_template('index.html')

@app.route('/control', methods=['POST'])
def control():
    """Handles generic commands like LED, buzzer, and servo control."""
    command = request.form.get('command')
    if not command:
        return jsonify({"status": "error", "message": "No command provided."}), 400
        
    # Format the command for LCD messages
    if command.startswith("lcd"):
        message = request.form.get('message', '')
        command = f'lcd "{message}"'

    response = send_command(command)
    return jsonify({"status": "success", "command": command, "response": response})

@app.route('/sensors')
def get_sensors():
    """Specifically handles the 'sensors' command to fetch all sensor data."""
    if not ser or not ser.is_open:
        return jsonify({"error": "Serial port not available."})

    ser.flushInput()  # Clear any old data in the input buffer
    response_lines = send_command("sensors")
    
    sensor_data = {}
    for line in response_lines:
        if "Result: " in line:
            # Parse lines like "Result: Temperature: 24.00C"
            try:
                key_part, value_part = line.split("Result: ", 1)[1].split(': ', 1)
                key = key_part.strip().lower().replace(" ", "_")
                sensor_data[key] = value_part.strip()
            except ValueError:
                # Handle lines without a key-value structure, like "Result: Fire Detected!"
                key = "status"
                value = line.split("Result: ", 1)[1]
                if "fire" in value.lower():
                    sensor_data["fire_safety"] = value
                elif "noise" in value.lower():
                    sensor_data["noise_status"] = value
                elif "intruder" in value.lower():
                    sensor_data["motion_status"] = value

    print(f"Parsed Sensor Data: {sensor_data}")
    return jsonify(sensor_data)

if __name__ == '__main__':
    print("Starting Flask server. Open http://<your-pi-ip-address>:5000 in a browser.")
    app.run(host='0.0.0.0', port=5000)
