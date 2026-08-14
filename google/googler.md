# googler (Google from the terminal)

## 1. What is it?

`googler` is a power-tool to search Google (and Google SiteSearch) from the command line. It's not affiliated with Google; it scrapes the search results page.

## 2. What is it for?

- Running web searches without leaving the terminal.
- Automated/JSON result extraction for scripts.

## 3. How to download / install

```bash
sudo apt-get install googler
# or from source: https://github.com/jarun/googler
```

## 4. How to use

```bash
# 15 results, start at offset 3, past-14-days, JSON, no prompt, save to file
googler -n 15 -s 3 -t m14 --json --np cmdline utility -C | tee output.txt
```

Flags:
- `-n N` : number of results
- `-s N` : start offset
- `-t m14` : time filter (past 14 days)
- `--json` : machine-readable output
- `--np` : do not show the omniprompt
- `-C` : enable color

## 5. Pitfalls

- **Not official / can break**: Google changes its HTML and googler stops working until updated.
- **Rate limiting / CAPTCHA**: heavy automated use triggers Google CAPTCHAs; throttle requests.
- **Region/geo**: results depend on the server's IP location; a VPS may return different results than your laptop.
- **Newer forks**: `googler` is semi-unmaintained; consider `ddgr` (same author) for DuckDuckGo if Google breaks.
