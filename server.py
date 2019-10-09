#--------------------------------------------------------------------------------------------------------------------------------------------
#    Name          : server.py
#    Author        : Stephen Amey
#    Date          : Aug. 9, 2019
#    Purpose  	   : This program serves as a test groundstation server for Western University's _HAB project.
#--------------------------------------------------------------------------------------------------------------------------------------------


#-----------------------------------------------------------------------------------------------------------\
#                                                    Imports                                                |
#-----------------------------------------------------------------------------------------------------------/


import socket
import tkinter as tk
import _thread
import time
import queue
import winsound


#-----------------------------------------------------------------------------------------------------------\
#                                                   Variables                                               |
#-----------------------------------------------------------------------------------------------------------/


request_queue = queue.Queue()
result_queue = queue.Queue()

server_address = socket.gethostbyname(socket.gethostname())
server_port = 54444 #0

remote_address = ''
remote_port = 0

#Heartbeat
beat_delay = 1.0
last_beat = 0
#Socket setup
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
serverSocket.bind((server_address, server_port))
serverSocket.settimeout(beat_delay)

remoteSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

#Open and close altitudes
act1OpenAlt = 2000
act1CloseAlt = 10000
act2OpenAlt = 12000
act2CloseAlt = 20000
act3OpenAlt = 22000
act3CloseAlt = 30000
act4OpenAlt = 32000
act4CloseAlt = 99999

actOpenLim = 10
actCloseLim = 1020


#-----------------------------------------------------------------------------------------------------------\
#                                              GUI thread functions                                         |
#-----------------------------------------------------------------------------------------------------------/


def submit_to_tkinter(callable, *args, **kwargs):
    request_queue.put((callable, args, kwargs))
    return result_queue.get()

t = None
def threadmain():
    global t

    def timertick():
        try:
            callable, args, kwargs = request_queue.get_nowait()
        except queue.Empty:
            pass
        else:
            print("something in queue")
            retval = callable(*args, **kwargs)
            result_queue.put(retval)

        t.after(700, timertick)

    t = tk.Tk()
    t.configure(width=1420, height=800)
    t.title("Western HAB ground server")
    t.iconbitmap('icon.ico')
	
    #Flight time
    timeLabel = tk.Label(text="Time")
    timeLabel.place(x=20, y=210)
	
    timeBox = tk.Label(height=1, width=20, bg="white", name='timeBox')
    timeBox.place(x=60, y=210)
	
    #_HAB GPS
    _HABGPSLabel = tk.Label(text="HAB GPS")
    _HABGPSLabel.place(x=120, y=30)
	
    _HABaltitudeLabel = tk.Label(text="Altitude")
    _HABaltitudeLabel.place(x=20, y=60)	
    _HABaltitudeBox = tk.Label(height=1, width=10, bg="white", name='_HABaltitudeBox')
    _HABaltitudeBox.place(x=100, y=60)

    _HABlongitudeLabel = tk.Label(text="Longitude")
    _HABlongitudeLabel.place(x=20, y=90)	
    _HABlongitudeBox = tk.Label(height=1, width=10, bg="white", name='_HABlongitudeBox')
    _HABlongitudeBox.place(x=100, y=90)
	
    _HABlatitudeLabel = tk.Label(text="Latitude")
    _HABlatitudeLabel.place(x=20, y=120)	
    _HABlatitudeBox = tk.Label(height=1, width=10, bg="white", name='_HABlatitudeBox')
    _HABlatitudeBox.place(x=100, y=120)
	
	#_CSA GPS
    _CSAGPSLabel = tk.Label(text="CSA GPS")
    _CSAGPSLabel.place(x=320, y=30)
	
    _CSAaltitudeLabel = tk.Label(text="Altitude")
    _CSAaltitudeLabel.place(x=220, y=60)	
    _CSAaltitudeBox = tk.Label(height=1, width=10, bg="white", name='_CSAaltitudeBox')
    _CSAaltitudeBox.place(x=300, y=60)

    _CSAlongitudeLabel = tk.Label(text="Longitude")
    _CSAlongitudeLabel.place(x=220, y=90)	
    _CSAlongitudeBox = tk.Label(height=1, width=10, bg="white", name='_CSAlongitudeBox')
    _CSAlongitudeBox.place(x=300, y=90)
	
    _CSAlatitudeLabel = tk.Label(text="Latitude")
    _CSAlatitudeLabel.place(x=220, y=120)	
    _CSAlatitudeBox = tk.Label(height=1, width=10, bg="white", name='_CSAlatitudeBox')
    _CSAlatitudeBox.place(x=300, y=120)
	
    #BME
    BMELabel = tk.Label(text="BME280")
    BMELabel.place(x=510, y=30)
	
    temperatureLabel = tk.Label(text="Temp       (*C)")
    temperatureLabel.place(x=420, y=60)	
    temperatureBox = tk.Label(height=1, width=10, bg="white", name='temperatureBox')
    temperatureBox.place(x=500, y=60)

    pressureLabel = tk.Label(text="Pressure   (Pa)")
    pressureLabel.place(x=420, y=90)	
    pressureBox = tk.Label(height=1, width=10, bg="white", name='pressureBox')
    pressureBox.place(x=500, y=90)
	
    humidityLabel = tk.Label(text="Humidity (%)")
    humidityLabel.place(x=420, y=120)	
    humidityBox = tk.Label(height=1, width=10, bg="white", name='humidityBox')
    humidityBox.place(x=500, y=120)

    #Act1
    Act1Label = tk.Label(text="Actuator 1")
    Act1Label.place(x=710, y=30)
	
    Act1PosLabel = tk.Label(text="Position")
    Act1PosLabel.place(x=620, y=60)	
    Act1PosBox = tk.Label(height=1, width=10, bg="white", name='act1PosBox')
    Act1PosBox.place(x=700, y=60)

    Act1TempLabel = tk.Label(text="Temperature")
    Act1TempLabel.place(x=620, y=90)	
    Act1TempBox = tk.Label(height=1, width=10, bg="white", name='act1TempBox')
    Act1TempBox.place(x=700, y=90)
	
    Act1AOSLabel = tk.Label(text="Act status")
    Act1AOSLabel.place(x=620, y=120)	
    Act1AOSBox = tk.Label(height=1, width=10, bg="white", name='act1AOSBox')
    Act1AOSBox.place(x=700, y=120)
	
    Act1HOSLabel = tk.Label(text="Heater status")
    Act1HOSLabel.place(x=620, y=150)	
    Act1HOSBox = tk.Label(height=1, width=10, bg="white", name='act1HOSBox')
    Act1HOSBox.place(x=700, y=150)
	
    Act1OpenLabel = tk.Label(text="Open altitude")
    Act1OpenLabel.place(x=620, y=180)	
    Act1OpenBox = tk.Label(height=1, width=10, bg="yellow", name="act1OpenBox", text=act1OpenAlt)
    Act1OpenBox.place(x=700, y=180)
	
    Act1CloseLabel = tk.Label(text="Close altitude")
    Act1CloseLabel.place(x=620, y=210)	
    Act1CloseBox = tk.Label(height=1, width=10, bg="yellow", name="act1CloseBox", text=act1CloseAlt)
    Act1CloseBox.place(x=700, y=210)

    #Act2
    Act2Label = tk.Label(text="Actuator 2")
    Act2Label.place(x=910, y=30)
	
    Act2PosLabel = tk.Label(text="Position")
    Act2PosLabel.place(x=820, y=60)	
    Act2PosBox = tk.Label(height=1, width=10, bg="white", name='act2PosBox')
    Act2PosBox.place(x=900, y=60)

    Act2TempLabel = tk.Label(text="Temperature")
    Act2TempLabel.place(x=820, y=90)	
    Act2TempBox = tk.Label(height=1, width=10, bg="white", name='act2TempBox')
    Act2TempBox.place(x=900, y=90)
	
    Act2AOSLabel = tk.Label(text="Act status")
    Act2AOSLabel.place(x=820, y=120)	
    Act2AOSBox = tk.Label(height=1, width=10, bg="white", name='act2AOSBox')
    Act2AOSBox.place(x=900, y=120)
	
    Act2HOSLabel = tk.Label(text="Heater status")
    Act2HOSLabel.place(x=820, y=150)	
    Act2HOSBox = tk.Label(height=1, width=10, bg="white", name='act2HOSBox')
    Act2HOSBox.place(x=900, y=150)
	
    Act2OpenLabel = tk.Label(text="Open altitude")
    Act2OpenLabel.place(x=820, y=180)	
    Act2OpenBox = tk.Label(height=1, width=10, bg="yellow", name="act2OpenBox", text=act2OpenAlt)
    Act2OpenBox.place(x=900, y=180)
	
    Act2CloseLabel = tk.Label(text="Close altitude")
    Act2CloseLabel.place(x=820, y=210)	
    Act2CloseBox = tk.Label(height=1, width=10, bg="yellow", name="act2CloseBox", text=act2CloseAlt)
    Act2CloseBox.place(x=900, y=210)
	
    #Act3
    Act3Label = tk.Label(text="Actuator 3")
    Act3Label.place(x=1110, y=30)
	
    Act3PosLabel = tk.Label(text="Position")
    Act3PosLabel.place(x=1020, y=60)	
    Act3PosBox = tk.Label(height=1, width=10, bg="white", name='act3PosBox')
    Act3PosBox.place(x=1100, y=60)

    Act3TempLabel = tk.Label(text="Temperature")
    Act3TempLabel.place(x=1020, y=90)	
    Act3TempBox = tk.Label(height=1, width=10, bg="white", name='act3TempBox')
    Act3TempBox.place(x=1100, y=90)
	
    Act3AOSLabel = tk.Label(text="Act status")
    Act3AOSLabel.place(x=1020, y=120)	
    Act3AOSBox = tk.Label(height=1, width=10, bg="white", name='act3AOSBox')
    Act3AOSBox.place(x=1100, y=120)
	
    Act3HOSLabel = tk.Label(text="Heater status")
    Act3HOSLabel.place(x=1020, y=150)	
    Act3HOSBox = tk.Label(height=1, width=10, bg="white", name='act3HOSBox')
    Act3HOSBox.place(x=1100, y=150)
	
    Act3OpenLabel = tk.Label(text="Open altitude")
    Act3OpenLabel.place(x=1020, y=180)	
    Act3OpenBox = tk.Label(height=1, width=10, bg="yellow", name="act3OpenBox", text=act3OpenAlt)
    Act3OpenBox.place(x=1100, y=180)
	
    Act3CloseLabel = tk.Label(text="Close altitude")
    Act3CloseLabel.place(x=1020, y=210)	
    Act3CloseBox = tk.Label(height=1, width=10, bg="yellow", name="act3CloseBox", text=act3CloseAlt)
    Act3CloseBox.place(x=1100, y=210)

    #Act4
    Act4Label = tk.Label(text="Actuator 4")
    Act4Label.place(x=1310, y=30)
	
    Act4PosLabel = tk.Label(text="Position")
    Act4PosLabel.place(x=1220, y=60)	
    Act4PosBox = tk.Label(height=1, width=10, bg="white", name='act4PosBox')
    Act4PosBox.place(x=1300, y=60)

    Act4TempLabel = tk.Label(text="Temperature")
    Act4TempLabel.place(x=1220, y=90)	
    Act4TempBox = tk.Label(height=1, width=10, bg="white", name='act4TempBox')
    Act4TempBox.place(x=1300, y=90)
	
    Act4AOSLabel = tk.Label(text="Act status")
    Act4AOSLabel.place(x=1220, y=120)	
    Act4AOSBox = tk.Label(height=1, width=10, bg="white", name='act4AOSBox')
    Act4AOSBox.place(x=1300, y=120)
	
    Act4HOSLabel = tk.Label(text="Heater status")
    Act4HOSLabel.place(x=1220, y=150)	
    Act4HOSBox = tk.Label(height=1, width=10, bg="white", name='act4HOSBox')
    Act4HOSBox.place(x=1300, y=150)
	
    Act4OpenLabel = tk.Label(text="Open altitude")
    Act4OpenLabel.place(x=1220, y=180)	
    Act4OpenBox = tk.Label(height=1, width=10, bg="yellow", name="act4OpenBox", text=act4OpenAlt)
    Act4OpenBox.place(x=1300, y=180)
	
    Act4CloseLabel = tk.Label(text="Close altitude")
    Act4CloseLabel.place(x=1220, y=210)	
    Act4CloseBox = tk.Label(height=1, width=10, bg="yellow", name="act4CloseBox", text=act4CloseAlt)
    Act4CloseBox.place(x=1300, y=210)
	
    #Event box
    eventBox = tk.Label(height=28, width=121, justify="left", bg="white", name="eventBox", anchor="sw")
    eventBox.place(x=40, y=270)
	
    #Commands
    commandBox = tk.Entry(width=142, name="commandBox")
    commandBox.place(x=40, y=740)
    sendButton = tk.Button(text='SEND', name='sendButton', height=2, width=10, command=sendCommand)
    sendButton.place(x=920, y=720)
    t.bind('<Return>', enterKeyPressed)
	
    #Halt button
    haltButton = tk.Button(width=21, bg="red", fg="white", relief="groove", bd=5, text="HALT ACTUATOR", command=buttonHaltCommand)
    haltButton.place(x=420, y=200)
	
    #Commands list
    commandsLabel = tk.Label(height=13, width=30, justify="left", text="SET_ACTIVE <pod name>\nOVR_ACT_OPEN\nOVR_ACT_CLOSE\nOVR_ACT_HALT\nACT_ENABLE_LOCK\nACT_DISABLE_LOCK\nSET_MAX_TEMP <-20 to 30>\nSET_MIN_TEMP <-20 to 30>\nOVR_HEAT_ENABLE\nOVR_HEAT_DISABLE\nOVR_HEAT_RELEASE\nSET_DESCENDING\nHAB_END_FLIGHT")
    commandsLabel.place(x=1050, y=300)
	
    #Start the GUI loop
    timertick()
    t.mainloop()
	
def updateDisplays(message_text):
    #Gets the first 3 characters
    identifier = message_text[1:6]

    if(identifier == "INTLZ"):
        #Nothing
        print("INIT MSG")
    elif(identifier == "EVENT"):
        #Just print it to the console
        t.children["eventBox"].configure(text=(t.children["eventBox"]["text"] + "\n" + message_text))
        eventFile = open("eventLog.txt", "a")
        eventFile.write(message_text + "\n")
        eventFile.close()
    else:
        telemetryFile = open("telemetryLog.txt", "a")
        telemetryFile.write(message_text)
        telemetryFile.close()
	
        #Assumed telemetry, so parse it
        fields = (message_text).split(",")
		
        #Finds which GPS is giving the highest altitude
        maxAlt = max(float(fields[4]), float(fields[8]))
		
        #Time
        t.children["timeBox"].configure(text=fields[2])
		
        #_HAB GPS
        t.children["_HABaltitudeBox"].configure(text=float(fields[4]))
        #Speed @ 5
        t.children["_HABlongitudeBox"].configure(text=float(fields[6]))
        t.children["_HABlatitudeBox"].configure(text=float(fields[7]))
		
		#_CSA GPS
        t.children["_CSAaltitudeBox"].configure(text=float(fields[8]))
        t.children["_CSAlongitudeBox"].configure(text=float(fields[9]))
        t.children["_CSAlatitudeBox"].configure(text=float(fields[10]))
		
        #BME
        t.children["temperatureBox"].configure(text=fields[11])
        t.children["pressureBox"].configure(text=fields[12])
        t.children["humidityBox"].configure(text=fields[13])
		
        #Act1
        t.children["act1PosBox"].configure(text=fields[14])
        t.children["act1PosBox"].configure(bg=getActuatorColour(fields[14]))	
        t.children["act1TempBox"].configure(text=fields[15])
        t.children["act1AOSBox"].configure(text=(("OVR_CLOSE", "OVR_OPEN","AUTO")[int(fields[16])]))
        t.children["act1HOSBox"].configure(text=(("OVR_DISABLE", "OVR_ENABLE","AUTO")[int(fields[17])]))
        if(t.children["act1OpenBox"].cget('bg') == "yellow" and maxAlt >= act1OpenAlt):
            t.children["act1OpenBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)
        if(t.children["act1CloseBox"].cget('bg') == "yellow" and maxAlt >= act1CloseAlt):
            t.children["act1CloseBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)
		
        #Act2
        t.children["act2PosBox"].configure(text=fields[18])
        t.children["act2PosBox"].configure(bg=getActuatorColour(fields[18]))
        t.children["act2TempBox"].configure(text=fields[19])
        t.children["act2AOSBox"].configure(text=(("OVR_CLOSE", "OVR_OPEN","AUTO")[int(fields[20])]))
        t.children["act2HOSBox"].configure(text=(("OVR_DISABLE", "OVR_ENABLE","AUTO")[int(fields[21])]))
        if(t.children["act2OpenBox"].cget('bg') == "yellow" and maxAlt >= act2OpenAlt):
            t.children["act2OpenBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)
        if(t.children["act2CloseBox"].cget('bg') == "yellow" and maxAlt >= act2CloseAlt):
            t.children["act2CloseBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)	
		
        #Act3
        t.children["act3PosBox"].configure(text=fields[22])
        t.children["act3PosBox"].configure(bg=getActuatorColour(fields[22]))
        t.children["act3TempBox"].configure(text=fields[23])
        t.children["act3AOSBox"].configure(text=(("OVR_CLOSE", "OVR_OPEN","AUTO")[int(fields[24])]))
        t.children["act3HOSBox"].configure(text=(("OVR_DISABLE", "OVR_ENABLE","AUTO")[int(fields[25])]))
        if(t.children["act3OpenBox"].cget('bg') == "yellow" and maxAlt >= act3OpenAlt):
            t.children["act3OpenBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)
        if(t.children["act3CloseBox"].cget('bg') == "yellow" and maxAlt >= act3CloseAlt):
            t.children["act3CloseBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)
		
        #Act4
        t.children["act4PosBox"].configure(text=fields[26])
        t.children["act4PosBox"].configure(bg=getActuatorColour(fields[26]))
        t.children["act4TempBox"].configure(text=fields[27])
        t.children["act4AOSBox"].configure(text=(("OVR_CLOSE", "OVR_OPEN","AUTO")[int(fields[28])]))
        t.children["act4HOSBox"].configure(text=(("OVR_DISABLE", "OVR_ENABLE","AUTO")[int(fields[29].split("\r")[0])]))
        if(t.children["act4OpenBox"].cget('bg') == "yellow" and maxAlt >= act4OpenAlt):
            t.children["act4OpenBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)
        if(t.children["act4CloseBox"].cget('bg') == "yellow" and maxAlt >= act4CloseAlt):
            t.children["act4CloseBox"].configure(bg="SpringGreen2")
            winsound.PlaySound('sound.wav', winsound.SND_FILENAME)				
		
		
def sendCommand():
    if(remote_address != '' and remote_port != '' and t.children["commandBox"].get() != ""):
        t.children["eventBox"].configure(text=(t.children["eventBox"]["text"] + "\nCOMMAND: " + t.children["commandBox"].get()))
        message = bytes("GROUNDSTATION," + t.children["commandBox"].get(), 'utf-8')
        remoteSocket.sendto(message, (str(remote_address), int(remote_port)))
        t.children["commandBox"].delete(0, 'end')

def buttonHaltCommand():
    if(remote_address != '' and remote_port != ''):
        message = bytes("GROUNDSTATION,OVR_ACT_HALT", 'utf-8')
        remoteSocket.sendto(message, (str(remote_address), int(remote_port)))

def enterKeyPressed(event):
    sendCommand()
	
def getActuatorColour(position):
    if(float(position) >= actCloseLim):
        return "SpringGreen2"
    elif(float(position) <= actOpenLim):
        return "orange"
    else:
        return "yellow"


#-----------------------------------------------------------------------------------------------------------\
#                                                  Program run                                              |
#-----------------------------------------------------------------------------------------------------------/  


if __name__ == '__main__':
    #If the files do not currently exist, create them
    eventFile = open("eventLog.txt", "a")
    eventFile.write("############################################\n#               Server start               #\n############################################\n")
    eventFile.close()

    telemetryFile = open("telemetryLog.txt", "a")
    telemetryFile.write("############################################\n#               Server start               #\n############################################\n")
    telemetryFile.close()

    #Hosts the server
    server_address = serverSocket.getsockname()[0]
    server_port = serverSocket.getsockname()[1]
    print('Hosting at : ' + server_address + ':' + str(server_port))
	
    #Starts the GUI
    _thread.start_new_thread(threadmain, ())

    #Enters the main loop
    while True:
        #Send a heart-beat every second
        if(remote_address != '' and remote_port != '' and (time.time() - last_beat) >= beat_delay):
            last_beat = time.time()
            message = bytes("GROUNDSTATION,HBT\0", 'utf-8')
            remoteSocket.sendto(message, (str(remote_address), int(remote_port)))

	    #Attempt to receive a packet
        try:      
            message, address = serverSocket.recvfrom(1024)
            print('(' + address[0] + ':' + str(address[1]) + ') : ' + message.decode('utf-8'))
            updateDisplays(message.decode('utf-8'))

		    #If no client address yet defined
            if(remote_address == ''):
                #Records the new sender
                remote_address = address[0]
                remote_port = str(address[1])	
			
        except Exception as e:	
            print(e)
            1