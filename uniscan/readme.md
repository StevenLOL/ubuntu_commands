# Uniscan (web vulnerability scanner)

## 1. What is it?

Uniscan is a Perl-based web-application vulnerability scanner (in the Kali `uniscan` package / SourceForge) that probes a site for common issues.

## 2. What is it for?

- Scanning a website you own/are authorized to test for basic vulnerabilities (SQLi, XSS, file inclusion, etc.).
- Producing an HTML report of findings.

## 3. How to download / install

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

## 4. How to use

```bash
uniscan.pl -u http://www.example.com/ -qweds
```
The flags run the various scan modules; it writes an HTML report when done.

## 5. Pitfalls

- **Only scan sites you're authorized to test** — unauthorized scanning may be illegal. Use it for your own/pen-test-approved targets.
- **`Moose` is required** (Perl dep) — install it or `uniscan.pl` won't run.
- **SourceForge link may move** — check the current Uniscan repo if the tree is gone.
- Refs: hackingvision.com Uniscan tutorial · SourceForge uniscan.
