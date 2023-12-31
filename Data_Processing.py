import numpy as np
from sklearn.impute import SimpleImputer

def validate_data(sensor_data):
    # Check for missing values
    if any(value is None for value in sensor_data.values()):
        return False

    # Verify data types and ranges (modify as per your sensor specifications)
    if not (20 <= sensor_data['temperature'] <= 40):
        return False

    # Add more validation rules as needed

    return True

def identify_outliers(sensor_data):
    # Assuming 'temperature' is the key for the sensor data
    temperature_values = [sensor['temperature'] for sensor in sensor_data]

    # Calculate mean and standard deviation
    mean_temp = np.mean(temperature_values)
    std_temp = np.std(temperature_values)

    # Identify outliers based on z-score (adjust the threshold as needed)
    z_scores = [(temp - mean_temp) / std_temp for temp in temperature_values]
    outliers = [sensor for sensor, z_score in zip(sensor_data, z_scores) if abs(z_score) > 3]

    return outliers

def impute_data(sensor_data):
    # Assuming 'temperature' is the key for the sensor data
    temperature_values = [sensor['temperature'] for sensor in sensor_data]

    # Use mean imputation (you can choose other strategies based on your data)
    imputer = SimpleImputer(strategy='mean')
    imputed_temps = imputer.fit_transform(np.array(temperature_values).reshape(-1, 1))

    # Update sensor_data with imputed values
    for sensor, imputed_temp in zip(sensor_data, imputed_temps):
        sensor['temperature'] = imputed_temp[0]

    return sensor_data

def filter_unreliable_data(sensor_data, unreliable_indices):
    # Assuming 'temperature' is the key for the sensor data
    filtered_data = [sensor for i, sensor in enumerate(sensor_data) if i not in unreliable_indices]
    return filtered_data


sensor_data = [
    {'temperature': 25, 'humidity': 60, 'pressure': 1015},
    {'temperature': None, 'humidity': 62, 'pressure': 1010},
    {'temperature': 30, 'humidity': 58, 'pressure': 1020},
  
]

# Assessing Data Reliability
unreliable_indices = [i for i, sensor in enumerate(sensor_data) if not validate_data(sensor)]
outliers = identify_outliers(sensor_data)

# Cleansing Unreliable Data
sensor_data = filter_unreliable_data(sensor_data, unreliable_indices)
sensor_data = impute_data(sensor_data)

# Print the cleansed sensor data
print("Cleansed Sensor Data:")
for sensor in sensor_data:
    print(sensor)
