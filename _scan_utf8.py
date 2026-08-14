import os
bad=[]
skip_ext={'.bak','.pdf','.png','.jpg','.docx','.zip','.notfound'}
skip_dirs={'.codebuddy','.git'}
for r,ds,fs in os.walk('.'):
    ds[:]=[d for d in ds if d not in skip_dirs]
    for f in fs:
        if any(f.endswith(e) for e in skip_ext):
            continue
        p=os.path.join(r,f)
        try:
            b=open(p,'rb').read()
        except Exception:
            continue
        try:
            t=b.decode('utf-8')
        except UnicodeDecodeError:
            bad.append(('NOT_UTF8',p))
            continue
        if '\ufffd' in t:
            bad.append(('HAS_FFFD',p))
print('ISSUES:' if bad else 'ALL_CLEAN')
for x in bad:
    print(x)
