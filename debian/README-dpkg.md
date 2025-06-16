# README for cloned repo

Contains `debian` directory to build DPKG package. It's a quick-and-dirty debianized package. But it's buildable.

Compiled binaries are available as PPA <https://launchpad.net/~sigsergv/+archive/ubuntu/mpris-scrobbler>.

We use separate branch `master-dpkg` that should be rebased when new upstream release arrives.

## Preparation steps

Download source package (version/tag is important!):

```sh
$ wget 'https://github.com/mariusor/mpris-scrobbler/archive/38c9949.tar.gz' -O 'mpris-scrobbler_0.5.7~git38c9949.orig.tar.gz'
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
v0.5.7-0-g38c9949
```

If there is a new version extract version name (`0.5.7`) and ref (`38c9949`), then rebase `origin/master-dpkg`
to `upstream/master`, also push tags to origin:

```sh
$ git push --tags origin
```

Save ref from previous version to tag with name like this: `v0.5.6-dpkg` and push to origin.

Update `changelog` and add new entry for a new version.

Update `rules` and specify new version and ref:

```
MPRIS_VERSION=0.5.7
MPRIS_VERSION_REV=38c9949
```

Update this file `README-dpkg.md` (wget instructions etc).

Build, test and perform ubuntu launchpad magic.

COMMIT change to origin repository.

## Ubuntu launchpad magic

Repeat steps below for each supported distribution. In my case: `jammy5` and `noble1`.

Edit `changelog` and replace `unstable` in top entry to corresponding distro name (`jammy` or `noble`),
also add distro suffix (`+jammy5`, `+noble1` etc) to version string.

```sh
$ dpkg-buildpackage --build=source
$ cd ..
$ dput -f ppa:sigsergv/mpris-scrobbler mpris-scrobbler_0.5.7~git38c9949-1+jammy5_source.changes
```

Wait for build to complete on project page <https://launchpad.net/~sigsergv/+archive/ubuntu/mpris-scrobbler>.

DO NOT commit to git changes made in this section.


## TODO

Make properly formatted debian directory that depens upon published upstream source code.

