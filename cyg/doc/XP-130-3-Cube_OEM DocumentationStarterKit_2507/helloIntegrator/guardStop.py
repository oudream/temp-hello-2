#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('stop guard...')
  cl.write_object("/system/network/guard/conDev/enable", [False])


  
