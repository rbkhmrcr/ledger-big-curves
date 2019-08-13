# Sample Large Group Operations App for Ledger Blue & Ledger Nano S

This application demonstrates a more complex user interface, the Secure Element
proxy logic, cryptographic APIs and flash storage.

Run `make load` to build and load the application onto the device. After
installing and running the application, you can run `demo.py` to test a
signature over USB. -- This has been upgraded from Ledger's version that
uses python2 to use python3 instead so you'll have to use `python3 demo.py`.
I think this makes the python much more readable. The errors (as always) 
try to be helpful, but if you are getting an error that gives almost no 
information, the first thing I'd recommend is always to check that your
Ledger device is unlocked. 

See [Ledger's documentation](http://ledger.readthedocs.io) for more info.
