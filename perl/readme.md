# Perl module install via CPAN (with China mirrors)

## 1. What is it / What is it for?

`cpan` is Perl's package manager (CPAN = Comprehensive Perl Archive Network). This note configures faster China mirrors and installs a module (`moose`).


- Installing Perl modules without a system package manager.
- Speeding up CPAN downloads from a nearby mirror.

## 2. How to download / install

Enter the CPAN shell and set mirror URLs:
```bash
cpan
o conf urllist push http://mirrors.sohu.com/CPAN/
o conf urllist push http://mirrors.163.com/cpan/
o conf commit
```

All CPAN sources: https://www.cpan.org/sites.html

## 3. How to use

Inside the `cpan` shell:
```bash
install Moose
```
Or non-interactively:
```bash
cpan install Moose
```

