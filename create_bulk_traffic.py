import sys
import random
import string

if __name__ == "__main__":
    N = int(sys.argv[1])  # number of routers to simulate
    M = int(sys.argv[2])  # number of types of files to create
    L = int(sys.argv[3])  # number of packets to be generated per file

    for i in range(1, M, 1):
        with open("traffic.{}.data".format(i), "w") as f:
            for j in range(L):
                # generate random lower case strings of length 30
                # destined to a randomly selected node
                f.write("{}\n{}\n".format(random.randint(1, N) , ''.join(random.choice(string.ascii_lowercase) for x in range(30))))
            f.write("-1\n")
