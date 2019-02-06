# $1 is node id
# $2 is total number of nodes to be deployed
# $3 is build mode which can be compile, test, debug or info

ifconfig sta$1-wlan0 netmask 255.255.255.0 broadcast 10.0.0.255
make $3
./local_server.out $1 $2 < traffic.$1.data > log.$1.txt
