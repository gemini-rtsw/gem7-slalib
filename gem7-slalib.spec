# SLALIB (Starlink Positional Astronomy Library) for the GEM7 / EPICS 3.13.4
# generation, cross-compiled for vxWorks ppc604.
#
# WHY THE VERSION IS IN THE PACKAGE NAME. VxWorks has no shared libraries: an
# IOC does `ld < .../slalib/V1-9-4/bin/ppc604/slalib` at boot, so the copy on
# the file server IS the code that runs, and two IOCs may need two versions
# resident at once. RPM permits only one version per package NAME, so a plain
# "gem7-slalib" could not satisfy that -- dnf would delete V1-9-4 to install
# V1-9-7 and the next boot of anything pinned to V1-9-4 would fail. Putting the
# version in the name makes them distinct, co-installable packages.
#
# (The alternative, Provides: installonlypkg(kernel), does work with no host
# config -- measured -- but installonly_limit=3 then silently deletes the
# oldest version when a fourth arrives, which on this tree means a running
# instrument losing the library it boots. Rejected for that reason.)

# The version appears twice, in %%define name and %%define version, and the
# %%if guard below turns a mismatch into a build error. It cannot be reduced to
# one place with today's pipeline: gemini-rtsw-ci/build_rpm.sh reads BOTH the
# name and the version by grepping the spec as TEXT, so a macro in either
# arrives unexpanded -- Name becomes the GHCR image tag verbatim, and Version
# silently falls back to "1.0", which then names a source tarball the spec
# cannot find ("Bad source: ...-1.0.tar.gz"). Its rpmspec fallback for Version
# exists but only fires under profile=lightweight, and this is an epics build.
#
# %%define name / %%define version is the shape build_rpm.sh greps for first, so
# it is the form that works unmodified. hrwfs carries
# docs/gemini-rtsw-ci-macro-name.patch to lift both greps to rpmspec; once
# that merges these collapse to a single %%global.
%define name    gem7-slalib-V1-9-4
%define version 1.9.4

# Everything else derives from those, including the install path, so the
# deployed directory can never disagree with the package version.
%global libdir  V%(echo %{version} | tr . -)
%global supdir  /gemini/epics3.13.4/support/slalib
%global instdir %{supdir}/%{libdir}

Name:           %{name}
Version:        %{version}

# A mismatch between the two literals above is a build error, not a package
# whose name and payload disagree.
%global expected_name gem7-slalib-%{libdir}
%if "%{name}" != "%{expected_name}"
%{error:Name is %{name} but version %{version} implies %{expected_name} -- update both}
%endif

# $GIT_HASH first: build_rpm.sh computes it on the HOST and passes it in;
# shelling out to git alone yields "nogit" inside the builder.
%define git_hash %(if [ -n "$GIT_HASH" ]; then echo "$GIT_HASH"; else git rev-parse --short HEAD 2>/dev/null || echo nogit; fi)
Release:        1.git%{git_hash}%{?dist}

Summary:        SLALIB %{version} for GEM7, cross-compiled for vxWorks ppc604
License:        Starlink / Gemini Observatory (org-internal)
Source0:        %{name}-%{version}.tar.gz
AutoReqProv:    no

# The payload is ppc604 objects the build host never executes -- it only
# serves them over NFS to a VME crate -- so the package must not be tagged
# with the builder's arch.
BuildArch:      noarch
%global _binaries_in_noarch_packages_terminate_build 0
%global _build_id_links none
%global __os_install_post %{nil}
%global debug_package %{nil}

# Pinned version-RELEASE, not just version: "= 2.0.2%%{?dist}" expands to
# "2.0.2.el9", which has no release component and matches nothing. The
# rpm-repo is flat -- el8 and el9 share one repo with no dist filtering -- so
# %%{?dist} is what keeps an EL9 build from resolving an EL8 package.
BuildRequires:  gem-tornado20-linux = 2.0.2-1%{?dist}
BuildRequires:  gem-epics3134gem7 = 3.13.4-1%{?dist}
BuildRequires:  make, gcc, perl, tcsh

# So a consumer can pin a version rather than spell out the package name.
Provides:       gem7-slalib = %{version}

%description
SLALIB %{version}, the version the GEM7 IOCs load at boot, installed at
%{instdir}. Built from the CVS history converted with full provenance: this
tree is CVS tag %{libdir}, byte-identical to a `cvs export` of the same tag.

The version is part of the package name so that several versions can be
installed at once -- see the comment at the top of the spec.

%package devel
Summary:        Headers for building against SLALIB %{version}
Provides:       gem7-slalib-devel = %{version}
%description devel
The slalib headers, for IOCs that compile against this version. An IOC's
applSetup.pl -d flag names %{instdir}.

%prep
%setup -q

%build
# Environment from the gem-epics3134gem7 package.
. /etc/profile.d/gem7.sh

# applSetup.pl stamps the checkout's absolute path into config/, so it must run
# here rather than being baked into an image. Its shebang is the Solaris perl
# path, hence the explicit interpreter.
echo "APPLIC_TOP = $PWD" > .applTop
perl $EPICS_BASE/bin/$HOST_ARCH/applSetup.pl -T ppc604 -I src -I startup
make

# The three files an IOC actually loads. A missing one is a crate that stops
# mid-startup, so fail the build instead of shipping it.
for f in bin/ppc604/slalib; do
    [ -f "$f" ] || { echo "ERROR: $f was not built" >&2; exit 1; }
done

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT%{instdir}
for d in bin include lib; do
    [ -d "$d" ] && cp -a "$d" $RPM_BUILD_ROOT%{instdir}/
done
[ -f RELEASE.NOTES ] && cp -a RELEASE.NOTES $RPM_BUILD_ROOT%{instdir}/

# The standalone-IOC startup script carries an absolute cd to the OLD shared
# path (/gemini/epics3.13.4/slalib/slalib). Nothing here sources it -- IOCs ld
# the object directly -- but it would be wrong at this location, so ship
# nothing rather than something misleading. Same call gmos-deplibs makes.
rm -f $RPM_BUILD_ROOT%{instdir}/bin/ppc604/startup \
      $RPM_BUILD_ROOT%{instdir}/bin/ppc604/local

%files
%defattr(-,root,root,-)
%{instdir}
%exclude %{instdir}/include

%files devel
%defattr(-,root,root,-)
%{instdir}/include

%changelog
* Thu Sep 10 2026 Hawi Stecher <hawi.stecher@noirlab.edu> - 1.9.4-1
- First packaged build. Source is CVS tag V1-9-4 from
  polaris:/usr/software/dev/cvsroot/rtcvsroot, converted with cvs2git and
  verified byte-identical to a cvs export of that tag. Built with ANL's
  Linux gcc 2.96 against Tornado 2.0.2 headers; the object is equivalent to
  the deployed one, differing only in that 2.96 applies -mlongcall
  consistently where gcc 2.7.2 emitted some direct bl (REL-4693 precedent).
