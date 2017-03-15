#I2C Scanner
An I2C Scanner sends a start/stop request to each of the possible addresses on the bus.  Those that are present return
with an ACK.  This can be used to detect the presence/absence of a device at that address.  The results are logged
to the serial output.  Useful for testing for the existence of a device.