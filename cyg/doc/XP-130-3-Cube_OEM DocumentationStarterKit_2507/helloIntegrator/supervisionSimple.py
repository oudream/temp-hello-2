#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  def stateToName(s):
    if s == 1:
      return 'ready'
    elif s == 2:
      return 'warning'
    elif s == 3:
      return 'user-clearable error'
    elif s == 4:
      return 'self-clearable error'
    elif s == 5:
      return 'non-clearable error'
  
  
  print('read supervision state...')
  state = cl.read_object("/operation/supervision/device/states")[0]
  print(stateToName(state))
  
  
  
