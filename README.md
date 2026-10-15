<div align="center">

# Watermelon

</div>

A work in progress Minecraft server written in C++ aiming to be fast and easily customizable. The goal of Watermelon is to work like a real Minecraft server while being way faster and having more features.

## What's working right now ?

Only a few bits are currently right now, the architecture of the project is still being worked on and features takes time to be implemented because almost nothing is already built for the moment.

The main things that are working right now are:

Configuration:
- Enabling java server
- Java server address and port
- Some java server properties like view and simulation distance, max player count and motd
- Some logger properties like the file where to write logs and the minimum level to log

Networking:
- The TCP stack to comunicate with Minecraft clients
- Parsing and serializing of java packets
- Basic server status/ping
- Basic login and configuration sequence

## What's next ?

I'm still adding the bare minimum to be able to join a world for now so the main features to implement are:

Networking:
- Send basic player info
- Send basic chunk and light data
- Send empty inventory

When this will be done I'll do back and add the finishing touches on the already working features to make them fully completed

Configuration:
- Add more config fields for the java server and the logger

Status:
- Send real player data on the status screen (player count and some online player data)
- Send the favicon on the status screen

Networking:
- Add compression
- Add encryption
- Add skin and cape data
- Implement remaining configuration state packages
- Implement remaining play state packages that are sent duriong login

Map and player data:
- Read real chunk from a map
- Sent real player position when logging in
- Sent real player inventory
