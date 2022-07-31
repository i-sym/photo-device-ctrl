import serial 
import serial.tools.list_ports
port_list = list(serial.tools.list_ports.comports())
print(port_list)
if len(port_list) == 0:
    print('No Available Serial Port')
else:
    print('COM detected:', port_list[0])
    ser = serial.Serial(port_list[0].device, 115200)
    print('Serial Port Opened')
    while True:
        data = ser.readline()
        print(data.decode('utf-8'))
        if data.decode('utf-8').startswith('[LOG]'):
            print('Log detected')
            break
    # Send "RUN TEST" and wait for response
    ser.write(b'RUN TEST\n')
   
    data = ser.readline()
    print(data.decode('utf-8'))

    ser.write(b'ERASE\n')

    print("Done so far")

    sample_scene = [
        'O 1',
        'W 4',
        'C 1',
        'O 2',
        'O 3',
        'W 3',
        'C 2',
        'C 3',
    ]
    for i in range(0, len(sample_scene)):
        # Write in format: STORE <index>:<value>
        cmd = 'STORE ' + str(i) + ':' + sample_scene[i] + '\n'
        print(">", cmd)
        ser.write(cmd.encode('utf-8'))
        data = ser.readline()
        print(data.decode('utf-8'))
    print("Done so far")
    ser.write(b'RUN TEST\n')
    data = ser.readline()
    print(data.decode('utf-8'))
    ser.write(b'START\n')
    data = ser.readline()
    print(data.decode('utf-8'))
    while True:
        data = ser.readline()
        print(data.decode('utf-8'))

            