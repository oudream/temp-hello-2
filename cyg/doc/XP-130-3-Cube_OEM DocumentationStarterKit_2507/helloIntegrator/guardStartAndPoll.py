#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('prepare guard...')
  cl.write_object("/system/network/guard/conDev/timeout", [3.0])
  cl.write_object("/system/network/guard/conDev/enable", [True])


  
  print('guarded forever...')
  while True:
    time.sleep(1.0)
    cl.read_object("/system/network/guard/conDev/poll")

  
