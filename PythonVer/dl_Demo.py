import requests

key = '2.mp3'
base_url = 'http://%s/%s' % ('XXXXXXX.clouddn.com', key)

print(base_url)

r = requests.get(base_url)

try:
    assert r.status_code == 200
except AssertionError:
    print 'Download Error'

with open(key, 'wb') as code:
    code.write(r.content)

