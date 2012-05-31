from time import time

class Meter(object):

    def __init__(self, initial=0):
        self.__offset= None;
        self.__count = initial;

    def sample(self, value):
        timestamp = time()

        if value < self.__count:
            self.__offset = self.__count
        elif self.__offset is None:
            self.__offset = value;

        self.__count = self.__offset + value

    def count(self):
        return self.__count
