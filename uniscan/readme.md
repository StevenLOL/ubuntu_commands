# Uniscan (web vulnerability scanner)

## 1. What is it / What is it for?

Uniscan is a Perl-based web-application vulnerability scanner (in the Kali `uniscan` package / SourceForge) that probes a site for common issues.


- Scanning a website you own/are authorized to test for basic vulnerabilities (SQLi, XSS, file inclusion, etc.).
- Producing an HTML report of findings.

## 2. How to download / install

```bash
# from Kali/Debian repos
sudo apt-get install uniscan
# or build from source:
# https://sourceforge.net/p/uniscan/code/ci/master/tree/
```

It depends on the Perl `Moose` module (install via CPAN):
```bash
sudo cpan install Moose
```

## 3. How to use

```bash
uniscan.pl -u http://www.example.com/ -qweds
```
The flags run the various scan modules; it writes an HTML report when done.

