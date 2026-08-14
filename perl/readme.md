# Perl module install via CPAN (with China mirrors)

## 1. What is it?

`cpan` is Perl's package manager (CPAN = Comprehensive Perl Archive Network). This note configures faster China mirrors and installs a module (`moose`).

## 2. What is it for?

- Installing Perl modules without a system package manager.
- Speeding up CPAN downloads from a nearby mirror.

## 3. How to download / install

Enter the CPAN shell and set mirror URLs:
```bash
cpan
o conf urllist push http://mirrors.sohu.com/CPAN/
o conf urllist push http://mirrors.163.com/cpan/
o conf commit
```

All CPAN sources: https://www.cpan.org/sites.html

## 4. How to use

Inside the `cpan` shell:
```bash
install Moose
```
Or non-interactively:
```bash
cpan install Moose
```

## 5. Pitfalls

- **`o conf commit` is required** to save the mirror list; without it, the changes are lost on exit.
- **Build deps**: installing XS modules needs a C compiler (`build-essential`); `make`/`gcc` failures mean you're missing them.
- **Mirror freshness**: Sohu/163 mirrors can lag; pick the nearest active one from cpan.org/sites.html.
- Modern Perl also uses `cpanm` (App::cpanminus) for a simpler one-liner: `cpanm Module::Name`.
