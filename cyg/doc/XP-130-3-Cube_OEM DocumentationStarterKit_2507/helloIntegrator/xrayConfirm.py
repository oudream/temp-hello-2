#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('prepare setpoints...')
  cl.write_object("/operation/setpoint/values/hv", [60000])
  cl.write_object("/operation/setpoint/values/emission", [0.000100])

  print('read back edited setpoints...')
  appliedSp = cl.read_object("/operation/setpoint/appliedValues")
  print(f'hv={appliedSp[1]}, emission={appliedSp[2]}')

  print('start xray...')
  cl.write_object("/operation/control/request", [0x30])
  
  print('wait until confirmed...')
  while(True):
    time.sleep(1.0)
    confirm = cl.read_object("/operation/control/confirm")
    print(f'confirm={confirm[0]}, phase={confirm[2]}')
    if confirm[0] == 0x30 and confirm[2] == 2:
      break

  
  print('wait 5 seconds...')
  time.sleep(5.0)

  print('stop xray...')
  cl.write_object("/operation/control/request", [0x10])
  time.sleep(1.0)

  print('wait until confirmed...')
  while(True):
    time.sleep(1.0)
    confirm = cl.read_object("/operation/control/confirm")
    print(f'confirm={confirm[0]}, phase={confirm[2]}')
    if confirm[0] == 0x10 and confirm[2] == 2:
      break

  
