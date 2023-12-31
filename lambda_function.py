import boto3

historical_temperature_data = []
historical_pressure_data = []

def handle_outliers(value, data, replace_high_value=0):
    if not data:
        return value, 'real'

    sorted_data = sorted(data)
    n = len(sorted_data)
    lower_percentile_index = int(0.25 * n)
    upper_percentile_index = int(0.75 * n)

    lower_percentile_value = sorted_data[lower_percentile_index]
    upper_percentile_value = sorted_data[upper_percentile_index]

    iqr = upper_percentile_value - lower_percentile_value
    outlier_multiplier = 1.5
    lower_threshold = lower_percentile_value - outlier_multiplier * iqr
    upper_threshold = upper_percentile_value + outlier_multiplier * iqr

    if value is None:
        return None, 'real'  # Handle case where value is None separately

    if value > upper_threshold or value > replace_high_value:
        return value, 'real'  # If the value is an outlier, processed value is the same as real value
    else:
        return value, 'real'


def lambda_handler(event, context):
    global historical_temperature_data, historical_pressure_data
    client = boto3.client('dynamodb')

    timestamp = event['timestamp']
    site = event['site']
    
    temperature = event.get('temperature', None)
    pressure = event.get('pressure', None)

    if temperature is not None:
        temperature = float(temperature)
        historical_temperature_data.append(temperature)

    if pressure is not None:
        pressure = float(pressure)
        historical_pressure_data.append(pressure)

    temperature_value, temperature_type = handle_outliers(temperature, historical_temperature_data)
    pressure_value, pressure_type = handle_outliers(pressure, historical_pressure_data)

    dynamodb_item = {
        'timestamp': {'S': timestamp},
        'site': {'S': site},
    }

    if temperature is not None:
        dynamodb_item['temperature_real'] = {'N': str(temperature)}
        dynamodb_item['temperature_processed'] = {'N': str(temperature_value)}

    if pressure is not None:
        dynamodb_item['pressure_real'] = {'N': str(pressure)}
        dynamodb_item['pressure_processed'] = {'N': str(pressure_value)}

    response = client.put_item(
        TableName='table_v3',
        Item=dynamodb_item
    )

    return 0
