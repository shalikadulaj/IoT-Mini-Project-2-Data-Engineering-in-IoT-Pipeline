from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import MQTTSNclient
import json

jsonP = ''

# clients for MQTT and MQTTS
MQTTClient = AWSIoTMQTTClient("MQTTSNbridge")
MQTTSNClient = MQTTSNclient.Client("bridge", port=1885)

class Callback:
  # function that replies a message from the MQTTSN broker to the MQTT one
  # and inserts into the database the message just arrived
  def messageArrived(self, topicName, payload, qos, retained, msgid):
      message = payload.decode("utf-8")
      jsonP = json.loads(message) 
      print(topicName, message)
      MQTTClient.publish(topicName, message, qos)
      #dynamoTable.put_item(Item=jsonP)
      return True


# path that indicates the certificates position
path = "/home/root/IoT-Mini-Project-1/certs/"

# configure the access with the AWS MQTT broker
MQTTClient.configureEndpoint("a5hi9k1blxeai-ats.iot.eu-west-1.amazonaws.com", 8883)
MQTTClient.configureCredentials(path+"AmazonRootCA1.pem",
                                path+"41caa97615d76184172427b6764bded91448dede2bfe1f8a7411c59de3300978-private.pem.key",
                                path+"41caa97615d76184172427b6764bded91448dede2bfe1f8a7411c59de3300978-certificate.pem.crt")

# configure the MQTT broker
MQTTClient.configureOfflinePublishQueueing(-1)  # Infinite offline Publish queueing
MQTTClient.configureDrainingFrequency(2)  # Draining: 2 Hz
MQTTClient.configureConnectDisconnectTimeout(10)  # 10 sec
MQTTClient.configureMQTTOperationTimeout(5)  # 5 sec

# register the callback
MQTTSNClient.registerCallback(Callback())

# make connections to the clients
MQTTClient.connect()
MQTTSNClient.connect()


while True:

    MQTTSNClient.subscribe("sensor/station1")

# disconnect from the clients
MQTTSNClient.disconnect()
MQTTClient.disconnect()
