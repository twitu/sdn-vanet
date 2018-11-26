ifconfig sta$1-mp0 netmask 255.255.255.0 broadcast 10.0.0.255
make $2
./local_server.out $1