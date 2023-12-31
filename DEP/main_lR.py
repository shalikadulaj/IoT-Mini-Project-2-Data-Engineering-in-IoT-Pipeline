from sklearn.linear_model import LinearRegression
from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import MQTTSNclient
import json
import numpy as np
import threading
from datetime import datetime


# Global list for storing temperature and pressure data
temperature_data = []
filtered_temperature_data = []
pressure_data = []
filtered_pressure_data = []

# Shared data structure and lock for synchronized access
combined_data = {
    "temperature": None,
    "pressure": None,
    "site": "Grenoble"  # Adding site name
}
data_lock = threading.Lock()

def extract_and_store_data(message):
    try:
        jsonP = json.loads(message)

        # Function to calculate Z-score
        def calculate_z_score(value, data):
            mean = np.mean(data)
            std = np.std(data)
            if std == 0:  # Avoid division by zero
                return 0
            return (value - mean) / std

        # Detect and handle outliers for temperature
        if 'temperature' in jsonP:
            temp_value = float(jsonP['temperature'])
            if len(temperature_data) >= 10:  # Ensure sufficient data for calculation
                temp_z_score = calculate_z_score(temp_value, temperature_data[-10:])
                print(temp_z_score)
                if abs(temp_z_score) > 3:  # Outlier detected
                    print("Outlier detected in temperature:", temp_value)
                    temp_value = np.median(temperature_data[-10:])  # Replace with median
                    # Or, skip adding: continue
                    # Or, replace with the last valid value: temp_value = temperature_data[-1]
            temperature_data.append(temp_value)

        # Detect and handle outliers for pressure
        if 'pressure' in jsonP:
            press_value = float(jsonP['pressure'])
            if len(pressure_data) >= 10:  # Ensure sufficient data for calculation
                press_z_score = calculate_z_score(press_value, pressure_data[-10:])
                if abs(press_z_score) > 3:  # Outlier detected
                    print("Outlier detected in pressure:", press_value)
                    press_value = np.median(pressure_data[-10:])  # Replace with median
                    # Or, skip adding: continue
                    # Or, replace with the last valid value: press_value = pressure_data[-1]
            pressure_data.append(press_value)

        #print("Data extracted with outlier detection: Temperature and Pressure")
    except Exception as e:
        print("Error in extract_and_store_data:", e)



def median_filter(data):
    return np.median(data)

def calculate_average(data):
    return sum(data) / len(data) if data else 0

def process_temperature_data():
    global combined_data
    while True:
        if len(temperature_data) >= 10:
            # Prepare data for regression
            X = np.arange(len(temperature_data[-10:])).reshape(-1, 1)
            y = np.array(temperature_data[-10:])

            # Create and train the regression model
            model = LinearRegression().fit(X, y)

            # Predict the next temperature value
            predicted_value = model.predict([[len(temperature_data)]])[0]

            # Update combined_data with the predicted value
            with data_lock:
                combined_data["temperature"] = predicted_value
                publish_combined_data()

            # Remove processed data
            del temperature_data[:10]            
            with data_lock:
                combined_data["temperature"] = filtered_value
                publish_combined_data()
def process_pressure_data():
    global combined_data
    while True:
        if len(pressure_data) >= 10:
            # Prepare data for regression
            X = np.arange(len(pressure_data[-10:])).reshape(-1, 1)
            y = np.array(pressure_data[-10:])

            # Create and train the regression model
            model = LinearRegression().fit(X, y)

            # Predict the next pressure value
            predicted_value = model.predict([[len(pressure_data)]])[0]

            # Update combined_data with the predicted value
            with data_lock:
                combined_data["pressure"] = predicted_value
                publish_combined_data()

            # Remove processed data
            del pressure_data[:10]
            with data_lock:
                combined_data["pressure"] = filtered_value
                publish_combined_data()

def publish_combined_data():
    global combined_data
    if combined_data["temperature"] is not None and combined_data["pressure"] is not None:
        
        current_datetime = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        combined_data["timestamp"] = current_datetime
        
        topicName = "Grenoble/Data"
        message = json.dumps(combined_data)
        MQTTClient.publish("Grenoble/Data", message, 1)  # Example topic
        print("Outgoing : ",topicName, message)
        # Reset the values after publishing
        combined_data["temperature"] = None
        combined_data["pressure"] = None

class Callback:
    def messageArrived(self, topicName, payload, qos, retained, msgid):
        message = payload.decode("utf-8")
        data_thread = threading.Thread(target=extract_and_store_data, args=(message,))
        data_thread.start()
        jsonP = json.loads(message)
        json_topic = f"sensor/node{jsonP['node']}"
        new_string_variable = f"{json_topic}"

        if json_topic in topics_to_subscribe:
            #print(f"Received message from subscribed topic: {json_topic}")
            topicName = new_string_variable
            #print("Incomming : ",topicName, message)
            #MQTTClient.publish(topicName, message, qos)
        else:
            print(f"Received message from an unsubscribed topic: {json_topic}")

        return True

# MQTT broker configuration
path = "/home/root/IoT-Mini-Project-2-Data-Engineering-in-IoT-Pipeline/certs/"
MQTTClient = AWSIoTMQTTClient("MQTTSNbridge")
MQTTSNClient = MQTTSNclient.Client("bridge", port=1885)

MQTTClient.configureEndpoint("a5hi9k1blxeai-ats.iot.us-east-1.amazonaws.com", 8883)
MQTTClient.configureCredentials(path+"AmazonRootCA1.pem", 
                                path+"cb9f87c8d846f3c7aec8b3deee46ee53cb2a7939a1972ff7c812db53ce3cc041-private.pem.key", 
                                path+"cb9f87c8d846f3c7aec8b3deee46ee53cb2a7939a1972ff7c812db53ce3cc041-certificate.pem.crt")

MQTTClient.configureOfflinePublishQueueing(-1)
MQTTClient.configureDrainingFrequency(2)
MQTTClient.configureConnectDisconnectTimeout(10)
MQTTClient.configureMQTTOperationTimeout(5)

MQTTSNClient.registerCallback(Callback())

# Connect to the clients
MQTTClient.connect()
MQTTSNClient.connect()

# Define topics for subscription
topics_to_subscribe = ["sensor/node1", "sensor/node2", "sensor/node3"]
for topic in topics_to_subscribe:
    MQTTSNClient.subscribe(topic)

# Start the data processing threads
temp_thread = threading.Thread(target=process_temperature_data)
temp_thread.start()

press_thread = threading.Thread(target=process_pressure_data)
press_thread.start()

# Keep the main thread alive
try:
    while True:
        pass
except KeyboardInterrupt:
    # Handle any cleanup here
    print("Shutting down...")

# Disconnect from the clients
MQTTSNClient.disconnect()
MQTTClient.disconnect()
print("Disconnected from MQTT clients.")
