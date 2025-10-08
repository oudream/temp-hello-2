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
  
  
  
  
  print('initialize supervision...')
  names = cl.read_object("/operation/supervision/device/yNames")
  oldHash = cl.read_object("/operation/supervision/device/hash")

  
  print('poll supervision, hash-based...')
  while True:
    time.sleep(1.0)
    print('.')
    newHash = cl.read_object("/operation/supervision/device/hash")
    #has supervision changed ?
    if newHash != oldHash:
      print('supervision has changed...')
      oldHash = newHash
      #present results...
      states = cl.read_object("/operation/supervision/device/states")
      for n,s in zip(names, states):
        print(f'{n}: {stateToName(s)}')

  
  
  
  
