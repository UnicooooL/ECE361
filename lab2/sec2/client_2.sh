#!/usr/bin/expect -f

# Start the client program
spawn ./client

# Wait for the prompt and then login
expect "Please enter command: "
send "/login amy 234 128.100.13.206 55000\r"

# Wait for the next prompt and list sessions
expect "Please enter command: "
send "/list\r"

# Join the session
expect "Please enter command: "
send "/joinsession lab_help\r"

# Send a message
expect "Please enter command: "
send "Hi Jill! This is Jack. How are TCP sockets different from UDP sockets?\r"

# Send a message
expect "Please enter command: "
send "Nice to meet u!\r"

# list 
expect "Please enter command: "
send "/list\r"

# private msg
expect "Please enter command: "
send "/privatemsg jill hiiiiii\r"

# logout
expect "Please enter command: "
send "/logout\r"

# Keep the session open to interact or listen
interact
