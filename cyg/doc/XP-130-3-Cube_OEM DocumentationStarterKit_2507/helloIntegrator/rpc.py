#!/usr/bin/env python
import websocket
from typing import Callable
import json
import math
import threading


class Client:
    """
    this class provides access to jsonRPC server:
    - object access of data dictionary
    """

    def __init__(self, url: str, timeout: float = 30, connect_timeout: float = 2, connect_attempts: int = 1):
        """
        create the client.
        url: target url, format "ws://<ip>:<port>"
        timeout: socket timeout while accessing objects on the connected target in seconds
        connect_timeout: socket timeout while establishing a connection with the target in seconds
        connect_attempts: number of times a connection with the target should be attempted
        """
        self._url = url
        self._id = 0
        self.socket = None
        self.connect_timeout = connect_timeout
        self.connect_attempts = connect_attempts
        self.access_timeout = timeout
        self.mutex = threading.RLock()

    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    @staticmethod
    def __create_json_rpc_message(method, params, message_id):
        # TODO: call get_id() here an remove message_id parameter
        message = {'jsonrpc': "2.0", 'method': method, 'params': params, 'id': message_id}
        return json.dumps(message)

    @staticmethod
    def __create_params_payload_write(resource, value):
        return {'resource': resource, 'value': value}

    @staticmethod
    def __create_params_payload_read(resource):
        return {'resource': resource}

    def __get_id(self):
        self._id = self._id + 1
        return self._id

    def read_object(self, resource: str):
        """
        read access to object from data dictionary
        resource: the resource path
        return: the object value
        """
        id = self.__get_id()
        request = Client.__create_json_rpc_message(method='read', 
                                                   params=Client.__create_params_payload_read(resource), 
                                                   message_id=id)
        reply = self.__access(request)
        parsed = json.loads(reply)
        if not parsed or 'error' in parsed:
            raise ValueError("read failed\n request: " + request + "\n reply: " + str(reply))
        elif parsed['id'] != id:
            self.close()
            txt = f"request and reply are desynchronized: request={id} reply={parsed['id']}"
            raise Exception(txt)
        else:
            return parsed['result']['value']

    def write_object(self, resource: str, value: list):
        """
        write access to object from data dictionary
        resource: the resource path
        value: the object value
        """
        id = self.__get_id()
        request = Client.__create_json_rpc_message(method='write',
                                                   params=Client.__create_params_payload_write(resource, value),
                                                   message_id=id)
        reply = self.__access(request)
        parsed = json.loads(reply)
        if not parsed or 'error' in parsed:
            raise ValueError("write failed\n request: " + request + "\n reply: " + str(reply))
        elif parsed['id'] != id:
            self.close()
            txt = f"request and reply are desynchronized: request={id} reply={parsed['id']}"
            raise Exception(txt)

    def __access(self, request: str):
        with self.mutex:
            try:
                self.socket.send(request)
                reply = self.socket.recv().strip()
            except Exception: #retry on any connection problems on an etablished socket, or not yet open connection...
                #attempt to close...
                if self.socket:
                    try:
                        self.socket.close()
                    except Exception:
                        pass
                #(re)open socket...
                attempt_count = 0
                while True:
                    try:
                        self.socket = websocket.create_connection(self._url, timeout=self.connect_timeout)
                        break
                    except Exception:
                        attempt_count += 1
                        if attempt_count >= self.connect_attempts:
                            raise
                self.socket.settimeout(self.access_timeout)
                self.socket.send(request)
                reply = self.socket.recv().strip()
            return reply

    def close(self):
        """
        explicitely close the connection.
        """
        with self.mutex:
            if self.socket is not None:
                self.socket.close()




