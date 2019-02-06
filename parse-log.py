import sys
import os
import matplotlib.pyplot as plt

from datetime import datetime

class Time(object):
    __slots__ = ['sec', 'usec']

    def __init__(self, sec, usec):
        self.sec = sec
        self.usec = usec

class Packet(object):
    __slots__ = ['sender', 'dest', 'index', 'time']

    def __init__(self, sender, dest, index):
        self.sender = sender
        self.dest = dest
        self.index = index
        self.time = []

if __name__ == "__main__":
    if len(sys.argv) != 2:
        exit("please specify directory for logs")
    directory = sys.argv[1]

    # store packet data in dictionary
    packets = {}

    # parse log files to store packet data
    for log_file in os.listdir(directory):
        with open(os.path.join(directory, log_file)) as log:
            # sample log line format: sender packet_index hh:mm:ss
            for log_line in log.readlines():
                # parse line
                log_line = log_line.split()
                sender = int(log_line[0])
                dest = int(log_line[1])
                index = int(log_line[2])
                packet = packets.get((sender, index))

                if packet is None:
                    packet = Packet(sender, dest, index)
                    packets[(sender, index)] = packet

                # TODO parse timeval format time
                packet.time.append(log_line[3])

    # plot graphs from the data

    # plot number of packets by number of hops
    hop_data = {}
    for key in packets.keys():
        hop_count = len(packets[key].time) - 1 # log is printed at destination also

        if hop_count not in hop_data:
            hop_data[hop_count] = 0
        hop_data[hop_count] += 1

    hop_count = sorted(hop_data.keys())
    packet_count = [hop_data[count] for count in hop_count]

    plt.bar(hop_count, packet_count)
    plt.xlabel("Hops", fontsize=20)
    plt.ylabel("Packets", fontsize=20)
    plt.xticks(hop_count, hop_count, fontsize=10)
    plt.title("Hops taken by packets to reach destination")
    plt.show()
