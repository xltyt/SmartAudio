#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import requests
import time
import base64

f = open("data/mda-qmwfy2k746929rxh.mp3", "rb")
obj = {}
obj["text"] = "2024年，我们一起走过春夏秋冬，一道经历风雨彩虹，一个个瞬间定格在这不平凡的一年，令人感慨、难以忘怀。"
obj["text_speech"] = base64.b64encode(f.read()).decode('utf-8')
obj["out_text"] = "今天的天气非常不错，出去玩吧，划划船、跑跑步，多运动运动"
time_start = time.time()
resp_new = requests.post(url="http://127.0.0.1:28800/audio", headers={"Accept": "application/json", "Content-Type": "application/json"}, json=obj)
#print(resp_new.json())
print(time.time() - time_start)
f_resp = open("test_output.wav", "wb")
f_resp.write(base64.b64decode(resp_new.json()["data"]))
f_resp.close()
