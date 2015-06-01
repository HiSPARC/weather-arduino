import serial
import datetime
import time
import collections
import numpy
import sys, serial
import time
import MySQLdb
import hashlib
import requests
import logging

import cPickle as pickle

from time import sleep
from requests.exceptions import ConnectionError, Timeout

logger = logging.getLogger(__name__)

DATASTORE_URL = "http://frome.nikhef.nl/hisparc/upload"


class Measurement(object):

    def __init__(self, output):

        # Datastore assumes date, time, temp_inside (detector), tempOutside, humidityInside ,
        # humidityOutside, barometer, windDir, windSpeed, solarRad, UV, ET, rainRate
        # heatIndex, dewPoint, windChill

        self.temp_inside, self.temp_outside, self.humidity_outside, self.barometer = output

        Tdew = self.dampdruk_calc(self.temp_outside, self.humidity_outside, self.barometer)

        # As we do not measure these weather variables we set them to '-999'
        # If the weatherstation does measure these variables we can add them to the list
        # extract them from the Arduino output.

        self.wind_dir = '-999'
        self.humidity_inside = '-999'
        self.wind_speed = '-999'
        self.solar_rad  = '-999'
        self.uv = '-999'
        self.evapotranspiration = '-999'
        self.rain_rate = '-999'
        self.heat_index = '-999'
        self.dew_point = Tdew
        self.wind_chill = '-999'

        # Add timetstamp of measurement
        self.datetime = datetime.datetime.now()
        self.nanoseconds = 0


    def dampdruk_calc(self, Tout, Hum_out, baro):
        RH = Hum_out/100  #calculate relative humidity

        # calculate vaporpressure, Dewpoint: Formula from Vantage Pro Davis instruments

        dampdruk = RH * 6.112 * numpy.exp((17.62 * Tout)/(Tout + 243.12))
        Numerator = 243.12 * numpy.log(dampdruk)-440.1
        Denominator = 19.43 - numpy.log(dampdruk)
        Tdew = Numerator / Denominator
        Tdew = "%4.1f" %Tdew

        return Tdew


class NikhefDataStore(object):

    """Send events over HTTP to the datastore at Nikhef.
    A call to :meth:`store_event` will send the event to the datastore at
    Nikhef, using the convoluted datastructure which was created for the
    old eventwarehouse, and still survives to this day.
    """

    def __init__(self, station_id, password):
        """Initialize the datastore.
        Each station has a unique station number / password combination.
        Provide this during initialization.
        """
        self.station_id = station_id
        self.password = password

    def store_event(self, event):
        """Store an event.
        Override this method.
        """
        data = self._create_event_container(event)
        self._upload_data(data)

    def _create_event_container(self, event):
        """Encapsulate an event in a container for the datastore.
        This hurts.  But it is necessary for historical reasons.
        :param event: HiSPARC event object.
        :returns: container for the event data.
        """
        header = {'eventtype_uploadcode': 'WTR',
                  'datetime': event.datetime,
                  'nanoseconds': event.nanoseconds}

        datalist = []
        self._add_value_to_datalist(datalist, 'WTR_TEMP_INSIDE', event.temp_inside)
        self._add_value_to_datalist(datalist, 'WTR_TEMP_OUTSIDE', event.temp_outside)
        self._add_value_to_datalist(datalist, 'WTR_HUMIDITY_INSIDE', event.humidity_inside)
        self._add_value_to_datalist(datalist, 'WTR_HUMIDITY_OUTSIDE', event.humidity_outside)
        self._add_value_to_datalist(datalist, 'WTR_BAROMETER', event.barometer)
        self._add_value_to_datalist(datalist, 'WTR_WIND_DIR', event.wind_dir)
        self._add_value_to_datalist(datalist, 'WTR_WIND_SPEED', event.wind_speed)
        self._add_value_to_datalist(datalist, 'WTR_SOLAR_RAD', event.solar_rad)
        self._add_value_to_datalist(datalist, 'WTR_UV', event.uv)
        self._add_value_to_datalist(datalist, 'WTR_ET', event.evapotranspiration)
        self._add_value_to_datalist(datalist, 'WTR_RAIN_RATE', event.rain_rate)
        self._add_value_to_datalist(datalist, 'WTR_HEAT_INDEX', event.heat_index)
        self._add_value_to_datalist(datalist, 'WTR_DEW_POINT', event.dew_point)
        self._add_value_to_datalist(datalist, 'WTR_WIND_CHILL', event.wind_chill)

        event_list = [{'header': header, 'datalist': datalist}]
        return event_list

    def _add_value_to_datalist(self, datalist, upload_code, value):
        """Add an event value to the datalist.
        :param datalist: datalist object (for upload).
        :param upload_code: the upload code (eg. 'TRIGPATTERN').
        :param value: the value to store in the datalist.
        """
        datalist.append({'data_uploadcode': upload_code,
                         'data': value})

    def _add_values_to_datalist(self, datalist, upload_code, values):
        """Add multiple event values to datalist.
        Takes a list of values and a partial upload code (e.g. 'PH') and
        adds them to the datalist as 'PH1', 'PH2', etc.
        :param datalist: datalist object (for upload).
        :param upload_code: the partial upload code (eg. 'PH').
        :param values: list of values to store in the datalist.
        """
        for idx, value in enumerate(values, 1):
            self._add_value_to_datalist(datalist, upload_code + str(idx),
                                        value)

    def _upload_data(self, data):
        """Upload event data to server.
        :param data: container for the event data.
        """
        pickled_data = pickle.dumps(data)
        checksum = hashlib.md5(pickled_data).hexdigest()

        payload = {'station_id': self.station_id,
                   'password': self.password, 'data': pickled_data,
                   'checksum': checksum}
        try:
            r = requests.post(DATASTORE_URL, data=payload, timeout=10)
            r.raise_for_status()
        except (ConnectionError, Timeout) as exc:
            raise UploadError(str(exc))
        else:
            logger.debug("Response from server: %s", r.text)
            if r.text != '100':
                raise UploadError("Server responded with error code %s" % r.text)

    def close(self):
        """Close the datastore."""

        pass


if __name__ == '__main__':
    # Send to NikhefDatastore(Station_id, password) not known please contact info@hisparc.nl
    datastore = NikhefDataStore(599, 'pysparc')

    #location of /USB wireless connection for MAC could be tty instead of cu adress
    strPort = '/dev/cu.SLAB_USBtoUART'
    ser = serial.Serial(strPort, baudrate=9600, parity=serial.PARITY_NONE,
            bytesize=serial.EIGHTBITS, stopbits=serial.STOPBITS_ONE, timeout=1.0)
    prev_data = [2]
    while True:
        try:
          data = ser.readline()
          if data == '':  #check if data is not empty
            time.sleep(2) # change to higher sleep time
            continue
          data = [float(val) for val in data.split(',')]
          if prev_data != data and len(data) <= 26:
            prev_data = data
            measurement = Measurement(data)
            #print measurement.dew_point
            datastore.store_event(measurement)
            time.sleep(1)    # can be changed to higher sleep time
        except KeyboardInterrupt:
            print ' exiting'
            break
