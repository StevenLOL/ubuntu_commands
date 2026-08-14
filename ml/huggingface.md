## install huggingface_hub

```
pip install -U huggingface_hub
```

## Huggingface 的中国镜像
### Linux
```
export HF_ENDPOINT=https://hf-mirror.com
```


### Windows Powershell
```
$env:HF_ENDPOINT = "https://hf-mirror.com"
```
## test
```
huggingface-cli download --resume-download gpt2 --local-dir gpt2
```
