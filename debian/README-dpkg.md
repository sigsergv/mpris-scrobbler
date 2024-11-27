# README for cloned repo

Contains `debian` directory to build DPKG package. It's a quick-and-dirty debianized package. But it's buildable.

Compiled binaries are available as PPA <https://launchpad.net/~sigsergv/+archive/ubuntu/mpris-scrobbler>.

## Preparation steps

Download source package (version/tag is important!):

```sh
$ wget 'https://github.com/mariusor/mpris-scrobbler/archive/0d42d4f.tar.gz' -O 'mpris-scrobbler_0.5.3~git0d42d4f.orig.tar.gz'
```

## Build steps

```sh
$ sudo apt install dpkg-dev debhelper libevent-2.1-7 libevent-dev libdbus-1-dev dbus dbus-user-session \
  libcurl4 libcurl4-openssl-dev libjson-c-dev libjson-c-dev meson m4 scdoc
$ dpkg-buildpackage -rfakeroot -b
```

## Package upgrade steps

Attach and fetch upstream:

```sh
$ git remote add upstream https://github.com/mariusor/mpris-scrobbler
$ git fetch upstream --tags
```

Compare revisions rebase to upstream master.

Fetch version:

```sh
$ git describe --tags --long --dirty=-git --always
```
and put it into `patches/02-meson-version-fix.patch`.

Update `changelog` and add new entry for a new version.

Update this file `README-dpkg.md` (wget instructions).

Then do build and/or ubuntu launchpad magic.

## Ubuntu launchpad magic

Repeat steps below for each supported distribution. In my case: `jammy5` and `noble1`.

Edit `debian/changelog` and replace `unstable` in top entry to corresponding distro name,
also add distor suffix (`-jammy5`, `-noble1` etc) to version string.

```sh
$ dpkg-buildpackage --build=source
$ cd ..
$ dput -f ppa:sigsergv/mpris-scrobbler mpris-scrobbler_0.5.3-4-jammy5_source.changes
```

Wait for build to complete.

Do not commit to git changes made in this section.


## TODO

Make properly formatted debian directory that depens upon published upstream source code.

