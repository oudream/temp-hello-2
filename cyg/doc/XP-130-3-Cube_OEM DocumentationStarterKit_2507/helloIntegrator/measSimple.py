#!/usr/bin/env python
import rpc
import time


if __name__ == '__main__':

  cl = rpc.Client('ws://192.168.177.197:80')

  print('read measurements...')
  meas = cl.read_object("/device/sensor/meas/values")
  print(f'hv={meas[0]}, emission={meas[2]}')
  