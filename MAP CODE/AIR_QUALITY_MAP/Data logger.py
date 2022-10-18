from asyncore import read
import paho.mqtt.client as mqtt  # import the client1
import time
from time import strftime
import serial

from unittest import result
import requests

apiquery = "https://maps.googleapis.com/maps/api/geocode/json?latlng=latitudehere,longitudehere&key=AIzaSyC8UiVldgTe6MhuLHK6yt5-ZJz6alJu7io"

airqdata = ''
gpsdata = ''
inmsg = ''
timetracker = time.time()
reconnecttimetracker = time.time()
acquisitiontime = 10
reconnecttime = 10

#client = mqtt.Client("P1")  # create new instance
#_serial = serial.Serial()

mqttconnected = False
serialconnected = False

#######################################


def on_message(client, userdata, message):
    global airqdata
    airqdata = str(message.payload.decode("utf-8"))
    #print(airqdata)
    #print("message topic=",message.topic)
    #print("message qos=",message.qos)
    #print("message retain flag=",message.retain)


def on_publish(client, userdata, result):  # create function for callback
    print("data published \n")
    pass


#broker_address = "airquality"
#client.on_message = on_message  # attach function to callback
#client.on_publish = on_publish
########################################


def configureMQTT():
    global mqttconnected
    global client
    client = mqtt.Client("P1")  # create new instance
    try:
        print("connecting to broker")
        broker_address = "airquality"
        client.connect(broker_address)  # connect to broker
        client.on_message = on_message  # attach function to callback
        client.on_publish = on_publish
        client.loop_start()  # start the loop
        client.subscribe("#")
        mqttconnected = True
    except:
        print("connection failed, check wifi")
        mqttconnected = False

def setSerialstatus(act: bool):
    global serialconnected
    serialconnected = act

def connectSerial():
    global serialconnected
    global _serial
    try:
        _serial = serial.Serial(
            'COM50',
            baudrate=115200,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=0.5
        )
        serialconnected = True
    except:
        print("Failed to open serial")
        serialconnected = False

    try:
        if _serial.is_open:
            print("serial opened")
    except:
        print("serial port definition failed")


def readfromserial():
    try:
        if _serial.is_open:
            indata = ''
            while _serial.in_waiting > 0:
                inbyte = _serial.read(1)
                if inbyte == b'$':
                    return indata
                try:
                    indata += inbyte.decode("utf-8")
                except:
                    print("decode error")
            return indata
        else:
            ...
    except:
        print("no serial")
        setSerialstatus(False)
    return ''



def setupA9G():
    try:
        if _serial.is_open:
            _serial.write(b'AT+GPS=1')
            time.sleep(0.5)
            res = readfromserial()
            print(res)
            _serial.write(b'AT+GPSRD=3')
    except:
        print("no serial")
        setSerialstatus(False)


def sendSMS(msgtxt: str):
    try:
        if _serial.is_open:
            _serial.write(b'AT+CHUP')
            time.sleep(5)
            _serial.flushInput()
            _serial.write(b'AT+CMGF=1')
            time.sleep(1)
            _serial.write(b'AT+CMGS="+2348168994374"')
            time.sleep(2)
            _serial.write(msgtxt.encode("utf-8"))
            time.sleep(1)
            ctz = chr(26)
            _serial.write(ctz.encode("utf-8"))
            time.sleep(5)
            _serial.flushInput()
            configureMQTT()
    except:
        print("no serial")
        setSerialstatus(False)


print("Starting")
#configureMQTT()
#connectSerial()



def createkmlfile():
    kmlfile = open(kmlpath, "a")
    kmlfile.write("""<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2">
<Document>
<Style id="sn_ylw-pushpin">
		<IconStyle>
			<color>ff0000ff</color>
			<scale>1.5</scale>
			<Icon>
				<href>http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png</href>
			</Icon>
			<hotSpot x="20" y="2" xunits="pixels" yunits="pixels"/>
		</IconStyle>
		<LabelStyle>
			<scale>1.1</scale>
		</LabelStyle>
		<BalloonStyle>
		</BalloonStyle>
		<ListStyle>
		</ListStyle>
	</Style>
	<Style id="sh_ylw-pushpin">
		<IconStyle>
			<color>ff0000ff</color>
			<scale>1.77273</scale>
			<Icon>
				<href>http://maps.google.com/mapfiles/kml/pushpin/ylw-pushpin.png</href>
			</Icon>
			<hotSpot x="20" y="2" xunits="pixels" yunits="pixels"/>
		</IconStyle>
		<LabelStyle>
			<scale>1.1</scale>
		</LabelStyle>
		<BalloonStyle>
		</BalloonStyle>
		<ListStyle>
		</ListStyle>
	</Style>
	<StyleMap id="msn_ylw-pushpin">
		<Pair>
			<key>normal</key>
			<styleUrl>#sn_ylw-pushpin</styleUrl>
		</Pair>
		<Pair>
			<key>highlight</key>
			<styleUrl>#sh_ylw-pushpin</styleUrl>
		</Pair>
	</StyleMap>
  
</Document></kml>""")
    kmlfile.close()


def get_geoCoding(cquery):
    gotArea = ""
    response = requests.get(cquery)
    data = response.json()
    for i in data:
        if i == "results":
            try:
                # print(data["results"][0]["address_components"][3]["long_name"])
                gotArea = data["results"][0]["address_components"][3]["long_name"]
                # print(j["formatted_address"])
            except:
                print("index error")
    return gotArea


def resetconnections():
    global reconnecttimetracker
    if (time.time() - reconnecttimetracker) > reconnecttime:
        print("checking connection status")
        ...
        if not mqttconnected:  # reconnect mqtt
            ...
            configureMQTT()
        if not serialconnected:  # reconnect serial
            ...
            connectSerial()
            setupA9G()
            ...
        reconnecttimetracker = time.time()


while 1:
    resetconnections()
    if serialconnected and mqttconnected:
        inmsg = readfromserial()
        if (time.time() - timetracker) > acquisitiontime:
            if inmsg != '':
                if 'GNGGA' in inmsg:
                    gpsdata = inmsg
                    # print(gpsdata)
                    if airqdata != '':
                        dataline = airqdata + ',' + gpsdata
                        dataline = dataline.replace("\n", "")
                        print(dataline)
                        # write to dated logfile
                        datalogfile = open(
                            strftime('%d-%m-%Y') + "_airqlog.csv", "a")
                        datalogfile.write(dataline)
                        datalogfile.close()
                        # create dated kml file for google earth
                        ind_data = gpsdata.split(",")
                        # get latitude
                        pos = ind_data[2].find('.')
                        lat = ind_data[2]
                        latdec = lat[(pos-2):(pos+5)]
                        lat = lat.replace(latdec, "")
                        lat = str(float(lat) + (float(latdec)/60))
                        if ind_data[3] == "S":
                            lat = "-" + lat

                        # get longitude
                        pos = ind_data[4].find('.')
                        lon = ind_data[4]
                        londec = lon[(pos-2):(pos+5)]
                        lon = lon.replace(londec, "")
                        lon = str(float(lon) + (float(londec)/60))
                        if ind_data[5] == "W":
                            lon = "-" + lon

                        # get altitude
                        alt = ind_data[9]

                        # get area name from lat/long
                        customquery = apiquery.replace("latitudehere", lat)
                        customquery = customquery.replace("longitudehere", lon)
                        cutdowndatastring = airqdata.replace("{", "")
                        cutdowndatastring = cutdowndatastring.replace("}", "")
                        cutdowndatastring = cutdowndatastring.replace(" ", "")
                        cutdowndatastring = cutdowndatastring.replace("\"", "")
                        # add location to string
                        cutdowndatastring += "," + get_geoCoding(customquery)
                        print(cutdowndatastring)
                        # publish string to map
                        ret = client.publish("map", cutdowndatastring)

                        kmlpath = strftime('%d-%m-%Y') + "_airqlog.kml"
                        try:
                            kmlfile = open(kmlpath, "r")
                        except FileNotFoundError:
                            print("no file")
                            createkmlfile()  # create kml file for the day

                        try:
                            kmlfile = open(kmlpath, "r")
                            dta = kmlfile.read()
                            kmlfile.close()
                            ind_airdata = airqdata.split(",")
                            coord_name = ind_airdata[11]
                            coord_name = coord_name.replace("}", "")
                            # generate kml point
                            dtaline = """<Placemark>
                <name>"""+coord_name+"""</name>
            <styleUrl>#msn_ylw-pushpin</styleUrl>
                <description>"""+airqdata+"""</description>
                <Point>
                <coordinates>""" + lon + """,""" + lat + """,""" + alt + """</coordinates>
                </Point>
            </Placemark>"""
                            dta = dta.replace("</Document></kml>",
                                            dtaline + "</Document></kml>")
                            kmlfile = open(kmlpath, "w")
                            kmlfile.write(dta)
                            kmlfile.close()
                        except:
                            ...
                        airqdata = ''
                        timetracker = time.time()

                # print(inmsg)
                inmsg = ''
