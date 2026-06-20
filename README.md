# GeoIP-Update-for-OPNsense
Update GeoIP data on OPNsense without using Maxmind's account.

## Source of GeoIP
https://github.com/ipverse/geo-ip-blocks

Many thanks for providing GeoIP data.

## Usage
### Option 1: Download binary file
Download the latest release from https://github.com/cnbatch/GeoIP-Update-for-OPNsense/releases

Run the following commands in OPNsense

```shell
# fetch https://github.com/cnbatch/GeoIP-Update-for-OPNsense/releases/latest/download/updategeoip.zip
# unzip updategeoip.zip
# chmod +x updategeoip
# mv updategeoip /usr/local/bin
```

and run it

```shell
# updategeoip
```

### Option 2: Compile and install
Run the following commands in FreeBSD or OPNsense

```shell
# make
# make install
```

will generate a binary application `updategeoip` to /usr/local/bin (root account required)

you may also just
```shell
# make
```
and copy `updategeoip` to any directory you want.

and run it

```shell
# updategeoip
```

it will update GeoIP data of OPNsense.

A better way is write a cron entry to update it automatically.
