# Reinstall Thunderbird (reset profile)

## 1. What is it / What is it for?

A one-liner to wipe the Thunderbird mail-client profile and reinstall it — a quick "factory reset" when the client misbehaves.


- Fixing a corrupted Thunderbird profile (stuck sync, broken UI, extension errors).
- Starting fresh without leftover old account/cache data.

## 2. How to download / install

```bash
# go to home dir
cd ~
# remove the profile (DESTRUCTIVE — deletes local mail cache/accounts)
rm -rf .thunderbird
# reinstall the client
sudo apt-get install --reinstall thunderbird
```

## 3. How to use

Run the two commands; on next launch Thunderbird recreates a clean profile and prompts to set up accounts again.

