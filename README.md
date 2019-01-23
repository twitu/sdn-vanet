# Introduction
This project tries to extend the Mininet-wifi to simulate SDN based VANET networks.

# Instructions for usage
1. Download Mininet-wifi image and run on Virtual Box
2. paste or `git clone` this repository in the virtual machine in `~/mininet-wifi/examples/` directory
3. `cd` into folder
4. run `sudo python stationary-test.py`
5. use `xterm sta<id>` in Mininet-wifi CLI where to id is node id from 1, to connect to nodes
6. `./run.sh <id> debug` to execute C application in debug mode in the node
7. `quit` in Mininet-wifi CLI to exit
