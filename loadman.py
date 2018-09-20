import json
from collections import OrderedDict
import pprint
import sys
import subprocess

args = sys.argv
now = 0

history = {}

def load():
    print(args)
    json_file = open(args[2], 'r')
    json_data = json.load(json_file)

    #now = int(json_data["turn"])
    history[now] = [[json_data["agent_e1_x"], json_data["agent_e1_y"]],
                                  [json_data["agent_e2_x"], json_data["agent_e2_y"]]]

def check(aid):
    print(now)
    if now <= 3:
        return [-1, -1]
    if history[now][aid] == history[now - 2][aid]:
        return history[now - 1][aid]
    else:
        return [-1, -1]

def point_format(point):
    return "{0}:{1}".format(point[0], point[1])
    
if __name__ == '__main__':
    subprocess.call("./bin.d/bin init ./sample_qrformat.dat 0 break -1:-1 -1:-1", shell=True)
    if args[1] == "play":
        input()
    elif args[1] == "self":
        subprocess.call("./bin continue ./cdump.json 0 break -1:-1 -1:-1", shell=True)
    else:
        print("unknown command: " + args[1])
        quit()
    subprocess.call("echo 0 0 > score.dat", shell=True)
    for i in range(70):
        i += 1
        now = i
        load()
        blacklist = " break "
        blacklist += point_format(check(0))
        blacklist += " "
        blacklist += point_format(check(1))
        print("blacklist: " + blacklist)
        print("-----------------------------------")
        subprocess.call("./bin.d/bin continue ./cdump.json " + str(now) + blacklist, shell=True)
        if args[1] == "play":
            input()
        elif args[1] == "self":
            subprocess.call("./bin continue ./cdump.json " + str(now) + blacklist, shell=True)
        else:
            print("unknown command: " + args[1])
            quit()
        subprocess.call("./bin.d/bin gnuscore ./cdump.json " + str(now) + blacklist + " >> score.dat", shell=True)
        print("turn {0}".format(i))
    subprocess.call("./bin.d/bin score ./cdump.json", shell=True)
