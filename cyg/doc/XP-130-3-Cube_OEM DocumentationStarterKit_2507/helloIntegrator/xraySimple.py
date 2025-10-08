#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('prepare setpoints...')
  cl.write_object("/operation/setpoint/values/hv", [60000])
  cl.write_object("/operation/setpoint/values/emission", [0.000100])

  print('start xray...')
  cl.write_object("/operation/control/request", [0x30])
  
  
  print('wait 10 seconds...')
  time.sleep(10.0)

  print('stop xray...')
  cl.write_object("/operation/control/request", [0x10])
  time.sleep(1.0)


  
