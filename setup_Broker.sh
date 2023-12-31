#!/bin/bash

# File name to be created/edited
FILENAME="config.conf"

# Create and edit the file using ex (part of Vim)
ex "$FILENAME" <<EOF
i
# add some debug output
#trace_output protocol
allow_anonymous true
   
# listen for MQTT-SN traffic on UDP port 1885
listener 1885 INADDR_ANY mqtts
ipv6 true
   
# listen to MQTT connections on tcp port 1886
listener 1886 INADDR_ANY
ipv6 true
.
wq
EOF

# Execute the command and store the output in a variable
output=$(ip -6 -o addr show eth0)

# Print the output
echo "The output of the command is:"
echo "$output"


# Loop to restart the broker_mqtts process
while true; do
    # Run your command
    broker_mqtts "$FILENAME"

    # Optional: sleep for a period to avoid rapid restarts in case of immediate failure
    sleep 5

    # Check for a stop file
    if [ -f "/tmp/stop_script" ]; then
        echo "Stop file detected. Exiting the script."
        break
    fi
done
