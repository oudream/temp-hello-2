#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('start warmup...')
  cl.write_object("/operation/control/request", [0x41])
  
  print('wait until terminated and idle again...')
  while(True):
    time.sleep(1.0)
    confirm = cl.read_object("/operation/control/confirm")
    print(f'confirm={confirm[0]}, phase={confirm[2]}')
    if confirm[0] == 0x10 and confirm[2] == 2:
      break

  
