# ledger-big-curves
Schnorr signatures with big curves on ledger's secure element

### General ledger development setup
- all info is here https://ledger.readthedocs.io/en/latest/userspace/getting_started.html
- [ ] create a ledger development environment directory: 
````
mkdir ledger-dev-env && cd ledger-dev-env
````
- [ ] download gcc binary
- [ ] download clang binary with read only position independent code (fropi) support
- [ ] set the `BOLOS_ENV` variable, e.g. `export BOLOS_ENV=$HOME/ledger/ledger-dev-env`
- [ ] download the appropriate sdk (not necessarily to the same directory as above)
- [ ] set the `BOLOS_SDK` variable, e.g. `export BOLOS_SDK=$HOME/ledger/nanos-secure-sdk`
- [ ] install the python loader as shown [here](https://github.com/LedgerHQ/blue-loader-python):
````
virtualenv ledger 
source ledger/bin/activate
pip install ledgerblue
````
- [ ] install dependencies: 
````
SECP_BUNDLED_EXPERIMENTAL=1 pip --no-cache-dir install --no-binary secp256k1 secp256k1
````
- [ ] add the following rules to `/etc/udev/rules.d/`:
````
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0000", MODE="0660", TAG+="uaccess", TAG+="udev-acl" OWNER="<UNIX username>"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0001", MODE="0660", TAG+="uaccess", TAG+="udev-acl" OWNER="<UNIX username>"
SUBSYSTEMS=="usb", ATTRS{idVendor}=="2c97", ATTRS{idProduct}=="0004", MODE="0660", TAG+="uaccess", TAG+="udev-acl" OWNER="<UNIX username>"
````
- [ ] go to the directory containing the app you want to install, and `make load`
- [ ] follow instructions on ledger device
