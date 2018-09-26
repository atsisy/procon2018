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
    json_file = open(args[1], 'r')
    json_data = json.load(json_file)

    #now = int(json_data["turn"])
    history[now] = [[json_data["agent_e1_x"], json_data["agent_e1_y"]],
                                  [json_data["agent_e2_x"], json_data["agent_e2_y"]]]

def check(aid):
    print(now)
    if history[now][aid] == history[now - 2][aid]:
        return history[now - 1]
    else:
        return [-1, -1]
        
if __name__ == '__main__':
    subprocess.call("./bin init ./sample_qrformat.dat 0", shell=True)
    subprocess.call("./bin.d/bin continue ./cdump.json 0", shell=True)
    subprocess.call("echo 0 0 > score.dat", shell=True)
    for i in range(70):
        i += 1
        now = i
        print(load())
        if now > 3:
            check(0)
        print("-----------------------------------")
        subprocess.call("./bin continue ./cdump.json " + str(now), shell=True)
        subprocess.call("./bin.d/bin continue ./cdump.json " + str(now), shell=True)
        subprocess.call("./bin.d/bin gnuscore ./cdump.json " + str(now) + " >> score.dat", shell=True)
        print("turn {0}".format(i))
    subprocess.call("./bin.d/bin score ./cdump.json", shell=True)
