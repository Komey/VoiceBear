# -*- coding: utf-8 -*-
# flake8: noqa
import os  
import string
import random
import tempfile
import requests

import unittest
import pytest

from qiniu import Auth, set_default, etag, PersistentFop, build_op, op_save
from qiniu import put_data, put_file, put_stream
from qiniu import BucketManager, build_batch_copy, build_batch_rename, build_batch_move, build_batch_stat, build_batch_delete
from qiniu import urlsafe_base64_encode, urlsafe_base64_decode

from qiniu.compat import is_py2, b

from qiniu.services.storage.uploader import _form_put

import qiniu.config

access_key = 'XXXXXXX' #自己的AK
secret_key = 'XXXXXXX' #自己的SK
bucket_name = 'XXXXX'  #bucket
q = Auth(access_key, secret_key)

localfile = "1.png"
key = etag(localfile)
mime_type = ""

token = q.upload_token(bucket_name, key)

ret, info = put_file(token, key, localfile, mime_type=mime_type, check_crc=True)
print(ret['key'])

try:
    assert ret['hash'] == etag(localfile)
except AssertionError:
    print 'Upload Error'



