#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('prepare guard...')
  cl.write_object("/system/network/guard/conDev/timeout", [3.0])
  cl.write_object("/system/network/guard/conDev/enable", [True])


  print('prepare setpoints...')
  cl.write_object("/operation/setpoint/values/hv", [60000])
  cl.write_object("/operation/setpoint/values/emission", [0.000100])

  print('start xray...')
  cl.write_object("/operation/control/request", [0x30])
  
  
  print('wait 10 seconds while guarded...')
  for i in range(10):
    time.sleep(1.0)
    cl.read_object("/system/network/guard/conDev/poll")


  print('wait 10 seconds while not guarded...')
  time.sleep(10.0)
  
  print('expect stopped...')
  confirm = cl.read_object("/operation/control/confirm")
  print(f'confirm={confirm[0]}, phase={confirm[2]}')


  
