#!/usr/bin/expect -f

# Start the client program
spawn ./client

# Wait for the prompt and then login
expect "Please enter command: "
send "/login jill 123 128.100.13.206 55000\r"

# Wait for the next prompt and create a session
expect "Please enter command: "
send "/createsession lab_help\r"

# Wait for the next prompt and create a session
expect "Please enter command: "
send "/createsession hi\r"

# Send a message
expect "Please enter command: "
send "Nice to meet u!\r"

# leave session 
expect "Please enter command: "
send "/leavesession\r"

# private msg
expect "Please enter command: "
send "/privatemsg amy hiiiiii\r"


# Keep the session open to interact or listen
interact
