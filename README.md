# GeoIP-Update-for-OPNsense
Update GeoIP data on OPNsense without using Maxmind's account.

## Source of GeoIP
https://github.com/herrbischoff/country-ip-blocks

Many thanks for providing GeoIP data.

## Usage
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
