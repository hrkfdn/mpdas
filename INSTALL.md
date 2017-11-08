## Requirements

The following packages are required in order to compile mpdas:

* libcurl, this is generally provided by one of the following (depending which "flavour" you favour):
    * libcurl4-gnutls-dev (GnuTLS flavour)
    * libcurl4-nss-dev (NSS flavour)
    * libcurl4-openssl-dev (OpenSSL flavour)
* libmpdclient-dev

## Building and Installing

mpdas uses `make` for building so this is generally as simple as

    make
    sudo make install
    