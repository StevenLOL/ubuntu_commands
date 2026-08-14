# Reinstall Thunderbird (reset profile)

## 1. What is it?

A one-liner to wipe the Thunderbird mail-client profile and reinstall it — a quick "factory reset" when the client misbehaves.

## 2. What is it for?

- Fixing a corrupted Thunderbird profile (stuck sync, broken UI, extension errors).
- Starting fresh without残留 old account/cache data.

## 3. How to download / install

```bash
# go to home dir
cd ~
# remove the profile (DESTRUCTIVE — deletes local mail cache/accounts)
rm -rf .thunderbird
# reinstall the client
sudo apt-get install --reinstall thunderbird
```

## 4. How to use

Run the two commands; on next launch Thunderbird recreates a clean profile and prompts to set up accounts again.

## 5. Pitfalls

- **`rm -rf .thunderbird` deletes local mail** (cached IMAP, offline stores, account settings). Back up first if mail isn't server-side only.
- **Only removes the current user's profile** — run as the affected user, not root.
- After reset you must re-add accounts and re-download offline mail.
