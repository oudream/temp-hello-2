#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('read allowed operations...')
  allowed = cl.read_object("/operation/control/allowed")
  print(" ".join([hex(x) for x in allowed]))
  
  
  
