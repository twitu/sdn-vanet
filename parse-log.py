import sys
import os
import matplotlib.pyplot as plt

from datetime import datetime


class Packet(object):
    __slots__ = ['sender', 'index', 'time']

    def __init__(self, sender, index):
        self.sender = sender
        self.index = index
        self.time = []

    def __hash__(self):
        return hash((self.sender, self.index))


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
                index = int(log_line[1])
                packet = packets.get((sender, index))

                if packet is None:
                    packet = Packet(sender, index)

                packet.time.append(datetime.strptime(log_line[2], "%H:%M:%S"))

    # plot graphs from the data

    # plot number of packets by number of hops
    hop_data = {}
    for packet in packets.items():
        hop_count = len(packet.time) - 1

        if hop_count not in hop_data:
            hop_data[hop_count] = 0
        hop_data[hop_count] += 1

        hop_count = hop_data.keys().sort()
        packet_count = [hop_data[count] for count in hop_count]

        plt.bar(hop_count, packet_count)
        plt.xlabel("Hops", fontsize=5)
        plt.ylabel("Packets", fontsize=5)
        plt.xticks(hop_count, hop_count, fontsize=5)
        plt.title("Hops taken by packets to reach destination")
        plt.show()
