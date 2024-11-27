# README for cloned repo

Contains `debian` directory to build DPKG package. It's a quick-and-dirty debianized package. But it's buildable.

Compiled binaries are available as PPA <https://launchpad.net/~sigsergv/+archive/ubuntu/mpris-scrobbler>.

We use separate branch for each upstream release: `dpkg-v0.5.10` etc.

This README contains instructions for current release.


## Preparation steps

Download source package (version/tag is important!):

```sh
$ wget 'https://github.com/mariusor/mpris-scrobbler/archive/0c6dc81.tar.gz' -O 'mpris-scrobbler_0.5.10~git0c6dc81.orig.tar.gz'
```


## Build steps

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

Steps from this section should be done when new upstream version arrives.

Attach and fetch upstream:

```sh
$ git remote add upstream https://github.com/mariusor/mpris-scrobbler
$ git fetch upstream --tags
```

Fetch most recent tag:

```sh
$ git describe --tags --long --always upstream/master
v0.5.10-0-g0c6dc81
```

Create a new branch with name `dpkg-v0.5.10` based on corresponding release ref:

```sh
$ git switch -c dpkg-v0.5.10 0c6dc81
```

Cherry-pick all relevant commits from previous `dpkg-` release into this new branch.

Update this (`debian/README-dpkg.md`) file, set new base ref and updated version.

Update `debian/changelog` and add new entry for a new version.

Update `debian/rules` and specify new version and ref:

```
MPRIS_VERSION=0.5.10
MPRIS_VERSION_REV=0c6dc81
```

Build, test and perform ubuntu launchpad magic.

Push new branch change to origin repository.


## Ubuntu launchpad magic

Repeat steps below for each supported distribution. In my case: `noble` and `resolute`.

Edit `changelog` and replace `unstable` in top entry to corresponding distro name (`noble`, `resolute`),
also add distro suffix (`+noble1`, `+resolute1` etc) to version string.

```sh
$ dpkg-buildpackage --build=source
$ cd ..
$ dput -f ppa:sigsergv/mpris-scrobbler mpris-scrobbler_0.5.10~git0c6dc81-1+noble1_source.changes
```

Wait for build to complete on project page <https://launchpad.net/~sigsergv/+archive/ubuntu/mpris-scrobbler>.

DO NOT commit to git changes made in this section.


## TODO

Make properly formatted debian directory that depens upon published upstream source code.

