import serial
import datetime
import time
import logging
import calendar

import numpy

from pysparc import storage


DATASTORE_URL = "http://frome.nikhef.nl/hisparc/upload"


class StorageError(Exception):

    """Base error class."""

    def __init__(self, msg):
        self.msg = msg

    def __str__(self):
        return "Error storing events (%s)" % self.msg


class UploadError(StorageError):

    """Error uploading events."""

    def __str__(self):
        return "Error uploading events (%s)" % self.msg


class Measurement(object):

    def __init__(self, output):

        # Datastore assumes date, time, temp_inside (detector), tempOutside,
        # humidityInside, humidityOutside, barometer, windDir, windSpeed,
        # solarRad, UV, ET, rainRate, heatIndex, dewPoint, windChill

        # Extracts variables from output, order of variables is important.
        # Set the order of variables in the Arduino Program.
        self.temp_inside, self.temp_outside, self.humidity_outside, \
            self.barometer = output

        Tdew = self.dampdruk_calc(self.temp_outside, self.humidity_outside,
                                  self.barometer)

        # As we do not measure these weather variables we set them to '-999'
        # If the weatherstation does measure these variables we can add
        # them to the list extract them from the Arduino output.

        self.wind_dir = '-999'
        self.humidity_inside = '-999'
        self.wind_speed = '-999'
        self.solar_rad = '-999'
        self.uv = '-999'
        self.evapotranspiration = '-999'
        self.rain_rate = '-999'
        self.heat_index = '-999'
        self.dew_point = Tdew
        self.wind_chill = '-999'

        # Add timetstamp of measurement
        self.datetime = datetime.datetime.now()
        self.nanoseconds = 0
        self.ext_timestamp = (calendar.timegm(self.datetime.utctimetuple()) *
                              int(1e9) + self.nanoseconds)

    def dampdruk_calc(self, Tout, Hum_out, baro):
        if Tout == '-999' and Hum_out == '-999':
            Tdew = '-999'

        else:

            RH = Hum_out / 100  # calculate relative humidity

            # calculate vaporpressure, Dewpoint: Formula from Vantage Pro Davis
            # instruments
            # This document can be found at:
            # http://docs.hisparc.nl/weather/_static/Parameter_Manual.pdf

            dampdruk = RH * 6.112 * numpy.exp((17.62 * Tout) / (Tout + 243.12))
            Numerator = 243.12 * numpy.log(dampdruk) - 440.1
            Denominator = 19.43 - numpy.log(dampdruk)
            Tdew = Numerator / Denominator
            Tdew = "%4.1f" % Tdew

        return Tdew


class WeatherDataStore(storage.NikhefDataStore):

    def _create_event_container(self, event):
        """Encapsulate an event in a container for the datastore.
        This hurts.  But it is necessary for historical reasons.
        :param event: Weather object.
        :returns: container for the event data.
        """
        header = {'eventtype_uploadcode': 'WTR',
                  'datetime': event.datetime,
                  'nanoseconds': event.nanoseconds}

        datalist = []
        self._add_value_to_datalist(datalist, 'WTR_TEMP_INSIDE',
                                    event.temp_inside)
        self._add_value_to_datalist(datalist, 'WTR_TEMP_OUTSIDE',
                                    event.temp_outside)
        self._add_value_to_datalist(datalist, 'WTR_HUMIDITY_INSIDE',
                                    event.humidity_inside)
        self._add_value_to_datalist(datalist, 'WTR_HUMIDITY_OUTSIDE',
                                    event.humidity_outside)
        self._add_value_to_datalist(datalist, 'WTR_BAROMETER',
                                    event.barometer)
        self._add_value_to_datalist(datalist, 'WTR_WIND_DIR', event.wind_dir)
        self._add_value_to_datalist(datalist, 'WTR_WIND_SPEED',
                                    event.wind_speed)
        self._add_value_to_datalist(datalist, 'WTR_SOLAR_RAD', event.solar_rad)
        self._add_value_to_datalist(datalist, 'WTR_UV', event.uv)
        self._add_value_to_datalist(datalist, 'WTR_ET',
                                    event.evapotranspiration)
        self._add_value_to_datalist(datalist, 'WTR_RAIN_RATE', event.rain_rate)
        self._add_value_to_datalist(datalist, 'WTR_HEAT_INDEX',
                                    event.heat_index)
        self._add_value_to_datalist(datalist, 'WTR_DEW_POINT', event.dew_point)
        self._add_value_to_datalist(datalist, 'WTR_WIND_CHILL',
                                    event.wind_chill)

        event_list = [{'header': header, 'datalist': datalist}]
        return event_list


if __name__ == '__main__':
    logging.basicConfig()

    # Send to WeatherDatastore(Station_id, password)
    # If Password not known please contact info@hisparc.nl
    # e.g. datastore = WeatherDatastore(23, '12345678')
    datastore = WeatherDataStore(99, 'password')

    storage_manager = storage.StorageManager()
    storage_manager.add_datastore(datastore, 'queue_nikhef')

    # location of USB wireless connection for Windows could be tty instead of
    # cu adress
    strPort = '/dev/cu.SLAB_USBtoUART'

    ser = serial.Serial(strPort, baudrate=9600, parity=serial.PARITY_NONE,
                        bytesize=serial.EIGHTBITS,
                        stopbits=serial.STOPBITS_ONE, timeout=1.0)
    # prev_data filled for the first time. Could be empty as well
    prev_data = [0]

    while True:
        try:
            data = ser.readline()
            if data == '':  # check if data is not empty
                time.sleep(2)  # change to higher sleep time
                continue
            data = [float(val) for val in data.split(',')]
            if prev_data != data and len(data) <= 26:
                prev_data = data
                # Send data to class measurement
                measurement = Measurement(data)

                # print for checking measurement; dew_point value between 0-10
                print measurement.dew_point
                storage_manager.store_event(measurement)
                time.sleep(1)    # can be changed to higher sleep time in s.

        except KeyboardInterrupt:
            print ' exiting'
            break
