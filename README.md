# The Cuatrovientos IoT Lab: A project-based learning activity

## _Project promoted by_ Engineering Excellence for the Mobility Value Chain (EE4M)

> @author: Ander F.L. <ander_frago@cuatrovientos.org>

This project-based learning (PBL) activity is designed for Vocational Education and Training (VET) computer science students at Cuatrovientos. It challenges students to build a complete Internet of Things (IoT) solution by integrating hardware, software, cloud services, and entrepreneurial thinking. Students will work in groups to develop a real-world prototype, from hardware assembly to digital twin creation, simulating a complete IoT product development cycle.

## Project components

### Slave device: Osoyoo Smart Home IoT Kit

- **Hardware**: Students will use the Osoyoo Smart Home IoT Kit, which includes an Arduino MEGA2560 and various sensors and actuators.
- **Function**: The Arduino board acts as the "slave," managing local interactions with sensors and actuators and communicating via the serial port.

### Master device: Raspberry Pi

- **Hardware**: A Raspberry Pi serves as the "master" controller and gateway to the internet.

- **Function**:
  - **Communication**: It communicates with the Arduino over a serial connection via a USB-to-USB cable.
  - **Web server**: It hosts a web server developed using the Python Flask framework, which can be extended with libraries like Flask-MQTT for IoT messaging.

### Cloud integration: AWS IoT Core

- **Function**: The Raspberry Pi will connect to AWS IoT Core, a managed cloud service that allows students to securely connect, manage, and process data from IoT devices.
- **Task**: Students must register their "IoT Thing" in AWS and establish a secure connection using device certificates.

### Digital Twin: Cisco Packet Tracer

- **Function**: Students will create a digital twin of their IoT solution in Cisco Packet Tracer to simulate and visualize network interactions.
- **Benefit**: This helps them understand the network layer of their project without physical constraints, and it provides a virtual replica that can be synchronized with the real hardware.

### Learning challenges

1.  **Mobile application development**: Design and create a mobile application for remote control of the smart home. The app should communicate with the Flask web server on the Raspberry Pi to control actuators (like lights or motors) and display data from sensors (temperature, motion).
2.  **IoT solution design**: In collaboration with the entrepreneurship subject, each group must:

- Identify a real-world IoT problem.

- Select specific sensors and actuators from the Osoyoo kit to represent their solution.
- Develop a unique "IoT solution" that addresses the problem identified, showcasing their creativity and problem-solving skills.

3. **Cloud-to-device communication**: Ensure seamless, bidirectional communication between the mobile app, the Flask server, AWS IoT, and the Arduino-controlled hardware. This involves using the MQTT protocol for lightweight messaging.

### Project workflow

1. Team formation and ideation: Students form groups and, in coordination with the entrepreneurship subject, generate an idea for their IoT solution.
2. Hardware setup: Assemble the Osoyoo kit and configure the Raspberry Pi to communicate with the Arduino via the serial port.
3. Web server development: Set up the Flask server on the Raspberry Pi to handle communication and control logic.
4. AWS IoT integration: Connect the Flask server to AWS IoT Core, configure policies, and handle secure cloud messaging.
5. Digital Twin creation: Build a virtual representation of the project in Cisco Packet Tracer, configuring the virtual devices to mirror the behavior of the physical hardware.
6. Mobile app development: Create the mobile application to provide a user interface for controlling and monitoring the system.
7. Testing and documentation: Thoroughly test the entire system and document the project, including the hardware schematic, software code, and a final presentation of the entrepreneurial solution.
