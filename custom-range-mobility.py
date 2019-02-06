#!/usr/bin/python

"""This example shows how to work in adhoc mode

It is a full mesh network

     .sta3.
    .      .
   .        .
sta1 ----- sta2"""

import sys

from mininet.log import setLogLevel, info
from mn_wifi.link import wmediumd, mesh, adhoc
from mn_wifi.cli import CLI_wifi
from mn_wifi.net import Mininet_wifi
from mn_wifi.wmediumdConnector import interference
from threading import Thread
from sysv_ipc import SharedMemory

import time
import sysv_ipc


class Topology(Thread):

    def __init__(self):
        Thread.__init__(self)
        self.mobility = None
        self.stations = None

    def assign(self, mobility, stations):
        self.mobility = mobility
        self.stations = stations


    def run(self):
        setLogLevel('info')

        "Create a network."
        net = Mininet_wifi(link=wmediumd, wmediumd_mode=interference)

        info("*** Creating nodes\n")

        self.stations.append(net.addStation('sta1', min_v=1, max_v=2))
        self.stations.append(net.addStation('sta2', min_v=1, max_v=2))
        self.stations.append(net.addStation('sta3', min_v=1, max_v=2))

        # use to configure range of wifi nodes
        # do not use addStation(range=..), because it does not work properly with the communication model
        # exp = 4 gives approx range of 65
        net.setPropagationModel(model="logDistance", exp=4)

        info("*** Configuring wifi nodes\n")
        net.configureWifiNodes()

        info("*** Creating links\n")
        net.addLink(self.stations[0], cls=adhoc, ssid='adhocNet', mode='g', channel=5)
        net.addLink(self.stations[1], cls=adhoc, ssid='adhocNet', mode='g', channel=5)
        net.addLink(self.stations[2], cls=adhoc, ssid='adhocNet', mode='g', channel=5)

        net.plotGraph(max_x=100, max_y=100)
        net.startMobility(time=0, model='RandomDirection', max_x=100, max_y=100, seed=20)

        info("*** Starting network\n")
        net.build()

        info("*** Running CLI\n")
        CLI_wifi(net)

        info("*** Stopping network\n")
        net.stop()


class Position(Thread):

    def __init__(self):
        Thread.__init__(self)

    def assign(self, stations):
        self.work = True
        self.stations = stations

    def run(self):
        time.sleep(3)

        shared_mem = SharedMemory(6789, flags=sysv_ipc.IPC_CREAT, size=sysv_ipc.PAGE_SIZE, init_character='\0')

        try:
            while self.work:
                for i in range(len(stations)): # convert position to integer then string and fill with 4 zero to align values
                    shared_mem.write(' '.join(str(pos).split(".")[0].zfill(4) for pos in stations[i].params["position"]) + ' ', i*15)
            else:
                print(shared_mem.read(45))
                shared_mem.remove()
        except:
            shared_mem.remove()

    def finish(self):
        self.work = False


if __name__ == '__main__':
    stations = []

    pos = Position()
    top = Topology()
    pos.assign(stations)
    top.assign(True, stations)

    top.start()
    pos.start()
    top.join()
    pos.finish()
    pos.join()