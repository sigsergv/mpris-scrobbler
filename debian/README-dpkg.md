# README for cloned repo

Contains `debian` directory to build DPKG package. It's a quick-and-dirty debianized package. But it's buildable.

Compiled binaries are available as PPA <https://launchpad.net/~sigsergv/+archive/ubuntu/mpris-scrobbler>.

We use separate branch `master-dpkg` that should be rebased when new upstream release arrives.

## Preparation steps

Download source package **outside** the project directory (version/tag is important!):

```sh
$ wget 'https://github.com/mariusor/mpris-scrobbler/archive/51d20ca.tar.gz' -O 'mpris-scrobbler_0.5.9~m1git51d20ca.orig.tar.gz'
```

## Build steps

Instructions to build binary package.

Install requirements:

```sh
$ sudo apt install dpkg-dev debhelper libevent-2.1-7 libevent-dev libdbus-1-dev dbus dbus-user-session \
  libcurl4 libcurl4-openssl-dev libjson-c-dev libjson-c-dev meson m4 scdoc
```

And build binary packages using this command (use this to test/debug):

```sh
$ dpkg-buildpackage -rfakeroot -b
```

## Package upgrade steps

Hard reset to upstream/master, copy `debian` directory, update files `debian/changelog`
and `debian/README-dpkg.md`: replace existing git commit ref `51d20ca` with a new one
from upstream repo (`upstream/master`).

Update `rules`, set a new version and ref:

```
MPRIS_VERSION=0.5.9
MPRIS_VERSION_REV=51d20ca
```

Build, test and perform ubuntu launchpad magic.

COMMIT change to origin repository.


## Ubuntu launchpad magic

Repeat steps below for each supported distribution. In my case: `noble` and `resolute`.

Edit `changelog` and replace `unstable` in top entry to corresponding distro name (`noble`, `resolute`),
also add distro suffix (`+noble1` etc) to version string.

```sh
$ dpkg-buildpackage --build=source
$ cd ..
$ dput -f ppa:sigsergv/mpris-scrobbler mpris-scrobbler_0.5.9~m1git51d20ca-1+noble1_source.changes
```

Wait for build to complete on project page <https://launchpad.net/~sigsergv/+archive/ubuntu/mpris-scrobbler>.

DO NOT commit to git changes made in this section.


## TODO

Make properly formatted debian directory that depens upon published upstream source code.
